#include "open_movie_wizard.hpp"

#include <string>

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/dirdlg.h>    // wxDirSelector()
#include <wx/filename.h>  // wxFileName
#include <wx/hyperlink.h> // wxHyperlinkCtrl
#include <wx/msgdlg.h>    // wxMessageBox()
#include <wx/sizer.h>
#include <wx/stattext.h>  // wxStaticText
#include <wx/valtext.h>   // wxTextValidator

class OpenMovieWizard::DirDialogPage : public wxWizardPageSimple
{
   public:

   DirDialogPage(OpenMovieWizard* parent, wxString* dir = nullptr,
      const wxFileHistory* dirHistory = nullptr);

   void onButton(wxCommandEvent&); // process a wxEVT_COMMAND_BUTTON_CLICKED

   private:

   struct DirValidator : public wxValidator
   {
      DirValidator(wxString* value) : wxValidator{}, value{value} {}

      DirValidator(const DirValidator& foo) : wxValidator{}, value{foo.value} {}

      virtual bool Validate(wxWindow*);  // override
      virtual bool TransferFromWindow(); // override
      virtual bool TransferToWindow();   // override

      virtual wxObject* Clone() const; // override

      private:

      wxString* value;
   };

   wxComboBox* comboBox;
};

class OpenMovieWizard::RegExCtrlPage : public wxWizardPageSimple
{
   public:

   RegExCtrlPage(OpenMovieWizard* parent, wxString* regEx, wxConfigBase* = nullptr);

   private:

   wxComboBox* comboBox;
};

OpenMovieWizard::OpenMovieWizard(wxWindow* parent, int id, wxString* dir, wxString* regEx,
   wxConfigBase* config, const wxFileHistory* dirHist) :
   wxWizard{parent, id, u8"Open Movie Wizard"},
   dirPage{new DirDialogPage{this, dir, dirHist}},
   regExPage{new RegExCtrlPage{this, regEx, config}}
{
   wxWizardPageSimple::Chain(dirPage, regExPage);

   // Here, "this will enlarge the wizard to fit the biggest page"; see the documentation
   GetPageAreaSizer()->Add(dirPage); // of wxWizard::GetPageAreaSizer().
}

bool OpenMovieWizard::RunWizard()
{
   return wxWizard::RunWizard(dirPage);
}

OpenMovieWizard::DirDialogPage::DirDialogPage(OpenMovieWizard* parent, wxString* dir,
   const wxFileHistory* dirHist) :
   wxWizardPageSimple{parent}
{
   //// <_configuration_information_handling_> ////
   ///
   // Note that the wizard itself does not store any directories; it could if it would
   // save the wxConfig object (that might be reasonable) - this page doesn't store them
   // either (tho, I guess, it could do so from its destructor).

   // Has, as per its documentation, the "full set of std::vector<wxString> compatible
   // methods"; I'll therefore use it as if it were a vector.
   wxArrayString dirs{};

   if (dirHist) // was a wxFileHistory provided?
   {
      for (std::size_t i = 0; i < dirHist->GetCount(); ++i)
         dirs.push_back(dirHist->GetHistoryFile(i));
   }

   // If possible, have the validator of the combo box transfer the most recent directory
   // to it when wxWindow::InitDialog() is called (i.e. as the dialog is being shown).
   if (!dirs.empty()) *dir = dirs[0];
   ///
   //// </_configuration_information_handling_> ////

   wxBoxSizer* topSizer = new wxBoxSizer{wxVERTICAL};

   wxBoxSizer* hBox = new wxBoxSizer{wxHORIZONTAL};
   hBox->Add(new wxStaticText{this, wxID_ANY, "Your directory", wxDefaultPosition,
      wxDefaultSize, wxALIGN_CENTER},
      1, wxALIGN_CENTER_VERTICAL);
   hBox->Add(new wxButton{this, wxID_ANY, "Select Directory"});

   topSizer->Add(new wxStaticText{this, wxID_ANY,
      "This wizard is meant to help you load a movie.\n\n"
      "First, please select the directory containing your movie."});
   topSizer->Add(0, 0, 1); // spacer with proportion of 1 ...
   topSizer->Add(hBox, 0, wxEXPAND);
   topSizer->Add(comboBox = new wxComboBox{this, wxID_ANY, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, dirs, 0, DirValidator{dir}}, 0, wxEXPAND);
   topSizer->Add(0, 0, 1); // ... and another identical spacer
   topSizer->Add(new wxStaticText{this, wxID_ANY, "Click \"Next >\" once you are done!"});

   comboBox->SetFocus();

   topSizer->SetSizeHints(this);
   SetSizer(topSizer);

   Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OpenMovieWizard::DirDialogPage::onButton, this,
      wxID_ANY);
}

void OpenMovieWizard::DirDialogPage::onButton(wxCommandEvent&)
{
   const auto& dir = wxDirSelector(wxEmptyString, comboBox->GetValue(), // read the combo box for the default path
      wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST, // the second style removes the button to create a new directory
      wxDefaultPosition, this); // pass this as the parent

   if (!dir.empty()) // did the user select a directory?
   {
      if (comboBox->FindString(dir) == wxNOT_FOUND) {
         comboBox->Append(dir);
      }
      comboBox->SetValue(dir);
   }
}

