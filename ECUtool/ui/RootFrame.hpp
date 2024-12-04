#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>

class RootFrame : public wxFrame 
{
public:
	RootFrame(const wxString &title, const wxPoint &position, const wxSize &size);

private:
	void OnAbout(wxCommandEvent &event);
};