#include "RootFrame.hpp"

RootFrame::RootFrame()
	: wxFrame(nullptr, wxID_ANY, "ECUtool")
{
	wxMenu *menuAbout = new wxMenu();
	menuAbout->Append(wxID_ABOUT);

	wxMenu* menuExit = new wxMenu();
	menuExit->Append(wxID_EXIT);

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(menuAbout, "About");

	SetMenuBar(menuBar);
	
	Bind(wxEVT_MENU, &RootFrame::OnAbout, this, wxID_ABOUT);
}

void RootFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(std::format("ECUtool\nVersion {}", ECUtool_VERSION_STRING),
		"About", wxOK | wxICON_INFORMATION);
}