OpenMovieWizard::RegExCtrlPage::RegExCtrlPage(OpenMovieWizard* parent, wxString* regEx,
   wxConfigBase* config) : wxWizardPageSimple{parent}
{
   wxControl* regExText;

   //// <_configuration_information_handling_> ////
   ///
   wxArrayString regExes{};
   std::string   helpURL{};

   if (config)
   {
      const auto oldPath = config->GetPath();

      config->SetPath(u8"RegularExpressions"); // relative path
      {
         wxString key;
         long     cookie; // This confusing variable is passed to the wxConfigBase
                          // enumeration functions by non-const reference; presumably in
                          // an attempt of bribery.

         if (config->GetFirstEntry(key, cookie))      // Note that config->GetFirstEntry()
         {                                            // and...
            regExes.push_back(config->Read(key));
            while (config->GetNextEntry(key, cookie)) // ...config->GetNextEntry()
            {                                         // seem to retrieve keys in
               regExes.push_back(config->Read(key));  // lexicographical order
            }
         }
      } config->SetPath(oldPath);

      helpURL = config->Read(u8"RegExHelpURL", u8"");
   }

   // if a URL could be obtained, a hyperlink control is used; otherwise just text
   if (!helpURL.empty()) {
      regExText = new wxHyperlinkCtrl{this, wxID_ANY,
         u8"(Perl-derived) Regular Expression", helpURL};
   } else {
      regExText = new wxStaticText{this, wxID_ANY, "(Perl-derived) Regular Expression"};
   }

   if (!regExes.empty()) *regEx = regExes[0];
   ///
   //// </_configuration_information_handling_> ////

   wxBoxSizer* topSizer = new wxBoxSizer{wxVERTICAL};

   wxBoxSizer* hBox = new wxBoxSizer{wxHORIZONTAL};
   hBox->Add(new wxStaticText{this, wxID_ANY, "Now you can provide a "});
   hBox->Add(regExText);

   topSizer->Add(hBox);
   topSizer->Add(new wxStaticText{this, wxID_ANY,
      "that all images relevant to your movie need to match.\n\n"
      "A few options can be selected from the drop-down list or,\n"
      "to load all files in the directory, the text box may just be\n"
      "left empty."});
   topSizer->Add(0, 0, 1);
   topSizer->Add(new wxStaticText{this, wxID_ANY, "Your regular expression"}, 0,
      wxALIGN_CENTER);
   topSizer->Add(comboBox = new wxComboBox{this, wxID_ANY, wxEmptyString,
      wxDefaultPosition, wxDefaultSize, regExes, 0,
      wxTextValidator{wxFILTER_NONE, regEx}}, 0, wxEXPAND);
   topSizer->Add(0, 0, 1);
   topSizer->Add(new wxStaticText{this, wxID_ANY, "Click \"Finish\" once you are done!"});

   comboBox->SetFocus();

   topSizer->SetSizeHints(this);
   this->SetSizer(topSizer);
}

//// <_DirValidator_member_definitions_> ////
///
bool OpenMovieWizard::DirDialogPage::DirValidator::Validate(wxWindow* parent)
{
   wxTextEntry* window = dynamic_cast<wxTextEntry*>(GetWindow());

   assert(window != nullptr);

   wxString    msg;
   const auto& data = window->GetValue();
   bool        isValid = data != wxEmptyString;

   if (!isValid) {
      msg = u8"Required information entry is empty.";
   } else if (!wxFileName::DirExists(data)) {
      msg = u8"'" + window->GetValue() + u8"' should be an existing directory.";
      isValid = false;
   } else if (!wxFileName::IsDirReadable(data) || !wxFileName::IsDirWritable(data)) {
      msg = u8"'" + window->GetValue() + u8"' should be a directory that this process has"
         "read and write permissions on.";
      isValid = false;
   }

   if (!isValid) {
      ::wxMessageBox(msg, u8"Validation conflict", wxOK | wxICON_EXCLAMATION, parent);
   }

   return isValid;
}

bool OpenMovieWizard::DirDialogPage::DirValidator::TransferFromWindow()
{
   wxTextEntry* window = dynamic_cast<wxTextEntry*>(GetWindow());
   assert(window != nullptr);

   *value = window->GetValue();
   return true;
}

bool OpenMovieWizard::DirDialogPage::DirValidator::TransferToWindow()
{
   wxTextEntry* window = dynamic_cast<wxTextEntry*>(GetWindow());
   assert(window != nullptr);

   window->SetValue(*value);
   return true;
}

wxObject* OpenMovieWizard::DirDialogPage::DirValidator::Clone() const
{
   return new DirValidator{*this};
}
///
//// </_DirValidator_member_definitions_> ////

// vim: tw=90 sts=3 sw=3 et
