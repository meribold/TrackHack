#include <wx/app.h>
#include <wx/fileconf.h> // wxFileConfig
#include <wx/log.h>      // ...

#include "main_frame.hpp"

class App : public wxApp
{
   public:

   virtual bool OnInit(); // Add override when switching to a more recent version of GCC.

   private:

   MainFrame* mainFrame;
};

IMPLEMENT_APP(App)

bool App::OnInit()
{
   SetAppName(u8"TrackHack");
   SetAppDisplayName(u8"TrackHack");

   wxFileName localFileName = wxFileConfig::GetLocalFile("track_hack.ini",
      wxCONFIG_USE_SUBDIR);
   if (!localFileName.FileExists() && // Does the user-specific configuration file exist?
       !localFileName.DirExists())    // If not, does the directory where it would have
                                      // been not exist either?
   {
      // Create that directory.
      bool success = wxFileName::Mkdir(localFileName.GetPath(), wxS_DIR_DEFAULT,
         wxPATH_MKDIR_FULL);

      if (!success) { // Assert success in creating it; otherwise terminate.
         ::wxLogFatalError("Failed to create local configuration directory: "
            "%s\n\nTerminating.", localFileName.GetPath());
      }
   }

   wxConfigBase::Set(new wxFileConfig{u8"TrackHack", wxEmptyString, u8"track_hack.ini",
      u8"track_hack.ini", wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_GLOBAL_FILE |
      wxCONFIG_USE_SUBDIR});

   // No wxInitAllImageHandlers() as for now, only the BMP/DIB file format is supported.

   mainFrame = new MainFrame(wxDefaultPosition, wxSize(640, 720));

   mainFrame->Show(true);
   SetTopWindow(mainFrame);

   return true;
}
