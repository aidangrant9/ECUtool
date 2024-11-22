#include "RootFrame.hpp"
#include "icons/icons.h"
#include <format>

RootFrame::RootFrame()
	: wxFrame(nullptr, wxID_ANY, "ECUtool")
{
	// Add icon to the window
	wxBitmapBundle ecuBitmapBundle = wxBitmapBundle::FromSVG(ECU_ICON, wxSize(50, 50));
	wxIcon ecuIcon = ecuBitmapBundle.GetIcon(ecuBitmapBundle.GetDefaultSize());
	SetIcon(ecuIcon);

	// Create help menu and add predefined about button
	wxMenu *menuHelp = new wxMenu();
	menuHelp->Append(wxID_ABOUT);

	Bind(wxEVT_MENU, &RootFrame::OnAbout, this, wxID_ABOUT);

	// Add menu bar
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuHelp, "Help");
	SetMenuBar(menuBar);

	// Initialise toolbar and add connect tool
	wxToolBar *toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_DEFAULT_STYLE | wxTB_TEXT);
	wxBitmapBundle connectIcon = wxBitmapBundle::FromSVG(ECU_ICON_CONNECT, wxSize(25, 25));
	toolbar->AddTool(wxID_ANY, "Connect", connectIcon, "Start connection to an ECU", wxITEM_NORMAL);
	toolbar->Realize();
	toolbar->Show();


}

void RootFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox(std::format("ECUtool\nVersion {}", ECUtool_VERSION_STRING),
		"About", wxOK | wxICON_INFORMATION);
}