#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <wx/debug.h>   // wxASSERT
#include <wx/menu.h>
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
   textCtrl{new wxTextCtrl{this, id, "1", wxDefaultPosition, wxDefaultSize,
      wxTE_PROCESS_ENTER, FilenameValidator{}}},
   listBox{new wxListBox{this, id}}
{
   vBox->Add(textCtrl, 0, wxEXPAND, 0);
   vBox->Add(listBox, 1, wxEXPAND | wxTOP, 1);

   SetSizer(vBox);
   InitDialog();

   //listBox->SetCanFocus(false);

   //// <_event_handler_mappings_> ////
   ///
   textCtrl->Bind(wxEVT_SET_FOCUS, &TrackeeBox::onFocus, this, id);
   listBox->Bind(wxEVT_SET_FOCUS, &TrackeeBox::onFocus, this, id);
   textCtrl->Bind(wxEVT_KILL_FOCUS, &TrackeeBox::onFocus, this, id);
   //Bind(wxEVT_COMMAND_TEXT_ENTER, &TrackeeBox::onEnter, this, id);
   Bind(wxEVT_KEY_UP, &TrackeeBox::onKeyUp, this);
   listBox->Bind(wxEVT_CONTEXT_MENU, &TrackeeBox::onContextMenu, this);
   ///
   //// </_event_handler_mappings_> ////
}

//// <_event_handler_definitions> ////
///

// The wxTextValidator used is not enough; we have to ensure that the string in *textCtrl
// does not collide with any strings in *listBox. On Windows 7, filenames differing only
// in case do collide but NTFS is case-sensitive (unlike VFAT or FAT32).
std::string TrackeeBox::addTrackee()
{
   if (Validate() && TransferDataFromWindow())
   {
      // case-insensitive search
      auto position = listBox->FindString(textCtrl->GetValue());
      if (position == wxNOT_FOUND)
      {
         auto trackeeId = textCtrl->GetValue().ToStdString();
         listBox->Insert(trackeeId, 0);

         suggestId();

         return trackeeId;
      }
      else // ...
      {
         listBox->SetSelection(position);
         textCtrl->SelectAll(); // ...

         // Generage a selection event manually:
         wxCommandEvent event{wxEVT_COMMAND_LISTBOX_SELECTED, listBox->GetId()};
         event.SetEventObject(listBox);
         event.SetInt(position);
         // Make sure that if event.GetString() is used at any later point during the
         // processing of this event, it will match the existing string even if a case
         // sensitive comparator is used since we don't allow strings that match an
         // existing one except for letter case.
         event.SetString(listBox->GetString(position));

         // The event will be processed immediately, i.e. when this function returns the
         // event was processed. Also, this method doesn't transfer ownership of the event
         // (unlike wxEventHandler::QueueEvent()).
         GetEventHandler()->ProcessEvent(event);
      }
   }
   return std::string{};
}

void TrackeeBox::deleteTrackee(unsigned n)
{
   listBox->Delete(n);
   suggestId();
}

void TrackeeBox::deleteSelection()
{
   auto selection = listBox->GetSelection();
   if (selection != wxNOT_FOUND) {
      deleteTrackee(selection);
      if (!listBox->IsEmpty()) {
         if (static_cast<unsigned>(selection) < listBox->GetCount()) {
            listBox->SetSelection(selection);
         }
         else {
            listBox->SetSelection(selection - 1);
         }
      }
   }
}

void TrackeeBox::reset()
{
   listBox->Clear();
   suggestId();
}

wxString TrackeeBox::getStringSelection() const
{
   return listBox->GetStringSelection();
}

void TrackeeBox::onFocus(wxFocusEvent& event)
{
   if (event.GetEventType() == wxEVT_SET_FOCUS) {
      if (event.GetEventObject() == textCtrl) {
         listBox->SetSelection(wxNOT_FOUND);
      }
   }
   /*
   else if (event.GetEventType() == wxEVT_KILL_FOCUS) {

   }
   */
   // "The focus event handlers should almost invariably call wxEvent::Skip() on their
   // event argument to allow the default handling to take place."
   event.Skip();
}

/*
void TrackeeBox::onEnter(wxCommandEvent& event)
{
   if (!addTrackee().empty())
   {
      // Select the newly inserted string; does not cause an event to be emitted.
      listBox->SetSelection(0);

      wxCommandEvent newEvent{myEVT_COMMAND_TRACKEEBOX_ADDED, GetId()};
      newEvent.SetEventObject(this);
      newEvent.SetInt(0);
      newEvent.SetString(listBox->GetString(0));
      GetEventHandler()->ProcessEvent(newEvent); // synchronous

   }
   event.Skip();
}
*/

void TrackeeBox::onKeyUp(wxKeyEvent&)
{
   assert(false);
}

void TrackeeBox::onContextMenu(wxContextMenuEvent&)
{
   auto mousePos = listBox->ScreenToClient(::wxGetMousePosition());
   auto index = listBox->HitTest(mousePos);

   if (index != wxNOT_FOUND) {
      wxMenu menu;
      menu.Append(wxID_DELETE);
      menu.Bind(wxEVT_COMMAND_MENU_SELECTED, [this, index](wxCommandEvent&) {
            wxCommandEvent* event =
               new wxCommandEvent{myEVT_COMMAND_TRACKEEBOX_DELETED, GetId()};
            event->SetEventObject(this);
            event->SetString(listBox->GetString(index));
            deleteTrackee(index);
            ::wxQueueEvent(GetEventHandler(), event);
         }, wxID_DELETE);
      PopupMenu(&menu);
   }
}
///
//// </_event_handler_definitions> ////

void TrackeeBox::suggestId()
{
   std::vector<unsigned> indices{};
   indices.reserve(listBox->GetCount());
   for (unsigned i = 0; i < listBox->GetCount(); ++i) {
      try {
         std::string trackeeKey = listBox->GetString(i).ToStdString();
         int index = std::stoi(trackeeKey);
         if (index > 0 && std::to_string(index) == trackeeKey) {
            indices.push_back(unsigned(index));
         }
      }
      catch (std::invalid_argument) {

      }
      catch (std::out_of_range) {

      }
   }
   std::sort(indices.begin(), indices.end());
   unsigned minFreeIndex = 1;
   for (const auto& i : indices) {
      if (minFreeIndex == i)
         ++minFreeIndex;
      else
         break;
   }

   // Does not generate a wxEVT_TEXT event; otherwise identical to SetValue().
   textCtrl->ChangeValue(std::to_string(minFreeIndex));
   textCtrl->SelectAll();

   //textCtrl->SetHint(std::to_string(minFreeIndex));
}

FilenameValidator::FilenameValidator(wxOperatingSystemId operatingSystemId,
   wxString* value) : wxTextValidator{wxFILTER_EMPTY | wxFILTER_EXCLUDE_CHAR_LIST, value}
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

wxDEFINE_EVENT(myEVT_COMMAND_TRACKEEBOX_ADDED, wxCommandEvent);
wxDEFINE_EVENT(myEVT_COMMAND_TRACKEEBOX_DELETED, wxCommandEvent);

// vim: tw=90 sw=3 et
