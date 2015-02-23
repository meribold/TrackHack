#include <algorithm>  // find_if
#include <array>
#include <bitset>
#include <cassert>
#include <cmath>      // round
#include <functional> // bind

#include <wx/dcbuffer.h> // wxBufferedPaintDC
#include <wx/graphics.h> // wxGraphicsContext
#include <wx/rawbmp.h>
#include <wx/sizer.h>

#include "track_panel.h"

/*
// http://www.iscc.org/pdf/PC54_1724_001.pdf
// http://hackerspace.lifehacker.com/some-os-x-calendar-tips-1658107833/1665644975
constexpr std::array<std::array<unsigned char, 3>, 18> colors = {
   //std::array<unsigned char, 3>{0xf2, 0xf3, 0xf4},
   //std::array<unsigned char, 3>{0x22, 0x22, 0x22},
   std::array<unsigned char, 3>{0xf3, 0xc3, 0x00},
   std::array<unsigned char, 3>{0x87, 0x56, 0x92},
   std::array<unsigned char, 3>{0xf3, 0x84, 0x00},
   std::array<unsigned char, 3>{0xa1, 0xca, 0xf1},
   std::array<unsigned char, 3>{0xbe, 0x00, 0x32},
   //std::array<unsigned char, 3>{0xc2, 0xb2, 0x80},
   //std::array<unsigned char, 3>{0x84, 0x84, 0x82},
   std::array<unsigned char, 3>{0x00, 0x88, 0x56},
   std::array<unsigned char, 3>{0xe6, 0x8f, 0xac},
   std::array<unsigned char, 3>{0x00, 0x67, 0xa5},
   std::array<unsigned char, 3>{0xf9, 0x93, 0x79},
   std::array<unsigned char, 3>{0x60, 0x4e, 0x97},
   std::array<unsigned char, 3>{0xf6, 0xa6, 0x00},
   std::array<unsigned char, 3>{0xb3, 0x44, 0x6c},
   std::array<unsigned char, 3>{0xdc, 0xd3, 0x00},
   std::array<unsigned char, 3>{0x88, 0x2d, 0x17},
   std::array<unsigned char, 3>{0x8d, 0xb6, 0x00},
   std::array<unsigned char, 3>{0x65, 0x45, 0x22},
   std::array<unsigned char, 3>{0xe2, 0x58, 0x22},
   std::array<unsigned char, 3>{0x2b, 0x3d, 0x26}
};
*/

/*
constexpr std::array<std::array<unsigned char, 3>, 8> colors = {
   std::array<unsigned char, 3>{0xdd, 0x48, 0x14}, // Ubuntu Orange
   std::array<unsigned char, 3>{0x55, 0x6b, 0x2f}, // CSS3 "X11" Dark Olive Green
   std::array<unsigned char, 3>{0x00, 0xbf, 0xff}, // W3C "X11" Deep Sky Blue
   std::array<unsigned char, 3>{0xff, 0xa7, 0x1a}, // Day[9] Yellow
   std::array<unsigned char, 3>{0x8b, 0x00, 0x00}, // CSS3 "X11" Dark Red
   std::array<unsigned char, 3>{0x19, 0x19, 0x70}, // W3C "X11" Midnight Blue
   std::array<unsigned char, 3>{0x32, 0xcd, 0x32}, // W3C "X11" Lime Green
   std::array<unsigned char, 3>{0x77, 0x29, 0x53}  // Canonical Aubergine
};
*/

