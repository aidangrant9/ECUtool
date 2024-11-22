#pragma once

#include <wx/wx.h>
#include <wx/aui/aui.h>

class RootFrame : public wxFrame 
{
public:
	RootFrame();

private:
	void OnAbout(wxCommandEvent &event);
};