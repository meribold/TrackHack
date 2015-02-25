#include "main_frame.h"

#include <algorithm>  // lower_bound
#include <array>
#include <cassert>
#include <fstream>    // ofstream
#include <functional> // bind
#include <iomanip>    // stream manipulator functions setfill() and setw()
#include <memory>     // shared_ptr
#include <sstream>    // stringstream
#include <string>

#include <wx/aboutdlg.h>    // wxAboutBox()
#include <wx/filehistory.h> // wxFileHistory
#include <wx/filename.h>    // wxFileName
#include <wx/log.h>         // wxLogError()
#include <wx/dcmemory.h>
#include <wx/rawbmp.h>
#include <wx/stdpaths.h>    // wxStandardPaths

#include "bitmap.h"
#include "open_movie_wizard.h"
#include "track_panel.h"
#include "trackee_box.h"

#include "ibidi_export.h"
#include "one_through_three.h"

// weakly typed enum because implicit conversion is convenient
enum mainFrameId : unsigned { myID_TRACKEEBOX = wxID_HIGHEST, myID_LINKBOX, myID_TRACK,
   myID_DELETE_TRACKEE, myID_REMOVE_LINK };

template <std::size_t N>
void createBitmaps(std::string name);

//// <_constructors_> ////
///
MainFrame::MainFrame(const wxPoint& pos, const wxSize& size) :
   wxFrame{nullptr, wxID_ANY, "TrackHack", pos, size},
   menuBar{new wxMenuBar{}},
   fileMenu{new wxMenu{}},
   editMenu{new wxMenu{}},
   //viewMenu{new wxMenu{}},
   toolsMenu{new wxMenu{}},
   plugInMenu{new wxMenu{}},
   helpMenu{new wxMenu{}},
   topPanel{new wxPanel{this, wxID_ANY, wxDefaultPosition, wxDefaultSize}},
   topSizer{new wxBoxSizer{wxVERTICAL}},
   trackeeBox{new TrackeeBox{topPanel, myID_TRACKEEBOX}},
   markBox{new wxListBox{topPanel, myID_LINKBOX}},
   trackPanel{new TrackPanel{topPanel}},
   movieSlider{new wxSlider{topPanel, wxID_ANY, 0, 0, 2, wxDefaultPosition, wxDefaultSize,
      wxSL_LABELS}},
   marks{}, movie{}, tracker{}, trackees{}
{
   {
      wxFileName splashFileName{wxStandardPaths::Get().GetUserDataDir().ToStdString(),
         "track_hack_splash_", ""};

      createBitmaps<3>(splashFileName.GetFullPath().ToStdString());
      movie = std::unique_ptr<Movie>{new Movie{splashFileName.GetPath().ToStdString(),
         "track_hack_splash_[0-9]{2}\\.bmp"}};

      if (!movie->getSize()) throw "CURSE IT!";
   }

   //// <_..._> ////
   ///
   wxBoxSizer* expandedHBox = new wxBoxSizer{wxHORIZONTAL};
   expandedHBox->Add(trackeeBox, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 1);
   expandedHBox->Add(markBox, 0, wxEXPAND | wxTOP | wxRIGHT, 1);

   topSizer->Add(expandedHBox, 0, wxEXPAND);
   topSizer->Add(trackPanel, 1, wxALIGN_CENTER | wxSHAPED | wxALL, 1);
   topSizer->Add(movieSlider, 0, wxEXPAND | wxLEFT | wxRIGHT, 16);

   markBox->Hide();

   topPanel->SetSizer(topSizer);
   SetMinSize(wxSize{384, 432});
   ///
   //// </_..._> ////

   //// <_menu_bar_contruction_> ////
   ///
   // See the documentation of wxMenuItem::SetItemLabel() to find how the mnemonics and
   // accelerator strings in the following menu items magically work all by themselves.
   fileMenu->Append(wxID_OPEN, "&Open\tCtrl+O", "Load a movie composed of grayscale "
      "bitmaps");
   fileMenu->AppendSeparator();
   fileMenu->Append(wxID_SAVE, "&Save image\tCtrl+S", "Save an image of the panel below");
   fileMenu->AppendSeparator();
   fileMenu->Append(wxID_EXIT, "&Quit\tCtrl+Q");

   editMenu->Append(myID_TRACK, "&Track\tCtrl+T");
   editMenu->AppendSeparator();
   editMenu->Append(myID_DELETE_TRACKEE, "&Delete trackee\tCtrl+D");
   editMenu->Enable(myID_DELETE_TRACKEE, false);
   editMenu->Append(myID_REMOVE_LINK, "&Remove link\tCtrl+R");
   editMenu->Enable(myID_REMOVE_LINK, false);
   editMenu->AppendSeparator();
   editMenu->Append(wxID_PROPERTIES, "&Properties\tCtrl+P");
   editMenu->Enable(wxID_PROPERTIES, false);

   toolsMenu->Append(wxID_ANY, "View s&ystem-wide configuration file");
   toolsMenu->Append(wxID_ANY, "View &user-specific configuration file");
   toolsMenu->AppendSeparator();
   toolsMenu->Append(wxID_PREFERENCES, "&Settings");

   wxWindowID myID_ONE_THREE = NewControlId();
   plugInMenu->Append(myID_ONE_THREE, "OneToThree");
   wxWindowID myID_IBIDI_EXPORT = NewControlId();
   plugInMenu->Append(myID_IBIDI_EXPORT, "ibidi export", "Save all tracks formatted to "
      "be imported by ibidi's Chemotaxis and Migration Tool");

   helpMenu->Append(wxID_ABOUT, "&About TrackHack");

   menuBar->Append(fileMenu, "&File");
   menuBar->Append(editMenu, "&Edit");
   //menuBar->Append(viewMenu, "&View");
   menuBar->Append(toolsMenu, "&Tools");
   //menuBar->Append(plugInMenu, "&Plug-ins");
   menuBar->Append(plugInMenu, "E&xtras");
   menuBar->Append(helpMenu, "&Help");

   SetMenuBar(menuBar);

   menuBar->EnableTop(menuBar->FindMenu("&Tools"), false);
   ///
   //// </_menu_bar_construction_> ////

   CreateStatusBar(1, wxSTB_SIZEGRIP | wxSTB_SHOW_TIPS | wxSTB_ELLIPSIZE_START |
      wxFULL_REPAINT_ON_RESIZE);

   trackPanel->setBitmap(getBitmap(0));
   SetStatusText(movie->getFilename(0));

   //// <_event_handler_mappings_> ////
   ///
   Bind(myEVT_COMMAND_TRACKEEBOX_ADDED, &MainFrame::onTrackeeBoxAdded, this,
      myID_TRACKEEBOX);
   Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &MainFrame::onTrackeeBoxSelected, this,
      myID_TRACKEEBOX);
   Bind(myEVT_COMMAND_TRACKEEBOX_DELETED, &MainFrame::onTrackeeBoxDeleted, this,
      myID_TRACKEEBOX);

   Bind(wxEVT_COMMAND_LISTBOX_SELECTED, &MainFrame::onMarkBoxSelected, this,
      myID_LINKBOX);

   Bind(myEVT_TRACKPANEL_MARKED, &MainFrame::onTrackPanelMarked, this, wxID_ANY);
   trackPanel->Bind(myEVT_TRACKPANEL_SAVE, &MainFrame::onTrackPanelSave, this);

   Bind(wxEVT_SCROLL_THUMBTRACK, &MainFrame::onScrollThumbtrack, this, wxID_ANY);
   Bind(wxEVT_SCROLL_CHANGED, &MainFrame::onScrollChanged, this, wxID_ANY);
   Bind(wxEVT_COMMAND_SLIDER_UPDATED, &MainFrame::onSlider, this, wxID_ANY);

   Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onOpen, this, wxID_OPEN);
   Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onSaveImage, this, wxID_SAVE);
   Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onTrack, this, myID_TRACK);
   Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onDeleteTrackee, this,
      myID_DELETE_TRACKEE);
   Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onRemoveLink, this, myID_REMOVE_LINK);
   // wxWindow::Close(true)
   Bind(wxEVT_COMMAND_MENU_SELECTED, std::bind(&MainFrame::Close, this, true), wxID_EXIT);
   Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onAbout, this, wxID_ABOUT);

   Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onIbidiExport, this, myID_IBIDI_EXPORT);
   Bind(wxEVT_COMMAND_MENU_SELECTED, &MainFrame::onOneThroughThree, this, myID_ONE_THREE);

   Bind(myEVT_TRACKING_COMPLETED, &MainFrame::onTrackingCompleted, this, wxID_ANY);

   Bind(wxEVT_CLOSE_WINDOW, &MainFrame::onClose, this);
   ///
   //// </_event_handler_mappings_> ////
}
///
//// <_/constructors_> ////

