#ifndef MAIN_FRAME_H
#define MAIN_FRAME_H

#include <cstddef> // size_t
#include <map>
#include <memory>  // unique_ptr
#include <string>
#include <vector>

#include <wx/bitmap.h>  // wxBitmap
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/slider.h>
#include <wx/sizer.h>
#include <wx/thread.h>  // wxThreadHelper
#include <wx/menu.h>

#include "movie.hpp"
#include "trackee.hpp"
#include "tracker.hpp"
#include "track_panel.hpp"

class TrackeeBox;

wxDECLARE_EVENT(myEVT_TRACKING_COMPLETED, wxThreadEvent); // ...

class MainFrame : public wxFrame, public wxThreadHelper
{
   public:

   MainFrame(const wxPoint& = wxDefaultPosition, const wxSize& = wxDefaultSize);

   virtual wxThread::ExitCode Entry(); // override; purely virtual in wxThreadHelper

   private:

   // Construct a platform-dependant wxBitmap for drawing it quickly and efficiently to a
   // device context from the bitmap (back end's representation) at the provided position.
   // I found wxBitmap to store at least RGB data on MSW and take no advantage of
   // grayscale images.
   wxBitmap getBitmap(std::size_t);

   // handlers for events generated by trackeeBox and propagated upwards
   void onTrackeeBoxAdded(wxCommandEvent&);    // process a myEVT_COMMAND_TRACKEEBOX_ADDED
   void onTrackeeBoxSelected(wxCommandEvent&); // process a wxEVT_COMMAND_LISTBOX_SELECTED
   void onTrackeeBoxDeleted(wxCommandEvent&);  // process a
                                               // myEVT_COMMAND_TRACKEEBOX_DELETED

   void onMarkBoxSelected(wxCommandEvent&); // process a wxEVT_COMMAND_LISTBOX_SELECTED

   void onTrackPanelMarked(TrackPanelEvent&); // process a myEVT_TRACKPANEL_MARKED
   void onTrackPanelSave(wxCommandEvent&);    // process a myEVT_TRACKPANEL_Save

   // handlers for events generated by movieSlider
   void onScrollThumbtrack(wxScrollEvent&); // process a wxEVT_SCROLL_THUMBTRACK event
   void onScrollChanged(wxScrollEvent&);    // process a wxEVT_SCROLL_CHANGED event
   void onScroll(wxScrollEvent&);           // ...
   void onSlider(wxCommandEvent&);          // process a wxEVT_COMMAND_SLIDER_UPDATED;
                                            // generated after any change of the sliders
                                            // position in addition to a wxScrollEvent

   // Process wxEVT_COMMAND_MENU_SELECTED
   void onOpen(wxCommandEvent&);
   void onSaveImage(wxCommandEvent&);
   void onTrack(wxCommandEvent&);
   void onDeleteTrackee(wxCommandEvent&);
   void onRemoveLink(wxCommandEvent&);
   void onAbout(wxCommandEvent&);

   void onIbidiExport(wxCommandEvent&);     // process a wxEVT_COMMAND_MENU_SELECTED
   void onOneThroughThree(wxCommandEvent&); // process a wxEVT_COMMAND_MENU_SELECTED

   void onTrackingCompleted(wxThreadEvent&); // process a myEVT_TRACKING_COMPLETED

   void onClose(wxCloseEvent&); // process a wxEVT_CLOSE_WINDOW

   void addTrackee(std::string);
   void saveImage();

   wxMenuBar* menuBar;
   wxMenu* fileMenu;
   wxMenu* editMenu;
   wxMenu* viewMenu;
   wxMenu* toolsMenu;
   wxMenu* plugInMenu;
   wxMenu* helpMenu;

   wxPanel* topPanel;

   wxBoxSizer* topSizer;

   TrackeeBox* trackeeBox;
   wxListBox* markBox;
   TrackPanel* trackPanel;
   wxSlider* movieSlider;

   // a map of vectors holding all the marks provided for a particular trackee
   std::map<std::string, std::vector<std::size_t>> marks;

   std::unique_ptr<Movie> movie;
   Tracker tracker;
   std::map<std::string, Trackee> trackees;
};

#endif //MAIN_FRAME_H

// vim: tw=90 sts=3 sw=3 et