constexpr std::array<std::array<unsigned char, 3>, 19> colors = {
   std::array<unsigned char, 3>{0x00, 0xff, 0xff}, // X11 Cyan
   std::array<unsigned char, 3>{0xff, 0x00, 0x00}, // X11 Red
   std::array<unsigned char, 3>{0x00, 0xff, 0x00}, // X11 Green
   std::array<unsigned char, 3>{0xff, 0xff, 0x00}, // X11 Yellow
   std::array<unsigned char, 3>{0x1e, 0x90, 0xff}, // X11 Dodger Blue
   std::array<unsigned char, 3>{0xff, 0x14, 0x93}, // X11 Deep Pink
   std::array<unsigned char, 3>{0xff, 0x8c, 0x00}, // X11 Dark Orange
   std::array<unsigned char, 3>{0x00, 0x00, 0xff}, // X11 Blue
   std::array<unsigned char, 3>{0x00, 0x80, 0x00}, // X11 Web Green
   std::array<unsigned char, 3>{0xff, 0x45, 0x00}, // X11 Orange Red
   std::array<unsigned char, 3>{0xa0, 0x20, 0xf0}, // X11 Purple
   std::array<unsigned char, 3>{0xff, 0xb6, 0xc1}, // X11 Light Pink
   std::array<unsigned char, 3>{0xad, 0xff, 0x2f}, // X11 Green Yellow
   std::array<unsigned char, 3>{0x00, 0xfa, 0x9a}, // X11 Medium Spring Green
   std::array<unsigned char, 3>{0x8b, 0x00, 0x00}, // X11 Dark Red
   std::array<unsigned char, 3>{0xf0, 0xe6, 0x8c}, // X11 Khaki
   std::array<unsigned char, 3>{0xad, 0xd8, 0xe6}, // X11 Light Blue
   std::array<unsigned char, 3>{0x19, 0x19, 0x70}, // X11 Midnight Blue
   std::array<unsigned char, 3>{0x80, 0x80, 0x00}, // X11 Olive
};

TrackPanel::TrackPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos,
   const wxSize& size) :
   wxPanel{parent, id, pos, size},
   bitmap{},
   drawingToolStack{},
   defaultPen{}, defaultBrush{},
   trackVisualsMap{},
   focusedIndex{0}
{
   //// <_..._> ////
   ///
   for (auto i = colors.rbegin(); i != colors.rend(); ++i) {
      unsigned char r = (*i)[0], g = (*i)[1], b = (*i)[2];
      drawingToolStack.push(DrawingTools{wxPen{wxColour{r, g, b}},
         wxBrush{wxColour{r, g, b, 0x70}}});
   }
   // "for (auto color : colors)" doesn't give us the right order.

   // no pop() before the pen and brush are assigned to a track
   defaultPen = drawingToolStack.top().pen;
   defaultBrush = drawingToolStack.top().brush;
   ///
   //// </_..._> ////

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
   ///
   //// </_event_handler_mappings_> ////
}

void TrackPanel::reset()
{
   for (const auto& pair : trackVisualsMap)
   {
      const TrackVisuals& trackVisuals = std::get<1>(pair);
      const DrawingTools& drawingTools = std::get<0>(trackVisuals);
      drawingToolStack.push(drawingTools);
   }
   trackVisualsMap.clear();
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
   assert (!drawingToolStack.empty());

   trackVisualsMap.insert(
      std::make_pair(key, TrackVisuals{drawingToolStack.top(), track}));

   // Reuse the last colour indefinitely. TODO: don't.
   if (drawingToolStack.size() > 1) drawingToolStack.pop();
}

void TrackPanel::eraseTrack(const std::string& key)
{
   auto it = trackVisualsMap.find(key);
   if (it != trackVisualsMap.end())
   {
      const DrawingTools& drawingTools = std::get<0>(std::get<1>(*it));
      drawingToolStack.push(drawingTools);
      trackVisualsMap.erase(it);
   }
}

void TrackPanel::useDrawingToolsOf(const std::string& key)
{
   auto it = trackVisualsMap.find(key);

   if (it != trackVisualsMap.end())
   {
      defaultPen   = std::get<0>(std::get<1>(*it)).pen;
      defaultBrush = std::get<0>(std::get<1>(*it)).brush;
   }
}

void TrackPanel::focusIndex(std::size_t index)
{
   focusedIndex = index;
}

//// <_event_handler_definitions_> ////
///
void TrackPanel::onPaint(wxPaintEvent&)
{
   assert (defaultPen.IsOk() && defaultBrush.IsOk());

   wxBufferedPaintDC dC{this};                            // prevents tearing
   wxGraphicsContext *gC = wxGraphicsContext::Create(dC); // uses GDI+ on Windows

   if (gC)
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

   unsigned char intensity = 0;
   wxPoint intensityPeak = rect.GetTopLeft();

   wxNativePixelData pixelData{bitmap};
   wxNativePixelData::Iterator iterator{pixelData};

   for (int row = rect.GetTop(); row < rect.y + rect.height; ++row)
   {
      iterator.MoveTo(pixelData, rect.GetLeft(), row);
      for (int column = rect.GetLeft(); column < rect.x + rect.width; ++column)
      {
         if (intensity < iterator.Red()) {
            intensity = iterator.Red();
            intensityPeak.x = column; intensityPeak.y = row;
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

// vim: tw=90 sw=3 et