//// <_overrides_> ////
///
wxThread::ExitCode MainFrame::Entry()
{
   tracker.track(trackees, *movie);

   // processed during the next event loop iteration
   QueueEvent(new wxThreadEvent{myEVT_TRACKING_COMPLETED});

   return wxThread::ExitCode{0};
}

///
//// <_overrides_> ////

//// <_event_handler_definitions> ////
///
void MainFrame::onTrackeeBoxAdded(wxCommandEvent& event)
{
   std::string key = event.GetString().ToStdString();
   addTrackee(key);

   event.Skip();
}

namespace {
   wxString makeMarkString(std::size_t); // ...
}

void MainFrame::onTrackeeBoxSelected(wxCommandEvent& event)
{
   markBox->Clear();

   if (!event.GetString().empty())
   {
      std::string key = event.GetString().ToStdString();

      auto cachedEffectiveMinHeight = markBox->GetEffectiveMinSize().GetHeight();

      for (std::size_t i : marks[key])
         markBox->Append(makeMarkString(i));

      // ...
      if (markBox->IsShown()) {
         if (markBox->IsEmpty()) {
            markBox->Hide();
            topPanel->Layout();
         }
         else if (markBox->GetEffectiveMinSize().GetHeight() != cachedEffectiveMinHeight) {
            topPanel->Layout();
         }
      }
      else if (!markBox->IsEmpty()) { // mark box is not shown; should it be?
         markBox->Show();
         topPanel->Layout();
      }
      // ...
      auto iterator = std::find(marks[key].begin(), marks[key].end(),
         movieSlider->GetValue());
      if (iterator != marks[key].end()) {
         markBox->SetSelection(iterator - marks[key].begin());
         GetMenuBar()->Enable(myID_REMOVE_LINK, true);
      }
      else {
         GetMenuBar()->Enable(myID_REMOVE_LINK, false);
      }

      GetMenuBar()->Enable(myID_DELETE_TRACKEE, true);

      trackPanel->useDrawingToolsOf(key);
      trackPanel->Refresh(false);
   }
   else
   {
      if (markBox->IsShown()) {
         markBox->Hide();
         topPanel->Layout();
      }
      trackPanel->useDrawingToolsOf();
      GetMenuBar()->Enable(myID_DELETE_TRACKEE, false);
      GetMenuBar()->Enable(myID_REMOVE_LINK, false);
   }

   // No event.Skip() necessary.
}

