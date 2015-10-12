#ifndef OPEN_MOVIE_WIZARD_H
#define OPEN_MOVIE_WIZARD_H

#include <wx/fileconf.h> // wxFileConfig
#include <wx/filehistory.h> // wxFileHistory
#include <wx/wizard.h>

class OpenMovieWizard : public wxWizard
{
   public:

   OpenMovieWizard(wxWindow* parent, int id = wxID_ANY, wxString* dir = nullptr,
      wxString* regEx = nullptr, wxConfigBase* = nullptr,
      const wxFileHistory* dirHistory = nullptr);

   // Execute the wizard starting at dirPage; return true if it was successfully finished,
   bool RunWizard(); // false if the user cancelled it.

   private:

   class DirDialogPage;
   class RegExCtrlPage;

   DirDialogPage* dirPage;
   RegExCtrlPage* regExPage;
};

#endif //OPEN_MOVIE_WIZARD_H

// vim: tw=90 sts=3 sw=3 et
