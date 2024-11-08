#pragma once

#include <wx/wx.h>
#include <format>

class RootFrame : public wxFrame 
{
public:
	RootFrame();

private:
	void OnAbout(wxCommandEvent& event);
};