// TODO: move common code from this and onDeleteTrackee() to new function.
void MainFrame::onTrackeeBoxDeleted(wxCommandEvent& event)
{
   std::string key = event.GetString().ToStdString();

   trackees.erase(key);
   marks.erase(key);

   trackPanel->eraseTrack(key);

   if (trackeeBox->getStringSelection().empty()) {
      GetMenuBar()->Enable(myID_DELETE_TRACKEE, false);
      GetMenuBar()->Enable(myID_REMOVE_LINK, false);
      markBox->Clear();
      markBox->Hide();
   }
   topPanel->Layout();
   trackPanel->Refresh(false);
}

void MainFrame::onMarkBoxSelected(wxCommandEvent& event)
{
   auto trackeeKey = trackeeBox->getStringSelection().ToStdString();
   movieSlider->SetValue(marks[trackeeKey][event.GetSelection()]); // doesn't generate an
                                                                   // event.
   trackPanel->setBitmap(getBitmap(movieSlider->GetValue()));
   trackPanel->focusIndex(movieSlider->GetValue());

   GetMenuBar()->Enable(myID_REMOVE_LINK, true);

   trackPanel->Refresh(false);

   // No event.Skip() necessary.
}

void MainFrame::onTrackPanelMarked(TrackPanelEvent& event)
{
   auto trackeeKey = trackeeBox->getStringSelection().ToStdString();

   if (trackeeKey.empty())
   {
      trackeeKey = trackeeBox->addTrackee();
      if (!trackeeKey.empty()) {
         addTrackee(trackeeKey);
      }
   }

   if (!trackeeKey.empty())
   {
      trackees[trackeeKey].setPoint(movieSlider->GetValue(),
         Point{event.getPoint().x, event.getPoint().y});

      // Shadows data member "marks".
      std::vector<std::size_t>& marks = this->marks[trackeeKey];

      // iterator to the first element in marks that does not compare less than
      // movieSlider->GetValue()
      auto iterator = std::lower_bound(marks.begin(), marks.end(),
         movieSlider->GetValue());

      if (iterator == marks.end() ||
          static_cast<int>(*iterator) != movieSlider->GetValue())
      {
         // insert before iterator, invalidating all previously obtained iterators,
         // references and pointers
         iterator = marks.insert(iterator, movieSlider->GetValue());
      }

      // Define which part of the track need be computed anew by assigning {-1, -1} to all
      // points that might change due to the new definitive point if tracking is done
      // again.
      {
         std::size_t i = (iterator == marks.begin()) ? 0 : *(iterator - 1) + 1;
         for (; i < static_cast<unsigned>(movieSlider->GetValue()); ++i)
         {
            trackees[trackeeKey].setPoint(i, Point{-1, -1});
         }
         i = (iterator + 1 == marks.end()) ? movie->getSize() - 1 : *(iterator + 1) - 1;
         for (; i > static_cast<unsigned>(movieSlider->GetValue()); --i)
         {
            trackees[trackeeKey].setPoint(i, Point{-1, -1});
         }
      }

      if (!trackeeBox->getStringSelection().empty() &&
         markBox->FindString(makeMarkString(movieSlider->GetValue())) == wxNOT_FOUND)
      {
         auto cachedEffectiveMinHeight = markBox->GetEffectiveMinSize().GetHeight();

         markBox->SetSelection(markBox->Insert(makeMarkString(movieSlider->GetValue()),
            iterator - marks.begin()));
         GetMenuBar()->Enable(myID_REMOVE_LINK, true);

         // ...
         if (!markBox->IsShown()) {
            markBox->Show();
            topPanel->Layout();
         }
         else if (markBox->GetEffectiveMinSize().GetHeight() != cachedEffectiveMinHeight)
         {
            topPanel->Layout();
         }
      }
      trackPanel->Refresh(false); // TODO: is this unneeded on Windows? Why?
   }
   // Don't Skip() the event.
}

