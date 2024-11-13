#include "RootFrame.hpp"
#include "icons/icons.h"
#include <format>

RootFrame::RootFrame()
	: wxFrame(nullptr, wxID_ANY, "ECUtool")
{
	wxMenu *menuAbout = new wxMenu();
	menuAbout->Append(wxID_ABOUT);

	wxMenu *menuExit = new wxMenu();
	menuExit->Append(wxID_EXIT);

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuAbout, "About");

	wxToolBar *toolbar = this->CreateToolBar(wxTB_HORIZONTAL, wxID_ANY, "MainToolbar");

	wxBitmapBundle connectIcon = wxBitmapBundle::FromSVG(ECU_ICON_CONNECT, wxSize(25,25));

	wxToolBarToolBase* connectButton = toolbar->AddTool(wxID_ANY, "Connect", connectIcon, "wxEmptyString", wxITEM_NORMAL);

	toolbar->Realize();

	toolbar->Show();
	
	SetMenuBar(menuBar);
	
	Bind(wxEVT_MENU, &RootFrame::OnAbout, this, wxID_ABOUT);
}

void RootFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox(std::format("ECUtool\nVersion {}", ECUtool_VERSION_STRING),
		"About", wxOK | wxICON_INFORMATION);
}