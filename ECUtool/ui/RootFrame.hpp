#pragma once

#include <wx/wx.h>

class RootFrame : public wxFrame 
{
public:
	RootFrame();

private:
	void OnAbout(wxCommandEvent &event);
};