void MainFrame::onTrackPanelSave(wxCommandEvent&)
{
   saveImage();
}

void MainFrame::onScrollChanged(wxScrollEvent& /*event*/)
{
   // topPanel->Layout() would be necessary but all bitmaps in a movie are required to be
   // of equal size.
   //trackPanel->setBitmap(getBitmap(event.GetPosition()));
}

void MainFrame::onScrollThumbtrack(wxScrollEvent& /*event*/)
{
   //trackPanel->setBitmap(getBitmap(event.GetPosition()));
}

void MainFrame::onSlider(wxCommandEvent&)
{
   trackPanel->setBitmap(getBitmap(movieSlider->GetValue()));
   {
      // ...
      std::vector<std::size_t>& marks =
         this->marks[trackeeBox->getStringSelection().ToStdString()];
      auto iterator = std::find(marks.begin(), marks.end(), movieSlider->GetValue());
      if (iterator != marks.end()) {
         markBox->SetSelection(iterator - marks.begin());
         GetMenuBar()->Enable(myID_REMOVE_LINK, true);
      } else {
         markBox->SetSelection(wxNOT_FOUND);
         GetMenuBar()->Enable(myID_REMOVE_LINK, false);
      }
   }

   SetStatusText(movie->getFilename(movieSlider->GetValue()));

   trackPanel->focusIndex(movieSlider->GetValue());
   trackPanel->Refresh(false);
}

