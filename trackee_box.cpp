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

TrackeeBox::TrackeeBox(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) :
	wxPanel{parent, id, pos, size},
	vBox{new wxBoxSizer{wxVERTICAL}},
	textCtrl{new wxTextCtrl{this, id, "my_trackee", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER,
		FilenameValidator{}}},
	listBox{new wxListBox{this, id, wxDefaultPosition, wxDefaultSize}}
{
	wxASSERT_MSG(typeid(*textCtrl->GetValidator()) == typeid(FilenameValidator), u8"CURSE IT!");

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

// the wxTextValidator used is not enough; now it has to be ensured that the string in *textCtrl does not
// collide with any strings in *listBox. On Windows 7 filenames differing only in case do collide;
void TrackeeBox::onEnter(wxCommandEvent& event) // NTFS is case-sensitive however (unlike VFAT or FAT32)
{
	if (Validate() && TransferDataFromWindow())
	{
		auto position = listBox->FindString(textCtrl->GetValue()); // case insensitive search
		if (position == wxNOT_FOUND)
		{
			listBox->Insert(textCtrl->GetValue(), 0);
			listBox->SetSelection(0); // the newly inserted string is selected; does not cause an event to be emitted
			textCtrl->Clear();
			event.Skip(); // someone might want to construct a Trackee now
		}
		else // ...
		{
			listBox->SetSelection(position);
			textCtrl->SelectAll(); // ...

			// a selection event is generated manually:

			event.SetEventType(wxEVT_COMMAND_LISTBOX_SELECTED);
			event.SetEventObject(this->listBox);
			event.SetInt(position);
			// in case event.GetString() is used at any later point during the processing of this event,
			// it will match an existing string even if a case sensitive comparator is used;
			// i.e. any behaviour will conform with our decision to disallow strings deviating only in case
			event.SetString(listBox->GetString(position));

			// the event will be processed immediately, i.e. when this function returns the event was processed
			GetEventHandler()->ProcessEvent(event);

			// don't skip the event!
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
	// the chars not accepted in filenames on the operating system (only Windows 7 is supported at this point),
	// which would thus render a string ineligible for being used to map a Trackee
	std::string charExcludes;

	switch (operatingSystemId)
	{
		case wxOS_WINDOWS_NT:
			charExcludes = u8R"(/\:*?"<>|)"; // C++11 raw string literal (UTF-8);
			                                 // to edgy even for vims syntax highlighting (for now)   :D
			break;
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
