#include <algorithm>  // find_if
#include <array>
#include <bitset>
#include <cassert>
#include <cmath>      // round
#include <functional> // bind

//#include <iostream>

#include <wx/dcbuffer.h> // wxBufferedPaintDC
#include <wx/graphics.h> // wxGraphicsContext
#include <wx/menu.h>
#include <wx/rawbmp.h>
#include <wx/sizer.h>

#include "track_panel.hpp"

TrackPanel::TrackPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos,
   const wxSize& size) :
   wxPanel{parent, id, pos, size},
   bitmap{},
   defaultPen{}, defaultBrush{},
   trackVisualsMap{},
   focusedIndex{0}
{
   //defaultColor = colorPool.getColor();
   defaultColor = colorPool.peek();
   defaultPen   = wxPen{*defaultColor};
   defaultBrush = wxBrush{wxColour{defaultColor->Red(), defaultColor->Green(),
      defaultColor->Blue(), 0x70}};

   // Don't erase the window background at all; this avoids flicker caused by pixels being
   // repainted twice when the window is redrawn: see this functions documentation for
   // more information.
   wxWindow::SetBackgroundStyle(wxBG_STYLE_PAINT);

   //// <_event_handler_mappings_> ////
   ///
   Bind(wxEVT_PAINT, &TrackPanel::onPaint, this);

   // call wxWindow::Refresh()
   Bind(wxEVT_SIZE, std::bind(&TrackPanel::Refresh, this, false, nullptr));

   Bind(wxEVT_LEFT_DOWN, &TrackPanel::onLeftDown, this);
   Bind(wxEVT_MOUSE_CAPTURE_LOST, &TrackPanel::onCaptureLost, this);

   Bind(wxEVT_CONTEXT_MENU, &TrackPanel::onContextMenu, this);
   ///
   //// </_event_handler_mappings_> ////
}

void TrackPanel::reset()
{
   for (const auto& pair : trackVisualsMap)
   {
      const TrackVisuals& trackVisuals = std::get<1>(pair);
      const DrawingTools& drawingTools = std::get<0>(trackVisuals);
      colorPool.returnColor(drawingTools.color);
   }
   trackVisualsMap.clear();
   useDrawingToolsOf();
   focusIndex(0);
}

void TrackPanel::setBitmap(const wxBitmap& newBitmap)
{
   bitmap = newBitmap; // wxBitmap uses reference counting

   wxSizer* sizer = GetContainingSizer();
   wxSizerItem* sizerItem;
   if (sizer && (sizerItem = sizer->GetItem(this)))
   {
      sizerItem->SetRatio(bitmap.GetSize());
   }

   // Calling SetMaxClientSize(bitmap.GetSize()) now does not do what I'd expect: the
   // bitmap will still be zoomed (scaled up instead of resizing being limited to the
   // bitmaps actual size).
}

void TrackPanel::addTrack(const std::string& key, std::weak_ptr<const Track> track)
{
   trackVisualsMap.insert(
      std::make_pair(key, TrackVisuals{DrawingTools{colorPool.getColor()}, track}));
   defaultColor = colorPool.peek();
   defaultPen   = wxPen{*defaultColor};
   defaultBrush = wxBrush{wxColour{defaultColor->Red(), defaultColor->Green(),
      defaultColor->Blue(), 0x70}};
}

void TrackPanel::eraseTrack(const std::string& key)
{
   auto it = trackVisualsMap.find(key);
   if (it != trackVisualsMap.end())
   {
      const DrawingTools& drawingTools = std::get<0>(std::get<1>(*it));
      colorPool.returnColor(drawingTools.color);
      trackVisualsMap.erase(it);
   }
}