void MainFrame::onOpen(wxCommandEvent&)
{
   wxString dir, regEx;

   wxFileHistory dirHistory{7}; // Seven.
   dirHistory.Load(*wxConfigBase::Get());

   OpenMovieWizard wiz{this, wxID_ANY, &dir, &regEx, wxConfigBase::Get(), &dirHistory};

   if (wiz.RunWizard()) // The user did not cancel the wizard.
   {
      dirHistory.AddFileToHistory(dir);
      dirHistory.Save(*wxConfigBase::Get());

      std::unique_ptr<Movie> newMovie{new Movie{dir.ToStdString(), regEx.ToStdString()}};

      if (newMovie->getSize() > 1) // And selected a movie with at least two frames.
      {
         movie = std::move(newMovie);

         GetMenuBar()->Enable(myID_DELETE_TRACKEE, false);
         GetMenuBar()->Enable(myID_REMOVE_LINK, false);
         trackeeBox->reset();
         trackeeBox->SetFocus();
         markBox->Clear();
         markBox->Hide();
         trackPanel->reset();

         marks.clear();
         trackees.clear();

         movieSlider->SetRange(0, movie->getSize() - 1);
         movieSlider->SetValue(0); // does not post or queue an event
         trackPanel->setBitmap(getBitmap(0));
         SetStatusText(movie->getFilename(0));

         // First, invoke the sizer-based layout algorithm for topPanel, THEN cause
         // movieSlider to be repainted.
         topPanel->Layout();       // Somehow repainting only movieSlider
                                   // (movieSlider->Refresh()) is not enough to fix it but
         topPanel->Refresh(false); // most of topPanels other children have to be
                                   // repainted anyway.
      }
   }
}

void MainFrame::onSaveImage(wxCommandEvent&)
{
   saveImage();
}

void MainFrame::onTrack(wxCommandEvent&)
{
   if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR) {
      return;
   }
   if (GetThread()->Run() != wxTHREAD_NO_ERROR) {
      return;
   }

   //trackPanel->SetEvtHandlerEnabled(false);
   trackeeBox->Disable();
   trackPanel->Disable();

   GetMenuBar()->Enable(myID_DELETE_TRACKEE, false);
   GetMenuBar()->Enable(myID_REMOVE_LINK, false);
}

// TODO: move common code from this and onTrackeeBoxDeleted() to new function.
void MainFrame::onDeleteTrackee(wxCommandEvent&)
{
   std::string key = trackeeBox->getStringSelection().ToStdString();
   assert(!key.empty());

   std::size_t erasedElements = trackees.erase(key);
   assert(erasedElements == 1);

   marks.erase(key);

   trackeeBox->deleteSelection();
   trackPanel->eraseTrack(key);

   if (trackeeBox->getStringSelection().empty()) {
      GetMenuBar()->Enable(myID_DELETE_TRACKEE, false);
      GetMenuBar()->Enable(myID_REMOVE_LINK, false);
      markBox->Clear();
      markBox->Hide();
   }
   topPanel->Layout();
   trackPanel->Refresh(false);
}

