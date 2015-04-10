#ifndef TRACK_PANEL_H
#define TRACK_PANEL_H

#include <cstddef> // size_t
#include <map>
#include <memory> // weak_ptr
#include <string>
#include <tuple>

#include <wx/bitmap.h> // wxBitmap
#include <wx/brush.h>
#include <wx/event.h> // wxPaintEvent, wxMouseEvent
#include <wx/gdicmn.h> // wxPoint, wxRect
#include <wx/graphics.h>
#include <wx/panel.h>
#include <wx/pen.h>

#include "color_pool.h"
#include "track.h" // conrete type Track used by the back end to model trajectories

struct DrawingTools {
   DrawingTools(const wxPen& pen, const wxBrush& brush) : pen{pen}, brush{brush} {}

   DrawingTools(const wxColour* color) : color{color}, pen{*color},
      brush{wxColour{color->Red(), color->Green(), color->Blue(), 0x70}} {}

   const wxColour* color;
   wxPen           pen;
   wxBrush         brush;
};

typedef std::tuple<DrawingTools, std::weak_ptr<const Track>> TrackVisuals;

class TrackPanelEvent; // derived from wxEvent; propagated upwards like command events

// generated when the user has marked a point in this wxPanel by dragging a rectangle; the
// pixel with the highest intesity inside the rectangle is used
wxDECLARE_EVENT(myEVT_TRACKPANEL_MARKED, TrackPanelEvent);
wxDECLARE_EVENT(myEVT_TRACKPANEL_SAVE, wxCommandEvent);

// Events emitted by this class:
//    custom event of type TrackPanelEvent with Id myEVT_TRACKPANEL_MARKED; no event macro
class TrackPanel : public wxPanel
{
   public:

   TrackPanel(wxWindow* parent, wxWindowID = wxID_ANY, const wxPoint& = wxDefaultPosition,
      const wxSize& = wxDefaultSize);

   void reset();

   void setBitmap(const wxBitmap&);

   void addTrack(const std::string& key, std::weak_ptr<const Track>);
   void eraseTrack(const std::string& key);

   // set the pen and brush to be used when drawing anything OTHER than a track, to those
   // used for a particular track
   void useDrawingToolsOf(const std::string& key = std::string());

   void focusIndex(std::size_t); // ...

   void draw(wxGraphicsContext*);

   // coordinate converion functions
   wxCoord bitmapToDeviceX(wxCoord) const;
   wxCoord bitmapToDeviceY(wxCoord) const;
   wxCoord deviceToBitmapX(wxCoord) const;
   wxCoord deviceToBitmapY(wxCoord) const;

   // Both functions return false: neither should this panel accept focus from the user
   // clicking it nor should it be included in the TAB traversal chain.
   virtual bool AcceptsFocus() const override;
   virtual bool AcceptsFocusFromKeyboard() const override;

   private:

   void onPaint(wxPaintEvent&);                  // process a wxEVT_PAINT
   void onSize(wxSizeEvent&);                    // process a wxEVT_SIZE

   void onLeftDown(wxMouseEvent&);               // process a wxEVT_LEFT_DOWN; captures
                                                 // the mouse
   void onCaptureLost(wxMouseCaptureLostEvent&); // process a wxEVT_MOUSE_CAPTURE_LOST;
                                                 // handling this event is mandatory for
                                                 // an application that captures the mouse
   void onMotion(wxMouseEvent&);                 // process a wxEVT_MOTION
   void onLeftUp(wxMouseEvent&);                 // process a wxEVT_LEFT_UP

   void onContextMenu(wxContextMenuEvent&);      // process a wxEVT_CONTEXT_MENU
   void onSave(wxCommandEvent&);                 // process a wxEVT_COMMAND_MENU_SELECTED

   wxBitmap bitmap; // platform-dependant bitmap

   ColorPool colorPool;

   // used when drawing anything other than TrackVisuals
   const wxColour* defaultColor;
   wxPen           defaultPen;
   wxBrush         defaultBrush;

   // When a Track is found to have been deleted, the corresponding TrackVisuals object is
   // removed and its color is returned to the ColorPool.
   std::map<std::string, TrackVisuals> trackVisualsMap;

   std::size_t focusedIndex; // ...

   wxPoint leftDownPoint; // stores the position of the mouse fetched from within a
                          // wxEVT_LEFT_DOWN event until a wxEVT_LEFT_UP is processed;
                          // otherwise wxDefaultPosition
   wxRect  rect;          // ...
};

class TrackPanelEvent : public wxEvent
{
   public:

   TrackPanelEvent(int id = 0, wxEventType eventType = wxEVT_NULL,
      const wxPoint& point = wxDefaultPosition) : wxEvent{id, eventType}, point{point}
   {
      // like command events: propagated as much as necessary
      wxEvent::m_propagationLevel = wxEVENT_PROPAGATE_MAX;
   }

   // Implement pure virtual function Clone().
   virtual TrackPanelEvent* Clone() const override { return new TrackPanelEvent{*this}; }

   const wxPoint& getPoint() const { return point; };
   void setPoint(const wxPoint& point) { this->point = point; };

   private:

   wxPoint point;
};

#endif //TRACK_PANEL_H

// vim: tw=90 sw=3 et