void TrackPanel::useDrawingToolsOf(const std::string& key)
{
   //std::cout << "useDrawingToolsOf(" << key << ")" << std::endl;
   if (!key.empty())
   {
      auto it = trackVisualsMap.find(key);
      if (it != trackVisualsMap.end())
      {
         defaultColor = std::get<0>(std::get<1>(*it)).color;
         defaultPen   = std::get<0>(std::get<1>(*it)).pen;
         defaultBrush = std::get<0>(std::get<1>(*it)).brush;
      }
   }
   else
   {
      defaultColor = colorPool.peek();
      defaultPen   = wxPen{*defaultColor};
      defaultBrush = wxBrush{wxColour{defaultColor->Red(), defaultColor->Green(),
         defaultColor->Blue(), 0x70}};
   }
}

void TrackPanel::focusIndex(std::size_t index)
{
   focusedIndex = index;
}

// TODO: use wxGraphicsMatrix for transformations?
void TrackPanel::draw(wxGraphicsContext* gC)
{
   // casting to wxDouble; hence parens and not curly braces are used
   gC->DrawBitmap(bitmap, 0., 0., wxDouble(GetClientSize().GetWidth()),
      wxDouble(GetClientSize().GetHeight()));

   if (rect.GetPosition().IsFullySpecified() && rect.GetSize().IsFullySpecified())
   {
      gC->SetPen(defaultPen);
      gC->SetBrush(defaultBrush);
      gC->DrawRectangle(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
   }

   for (auto i : trackVisualsMap) // i is a pair of a key and a trackVisuals tuple
   {
      const TrackVisuals& trackVisuals = std::get<1>(i);
      std::shared_ptr<const Track> track = std::get<1>(trackVisuals).lock();

      //// <_..._> ////
      ///
      {
         wxColour colour{std::get<0>(trackVisuals).pen.GetColour()};
         colour.Set(colour.Red(), colour.Green(), colour.Blue(), 0xc0); // ...
         gC->SetPen(wxPen{colour});

         wxGraphicsPath path = gC->CreatePath();

         auto it = std::find_if(track->begin(), track->end(),
            [](const Point& point) -> bool {
               return point != Point{-1, -1};
            }
         );

         while (it != track->end())
         {
            path.MoveToPoint(bitmapToDeviceX(it->x), bitmapToDeviceY(it->y));

            while (++it != track->end() && *it != Point{-1, -1}) {
               path.AddLineToPoint(bitmapToDeviceX(it->x), bitmapToDeviceY(it->y));
            }

            it = std::find_if(it, track->end(), [](const Point& point) -> bool {
               return point != Point{-1, -1};
               }
            );
         } gC->StrokePath(path);
      }
      ///
      //// </_..._> ////

      //// <_..._> ////
      ///
      {
         gC->SetPen(*wxBLACK_PEN);
         wxColour colour{std::get<0>(trackVisuals).brush.GetColour()};
         colour.Set(colour.Red(), colour.Green(), colour.Blue(), 0xff); // opaque
         gC->SetBrush(wxBrush{colour});

         if ((*track)[focusedIndex] != Point{-1, -1})
         {
            wxCoord deviceX = bitmapToDeviceX((*track)[focusedIndex].x);
            wxCoord deviceY = bitmapToDeviceY((*track)[focusedIndex].y);

            gC->DrawEllipse(bitmapToDeviceX((*track)[focusedIndex].x) - 3.,
                              bitmapToDeviceY((*track)[focusedIndex].y) - 3., 6., 6.);

            {
               const std::string& label = std::get<0>(i);
               gC->SetFont(wxFont{*wxNORMAL_FONT}, *wxWHITE);

               double width, height, descent, externalLeading;
               gC->GetTextExtent(label, &width, &height, &descent, &externalLeading);
               gC->SetPen(*wxTRANSPARENT_PEN);
               gC->SetBrush(wxBrush{wxColour{0x00, 0x00, 0x00, 0x60}});
               gC->DrawRoundedRectangle(deviceX + 4., deviceY + 4., width, height, 3);
               gC->DrawText(label, deviceX + 4., deviceY + 4.);
            }
         }
      }
      ///
      //// </_..._> ////
   }
}

//// <_event_handler_definitions_> ////
///
void TrackPanel::onPaint(wxPaintEvent&)
{
   assert (defaultPen.IsOk() && defaultBrush.IsOk());

   //wxGraphicsContext *gC = wxGraphicsContext::Create(this);
   wxAutoBufferedPaintDC dC{this};                        // prevents tearing
   wxGraphicsContext *gC = wxGraphicsContext::Create(dC); // uses GDI+ on Windows

   //gC->SetAntialiasMode(wxANTIALIAS_DEFAULT);
   gC->SetInterpolationQuality(wxINTERPOLATION_NONE);

   if (gC)
   {
      draw(gC);
      delete gC;
   }
}

void TrackPanel::onLeftDown(wxMouseEvent& event)
{
   assert (!HasCapture());

   CaptureMouse();

   leftDownPoint = event.GetPosition();
   rect = wxRect{event.GetPosition(), wxSize{1, 1}};

   Bind(wxEVT_MOTION, &TrackPanel::onMotion, this);
   Bind(wxEVT_LEFT_UP, &TrackPanel::onLeftUp, this);

   Refresh(false);
   event.Skip(); // It is recommended to, in general, skip all non-command events
                 // (documentation of wxEvent::Skip()).
}

void TrackPanel::onCaptureLost(wxMouseCaptureLostEvent&)
{
   assert (!HasCapture());

   bool didUnbind = Unbind(wxEVT_MOTION, &TrackPanel::onMotion, this) &&
                    Unbind(wxEVT_LEFT_UP, &TrackPanel::onLeftUp, this);
   assert (didUnbind);

   leftDownPoint = wxDefaultPosition;
   rect = wxRect{wxDefaultPosition, wxDefaultSize};

   Refresh(false);
}

void TrackPanel::onMotion(wxMouseEvent& event)
{
   // If the capture was lost due to an external event, the handler for
   // wxEVT_MOUSE_CAPTURE_LOST will unbind this handler.
   assert (HasCapture()); // Thus, HasCapture() should always be true.
   assert (event.Dragging()); // Ditto.

   assert (!rect.IsEmpty());

   //// <_..._> ////
   ///
   if (event.GetX() < 0)
      event.SetX(0);
   else if (event.GetX() >= GetClientSize().GetWidth())
      event.SetX(GetClientSize().GetWidth() - 1);

   if (event.GetY() < 0)
      event.SetY(0);
   else if (event.GetY() >= GetClientSize().GetHeight())
      event.SetY(GetClientSize().GetHeight() - 1);
   ///
   //// </_..._> ////

   rect = wxRect{leftDownPoint, wxSize{1, 1}} + wxRect{event.GetPosition(), wxSize{1, 1}};
   Refresh(false);
}

void TrackPanel::onLeftUp(wxMouseEvent&)
{
   assert (HasCapture());
   assert (rect.GetPosition().IsFullySpecified() && rect.GetSize().IsFullySpecified());
   assert (!rect.IsEmpty());

   bool didUnbind = Unbind(wxEVT_MOTION, &TrackPanel::onMotion, this) &&
                    Unbind(wxEVT_LEFT_UP, &TrackPanel::onLeftUp, this);
   assert (didUnbind);

   leftDownPoint = wxDefaultPosition;

   //// <_..._> ////
   ///
   // convert rect to bitmap coordinates
   rect.x      = deviceToBitmapX(rect.x);
   rect.y      = deviceToBitmapY(rect.y);
   rect.width  = deviceToBitmapX(rect.width);
   rect.height = deviceToBitmapY(rect.height);

   // disallow empty rects
   if (rect.width == 0) rect.width = 1;
   if (rect.height == 0) rect.height = 1;

   int centerX = rect.GetX() + rect.GetWidth() / 2;
   int centerY = rect.GetY() + rect.GetHeight() / 2;

   // It's squared but that's okay.
   auto distanceFromCenter = [&](int x, int y) {
      int xOffset = x - centerX;
      int yOffset = y - centerY;
      return xOffset * xOffset + yOffset * yOffset;
   };

   unsigned char intensity = 0;
   wxPoint intensityPeak = rect.GetTopLeft();
   int distance = distanceFromCenter(intensityPeak.x, intensityPeak.y);

   wxNativePixelData pixelData{bitmap};
   wxNativePixelData::Iterator iterator{pixelData};

   for (int row = rect.GetTop(); row < rect.y + rect.height; ++row)
   {
      iterator.MoveTo(pixelData, rect.GetLeft(), row);
      for (int column = rect.GetLeft(); column < rect.x + rect.width; ++column)
      {
         if (intensity < iterator.Red() || (intensity == iterator.Red() &&
                                            distanceFromCenter(column, row) < distance))
         {
            intensity = iterator.Red();
            intensityPeak.x = column; intensityPeak.y = row;
            distance = distanceFromCenter(column, row);
         }
         ++iterator;
      }
   }
   ///
   //// <_..._> ////

   rect = wxRect{wxDefaultPosition, wxDefaultSize};

   //// <_generate_event_> ////
   ///
   TrackPanelEvent* trackPanelEvent = new TrackPanelEvent{GetId(),
      myEVT_TRACKPANEL_MARKED, intensityPeak};
   trackPanelEvent->SetEventObject(this);
   ::wxQueueEvent(GetEventHandler(), trackPanelEvent); // Queue event for processing
   // during the next event loop iteration; ownership of the event is transferred,
   // therefore trackPanelEvent is not deleted.

   ///
   //// </_generate_event_> ////

   // somehow skipping this event crashes the program

   ReleaseMouse();
   Refresh(false);
}

void TrackPanel::onContextMenu(wxContextMenuEvent&)
{
   wxMenu menu{};
   menu.Append(wxID_SAVE, "&Save");
   menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &TrackPanel::onSave, this, wxID_SAVE);
   PopupMenu(&menu);
}