// Somewhat similar to code in MainFrame::onTrackPanelMarked().
void MainFrame::onRemoveLink(wxCommandEvent&)
{
   auto linkKey = markBox->GetStringSelection().ToStdString();

   if (!linkKey.empty())
   {
      auto trackeeKey = trackeeBox->getStringSelection().ToStdString();
      assert (!trackeeKey.empty());

      std::vector<std::size_t>& links = this->marks[trackeeKey];
      auto frameIndex = std::stoi(linkKey);

      auto iterator = std::find(links.begin(), links.end(), frameIndex);
      assert (iterator != links.end());

      std::size_t first = (iterator == links.begin()) ? 0 : *(iterator - 1) + 1;
      std::size_t last = (iterator + 1 == links.end()) ?
                         movie->getSize() : *(iterator + 1);
      for (auto i = first; i < last; ++i)
      {
         trackees[trackeeKey].setPoint(i, Point{-1, -1});
      }

      links.erase(iterator);

      markBox->Delete(markBox->GetSelection());
      if (markBox->IsEmpty()) {
         markBox->Hide();
         topPanel->Layout();
      }
      GetMenuBar()->Enable(myID_REMOVE_LINK, false);

      trackPanel->Refresh(false);
   }
}

void MainFrame::onAbout(wxCommandEvent&)
{
   wxAboutDialogInfo info;
   info.SetName(u8"TrackHack");
   info.SetDescription(u8"This is a simple program for tracking objects in a movie "
      "composed of individual grayscale images; tracking is based on intensity peaks and "
      "speed caps.");

   ::wxAboutBox(info, this);
}

void MainFrame::onIbidiExport(wxCommandEvent&)
{
   if (trackees.empty()) return;

   std::string fileName = movie->getDir() + "ibidi_tracks.txt";
   ibidiExport(fileName, trackees);
}

void MainFrame::onOneThroughThree(wxCommandEvent&)
{
   std::string key = trackeeBox->getStringSelection().ToStdString();
   if (!key.empty()) {
      auto it = trackees.find(key);
      if (it != trackees.end()) {
         const Trackee& trackee = std::get<1>(*it);
         std::shared_ptr<const Track> track = trackee.getTrack().lock();
         if (track) {
            const Point& point = (*track)[movieSlider->GetValue()];
            oneThroughThree(getBitmap(movieSlider->GetValue()), wxPoint{point.x, point.y},
                            movie.get(), &key, &trackee);
         }
      }
   }
}

/* From ##c++
 *
 * 10:20 < meribold> Does std::ofstream clear the contents of a file when i don't specify
 *                   an openmode?
 * 10:23 < TinoDidriksen> Default is to truncate, yes.
 * 10:24 < meribold> TinoDidriksen: Good to know; i was looking at some code i wrote 2
 *                   years ago and wondering if it only worked because i was lucky
 * 10:24 < GrecKo> is it ? reading http://en.cppreference.com/w/cpp/io/basic_ofstream/open
 *                 I would assume default is only out (open for writing)
 * 10:28 < TinoDidriksen> GrecKo, devil is in the details. It must behave as if fopen()
 *                        was used, and fopen() with flag "w" truncates.
 * 10:28 < GrecKo> ok
 * 10:29 < GrecKo> ok I didn't read this
 *                 http://en.cppreference.com/w/cpp/io/basic_filebuf/open , I understand
 *                 now
 * 10:30 < TinoDidriksen> Ah, nice overview.
 */
