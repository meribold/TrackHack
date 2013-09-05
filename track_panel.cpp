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

TrackPanel::TrackPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) :
	wxPanel{parent, id, pos, size},
	bitmap{},
	drawingToolStack{},
	defaultPen{}, defaultBrush{},
	trackVisualsMap{},
	focusedIndex{0}
{
	//// <_..._> ////
	///
	// Ubuntu Orange
	drawingToolStack.push(DrawingTools{wxPen{wxColour{0xdd, 0x48, 0x14}}, wxBrush{wxColour{0xdd, 0x48, 0x14, 0x70}}});
	// CSS3 "X11" Dark Olive Green
	drawingToolStack.push(DrawingTools{wxPen{wxColour{0x55, 0x6b, 0x2f}}, wxBrush{wxColour{0x55, 0x6b, 0x2f, 0x70}}});
	// W3C "X11" Deep Sky Blue
	drawingToolStack.push(DrawingTools{wxPen{wxColour{0x00, 0xbf, 0xff}}, wxBrush{wxColour{0x00, 0xbf, 0xff, 0x70}}});
	// Day[9] Yellow
	drawingToolStack.push(DrawingTools{wxPen{wxColour{0xff, 0xa7, 0x1a}}, wxBrush{wxColour{0xff, 0xa7, 0x1a, 0x70}}});
	// CSS3 "X11" Dark Red
	drawingToolStack.push(DrawingTools{wxPen{wxColour{0x8b, 0x00, 0x00}}, wxBrush{wxColour{0x8b, 0x00, 0x00, 0x70}}});
	// W3C "X11" Midnight Blue
	drawingToolStack.push(DrawingTools{wxPen{wxColour{0x19, 0x19, 0x70}}, wxBrush{wxColour{0x19, 0x19, 0x70, 0x70}}});
	// W3C "X11" Lime Green
	drawingToolStack.push(DrawingTools{wxPen{wxColour{0x32, 0xcd, 0x32}}, wxBrush{wxColour{0x32, 0xcd, 0x32, 0x70}}});
	// Canonical Aubergine
	drawingToolStack.push(DrawingTools{wxPen{wxColour{0x77, 0x29, 0x53}}, wxBrush{wxColour{0x77, 0x29, 0x53, 0x70}}});
	// ... to be continued

	// no pop() before the pen and brush are assigned to a track
	defaultPen = drawingToolStack.top().pen;
	defaultBrush = drawingToolStack.top().brush;
	///
	//// </_..._> ////

	// don't erase the window background at all; this avoids flicker caused by pixels being repainted twice when the
	// window is redrawn: see this functions documentation for more information
	wxWindow::SetBackgroundStyle(wxBG_STYLE_PAINT);

	//// <_event_handler_mappings_> ////
	///
	Bind(wxEVT_PAINT, &TrackPanel::onPaint, this);
	Bind(wxEVT_SIZE, std::bind(&TrackPanel::Refresh, this, false, nullptr)); // call wxWindow::Refresh()

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

	wxSizer*     sizer = GetContainingSizer();
	wxSizerItem* sizerItem;
	if (sizer && (sizerItem = sizer->GetItem(this)))
	{
		sizerItem->SetRatio(bitmap.GetSize());
	}

	// calling SetMaxClientSize(bitmap.GetSize()) now does not do what I'd expect: the bitmap will still be zoomed
	// (scaled up instead of resizing being limited to the bitmaps actual size)
}

void TrackPanel::addTrack(const std::string& key, std::weak_ptr<const Track> track)
{
	assert (!drawingToolStack.empty());

	trackVisualsMap.insert(std::make_pair(key, TrackVisuals{drawingToolStack.top(), track}));

	if (drawingToolStack.size() > 1) drawingToolStack.pop(); // reuse the last colour indefinitely
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
		gC->DrawBitmap(bitmap, 0., 0., wxDouble{GetClientSize().GetWidth()}, wxDouble{GetClientSize().GetHeight()});

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

				auto it = std::find_if(track->begin(), track->end(), [](const Point& point) -> bool {
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

					gC->DrawEllipse(bitmapToDeviceX((*track)[focusedIndex].x) - 4.,
					                bitmapToDeviceY((*track)[focusedIndex].y) - 4., 8., 8.);

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
	event.Skip(); // it is recommended to, in general, skip all non-command events (documentation of wxEvent::Skip())
}

void TrackPanel::onCaptureLost(wxMouseCaptureLostEvent&)
{
	assert (!HasCapture());

	bool didUnbind =
		Unbind(wxEVT_MOTION, &TrackPanel::onMotion, this) && Unbind(wxEVT_LEFT_UP, &TrackPanel::onLeftUp, this);
	assert (didUnbind);

	leftDownPoint = wxDefaultPosition;
	rect = wxRect{wxDefaultPosition, wxDefaultSize};

	Refresh(false);
}

void TrackPanel::onMotion(wxMouseEvent& event)
{
	// if the capture was lost due to an external event, the handler for wxEVT_MOUSE_CAPTURE_LOST will
	assert (HasCapture()); // unbind this handler; thus, HasCapture() should always be true
	assert (event.Dragging()); // ditto

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

	bool didUnbind =
		Unbind(wxEVT_MOTION, &TrackPanel::onMotion, this) && Unbind(wxEVT_LEFT_UP, &TrackPanel::onLeftUp, this);
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

	for (std::size_t row = rect.GetTop(); row < std::size_t{rect.y + rect.height}; ++row)
	{
		iterator.MoveTo(pixelData, rect.GetLeft(), row);
		for (std::size_t column = rect.GetLeft(); column < std::size_t{rect.x + rect.width}; ++column)
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
	TrackPanelEvent* trackPanelEvent = new TrackPanelEvent{GetId(), myEVT_TRACKPANEL_MARKED, intensityPeak};
	trackPanelEvent->SetEventObject(this);
	::wxQueueEvent(GetEventHandler(), trackPanelEvent); // queue event for processing during the next event loop
	///                                                 // iteration; ownership of the event is transferred,
	//// </_generate_event_> ////                       // therefore trackPanelEvent is not deleted

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
	return std::round(double{bitmapX * GetClientSize().GetWidth()} / double{bitmap.GetWidth()});
}

wxCoord TrackPanel::bitmapToDeviceY(wxCoord bitmapY) const
{
	return std::round(double{bitmapY * GetClientSize().GetHeight()} / double{bitmap.GetHeight()});
}

wxCoord TrackPanel::deviceToBitmapX(wxCoord deviceX) const
{
	return std::round(double{deviceX * bitmap.GetWidth()} / double{GetClientSize().GetWidth()});
}

wxCoord TrackPanel::deviceToBitmapY(wxCoord deviceY) const
{
	return std::round(double{deviceY * bitmap.GetHeight()} / double{GetClientSize().GetHeight()});
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