void TrackPanel::onSave(wxCommandEvent&)
{
   wxCommandEvent newEvent{myEVT_TRACKPANEL_SAVE, GetId()};
   newEvent.SetEventObject(this);
   GetEventHandler()->ProcessEvent(newEvent);
}
///
//// </_event_handler_definitions_> ////

//// </_coordinate_conversion_functions_> ////
///
wxCoord TrackPanel::bitmapToDeviceX(wxCoord bitmapX) const
{
   // casting to double; curly braces would cause an implicit, narrowing conversion (bad)
   return std::round(double(bitmapX * GetClientSize().GetWidth()) /
                     double(bitmap.GetWidth()));
}

wxCoord TrackPanel::bitmapToDeviceY(wxCoord bitmapY) const
{
   return std::round(double(bitmapY * GetClientSize().GetHeight()) /
                     double(bitmap.GetHeight()));
}

wxCoord TrackPanel::deviceToBitmapX(wxCoord deviceX) const
{
   return std::round(double(deviceX * bitmap.GetWidth()) /
                     double(GetClientSize().GetWidth()));
}

wxCoord TrackPanel::deviceToBitmapY(wxCoord deviceY) const
{
   return std::round(double(deviceY * bitmap.GetHeight()) /
                     double(GetClientSize().GetHeight()));
}
///
//// </_coordinate_conversion_functions_> ////

//// <_overrides_> ////
///
bool TrackPanel::AcceptsFocus() const {
   return false;
}

bool TrackPanel::AcceptsFocusFromKeyboard() const {
   return false;
}
///
//// </_overrides_> ////

wxDEFINE_EVENT(myEVT_TRACKPANEL_MARKED, TrackPanelEvent);
wxDEFINE_EVENT(myEVT_TRACKPANEL_SAVE, wxCommandEvent);

// vim: tw=90 sts=3 sw=3 et