void MainFrame::onTrackingCompleted(wxThreadEvent&)
{
   if (trackees.empty()) return;

   {
      std::ofstream oStream{movie->getDir() + "all_tracks.txt"}; // RAII

      oStream << "File name";

      for (const auto& pair : trackees)
      {
         const std::string key = std::get<0>(pair);
         oStream << '\t' << key << " (x)\t" << key << " (y)";
      }
      oStream << '\n';

      for (std::size_t i = 0; i < movie->getSize(); ++i)
      {
         oStream << movie->getFrame(i).getFilename();

         for (const auto& pair : trackees)
         {
            const Trackee trackee = std::get<1>(pair);
            std::shared_ptr<const Track> track = trackee.getTrack().lock();
            oStream << '\t' << (*track)[i].x << '\t' << (*track)[i].y;
         }
         oStream << '\n';
      }
   }

   for (const auto& pair : trackees)
   {
      const std::string key = std::get<0>(pair);
      const Trackee trackee = std::get<1>(pair);

      std::shared_ptr<const Track> track = trackee.getTrack().lock();

      std::ofstream oStream{movie->getDir() + key + "_track.txt"};

      for (std::size_t i = 0; i < movie->getSize(); ++i)
      {
         oStream << movie->getFrame(i).getFilename() << '\t' << (*track)[i].x << '\t' <<
            (*track)[i].y << '\n';
      }
   }

   trackeeBox->Enable();
   trackPanel->Enable();

   if (!trackeeBox->getStringSelection().empty()) {
      GetMenuBar()->Enable(myID_DELETE_TRACKEE, true);
   }
   if (markBox->IsShown() && markBox->GetSelection() != wxNOT_FOUND) {
      GetMenuBar()->Enable(myID_REMOVE_LINK, true);
   }

   trackPanel->Refresh(false);
}

void MainFrame::onClose(wxCloseEvent& event)
{
   // true if tracking was started and is still working.
   if (GetThread() && GetThread()->IsRunning()) {
      GetThread()->Wait(); // join
   }

   event.Skip(); // The default handler will call this->Destroy().
}
///
//// </_event_handler_definitions> ////

void MainFrame::addTrackee(std::string key)
{
   assert (!key.empty());

   auto pair = trackees.insert(std::make_pair(key, Trackee{9, movie->getSize()})); // ...
   assert (std::get<1>(pair));

   trackPanel->addTrack(key, std::get<1>(*std::get<0>(pair)).getTrack());
   trackPanel->Refresh(false);

   if (markBox->IsShown())
   {
      assert (!markBox->IsEmpty());
      markBox->Clear();
      markBox->Hide();
      topPanel->Layout();
   }
   // wxWindow::GetEffectiveMinSize() is, as per its documentation, "the value used by
   // sizers to determine the appropriate amount of space to allocate for the widget" and
   // "called by a wxSizer when it queries the size of a window or control"; as *listBox
   // is added to its sizer with a proportion of 0, that should be the value the sizer
   // ends up giving it (afaics); thus, this condition should determine if posting a size
   // event to *listBox' parent would change anything.
   else if (trackeeBox->GetEffectiveMinSize().GetHeight() !=
            trackeeBox->GetSize().GetHeight())
   {
      // From the documentation of wxWindow::SendSizeEvent(): "It is sometimes useful to
      // call this function after [...] a child size changes.  Note that if the frame is
      // using [...] sizers [...], it is enough to call wxWindow::Layout() directly and
      // this function should not be used [...]."
      topPanel->Layout();
   }

   GetMenuBar()->Enable(myID_REMOVE_LINK, false);
}

void MainFrame::saveImage()
{
   auto filename = movie->getFilename(movieSlider->GetValue());
   auto found = filename.rfind('.');
   if (found != std::string::npos) {
      filename.insert(found, "_tracks");
      //wxBitmap bitmap = getBitmap(0);
      wxBitmap bitmap{trackPanel->GetClientSize().GetWidth(),
         trackPanel->GetClientSize().GetHeight()};
      wxMemoryDC dC{bitmap};
      wxGraphicsContext *gC = wxGraphicsContext::Create(dC);
      gC->SetInterpolationQuality(wxINTERPOLATION_BEST);
      trackPanel->draw(gC);
      delete gC;
      dC.SelectObject(wxNullBitmap);
      bitmap.SaveFile(filename, wxBITMAP_TYPE_BMP);
   }
}

