#include <wx/debug.h>   // wxASSERT
#include <wx/sizer.h>
#include <wx/utils.h>   // wxOperatingSystemId wxGetOsVersion()
#include <wx/valtext.h> // wxTextValidator

#include "trackee_box.h"

class FilenameValidator : public wxTextValidator
{
   public:

   FilenameValidator(wxOperatingSystemId = ::wxGetOsVersion(), wxString* value = nullptr);

   // ... implicit copy ctor etc.

   // all validator classes must implement this function, as it's used by control ctors
   virtual wxObject* Clone() const; // override

   // pretty much all functionality is inherited from wxTextValidator
};

TrackeeBox::TrackeeBox(wxWindow* parent, wxWindowID id, const wxPoint& pos,
   const wxSize& size) :
   wxPanel{parent, id, pos, size},
   vBox{new wxBoxSizer{wxVERTICAL}},
   textCtrl{new wxTextCtrl{this, id, "my_trackee", wxDefaultPosition, wxDefaultSize,
      wxTE_PROCESS_ENTER, FilenameValidator{}}},
   listBox{new wxListBox{this, id}}
{
   wxASSERT_MSG(typeid(*textCtrl->GetValidator()) == typeid(FilenameValidator),
      u8"CURSE IT!");

   vBox->Add(textCtrl, 0, wxEXPAND, 0);
   vBox->Add(listBox, 1, wxEXPAND | wxTOP, 1);

   SetSizer(vBox);
   InitDialog();

   //// <_event_handler_mappings_> ////
   ///
   Bind(wxEVT_COMMAND_TEXT_ENTER, &TrackeeBox::onEnter, this, id);
   Bind(wxEVT_KEY_UP, &TrackeeBox::onKeyUp, this);
   ///
   //// </_event_handler_mappings_> ////
}

//// <_event_handler_definitions> ////
///

void TrackeeBox::deleteSelection()
{
   if (listBox->GetSelection() != wxNOT_FOUND) listBox->Delete(listBox->GetSelection());
}

void TrackeeBox::reset()
{
   listBox->Clear();
   textCtrl->SetValue("my_trackee");
   textCtrl->SelectAll();
}

wxString TrackeeBox::getStringSelection() const
{
   return listBox->GetStringSelection();
}

// The wxTextValidator used is not enough; we have to ensure that the string in *textCtrl
// does not collide with any strings in *listBox. On Windows 7, filenames differing only
// in case do collide but NTFS is case-sensitive (unlike VFAT or FAT32).
void TrackeeBox::onEnter(wxCommandEvent& event)
{
   if (Validate() && TransferDataFromWindow())
   {
      // case-insensitive search
      auto position = listBox->FindString(textCtrl->GetValue());
      if (position == wxNOT_FOUND)
      {
         listBox->Insert(textCtrl->GetValue(), 0);
         listBox->SetSelection(0); // Select the newly inserted string; does not cause an
                                   // event to be emitted.
         textCtrl->Clear();
         event.Skip(); // Someone might want to construct a Trackee now.
      }
      else // ...
      {
         listBox->SetSelection(position);
         textCtrl->SelectAll(); // ...

         // Generage a selection event manually:

         event.SetEventType(wxEVT_COMMAND_LISTBOX_SELECTED);
         event.SetEventObject(this->listBox);
         event.SetInt(position);
         // Make sure that if event.GetString() is used at any later point during the
         // processing of this event, it will match the existing string even if a case
         // sensitive comparator is used since we don't allow strings that match an
         // existing one except for letter case.
         event.SetString(listBox->GetString(position));

         // The event will be processed immediately, i.e. when this function returns the
         // event was processed.
         GetEventHandler()->ProcessEvent(event);

         // Don't skip the event!
      }
   }
}

void TrackeeBox::onKeyUp(wxKeyEvent&)
{
   assert(false);
}
///
//// </_event_handler_definitions> ////

FilenameValidator::FilenameValidator(wxOperatingSystemId operatingSystemId, wxString* value) :
   wxTextValidator{wxFILTER_EMPTY | wxFILTER_EXCLUDE_CHAR_LIST, value}
{
   // The chars not accepted in filenames on the operating system (only Windows 7 is
   // supported at this point), which would thus render a string ineligible for being used
   // to map a Trackee.
   std::string charExcludes;

   switch (operatingSystemId)
   {
      case wxOS_WINDOWS_NT:
         charExcludes = u8R"(/\:*?"<>|)"; // C++11 raw string literal (UTF-8); not even
                                          // Vim's syntax highlighting can handle it.
                                          // Edit: now it can \o/
         break;
      // TODO: GNU/Linux.
      default:
         ; // ... throw an exception?
   }

   SetCharExcludes(wxString{charExcludes});
}

wxObject* FilenameValidator::Clone() const
{
   return new FilenameValidator{*this};
}

wxDEFINE_EVENT(EVT_COMMAND_TRACKEEBOX_DELETED, wxCommandEvent);

// vim: tw=90 sw=3 et