wxBitmap MainFrame::getBitmap(std::size_t index)
{
   //std::shared_ptr<const Bitmap> bitmap = movie.getFrame(index).getBitmap(false);
   std::shared_ptr<const Bitmap> bitmap = movie->getFrame(index).getBitmap(true);
   wxBitmap nativeBitmap;

   if (bitmap)
   {
      nativeBitmap.Create(bitmap->width, bitmap->height, 24);
      wxNativePixelData pixelData{nativeBitmap};
      wxNativePixelData::Iterator iterator{pixelData};

      for (std::size_t row = 0; row < bitmap->height; ++row)
      {
         iterator.MoveTo(pixelData, 0, row);
         for (std::size_t column = 0; column < bitmap->width; ++column)
         {
            iterator.Red() = iterator.Green() = iterator.Blue() = (*bitmap)[row][column];
            ++iterator;
         }
      }
   }
   /*else
   {
      nativeBitmap.LoadFile(movie.getFilename(index), wxBITMAP_TYPE_ANY);
      wxNativePixelData pixelData{nativeBitmap};
      wxNativePixelData::Iterator iterator{pixelData};

      bitmap = std::make_shared<Bitmap>(nativeBitmap.GetWidth(), nativeBitmap.GetHeight());

      for (std::size_t row = 0; row < bitmap->height; ++row)
      {
         iterator.MoveTo(pixelData, 0, row);
         for (std::size_t column = 0; column < bitmap->width; ++column)
         {
            (*bitmap)[row][column] = iterator.Red();
            ++iterator;
         }
      }
      movie.getFrame(index).setBitmap(bitmap);
   }*/
   return nativeBitmap;
}

namespace {
   wxString makeMarkString(std::size_t i)
   {
      std::stringstream sStream; sStream << i;
      return sStream.str();
   }
}

wxDEFINE_EVENT(myEVT_TRACKING_COMPLETED, wxThreadEvent);

//// <_no_comment_> ////
///
template <std::size_t N>
void createBitmaps(std::string name)
{
   constexpr std::size_t SIZE = 24 * 24 / 8;
   //std::array<std::array<char, SIZE>, N>* xBitMaps =
      //new std::array<std::array<char, SIZE>, N>;
   std::array<std::array<char, SIZE>, N> xBitMaps;

   for (std::size_t i = 0; i < N; ++i)
   {
      //std::array<char, SIZE>& bits = (*xBitMaps)[i];
      std::array<char, SIZE>& bits = xBitMaps[i];
      for (std::size_t j = 0; j < bits.size(); ++j)
         bits[j] = ~bits[j];

      for (std::size_t j = 0 + 3 + 1; j < 24 - 3 + 1; j = j + 3)
         bits[j] = bits[j] ^ std::bitset<8>{"01111110"}.to_ulong();
      for (std::size_t j = 24 + 3 + 2; j < 48 - 3 + 2; j = j + 3)
         bits[j] = bits[j] ^ std::bitset<8>{"01111110"}.to_ulong();
      for (std::size_t j = 48 + 3; j < 72 - 3;) {
         bits[j] = bits[j] ^ std::bitset<8>{"01111110"}.to_ulong(); ++j;
         bits[j] = bits[j] ^ std::bitset<8>{"01111110"}.to_ulong(); ++j;
         bits[j] = bits[j] ^ std::bitset<8>{"01111110"}.to_ulong(); ++j;
      }

      wxBitmap bitmap{bits.data(), 24, 24};

      std::stringstream sStream;
      sStream << std::setfill('0') << std::setw(2) << i;
      bitmap.ConvertToImage().SaveFile(name + sStream.str() + ".bmp", wxBITMAP_TYPE_BMP);
   }

   //delete[] xBitMaps;
}
///
//// </_no_comment_> ////

// vim: tw=90 sw=3 et
