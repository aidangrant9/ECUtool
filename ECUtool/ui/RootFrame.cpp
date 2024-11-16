#include "RootFrame.hpp"
#include "icons/icons.h"
#include <format>

RootFrame::RootFrame()
	: wxFrame(nullptr, wxID_ANY, "ECUtool")
{
	wxBitmapBundle ecuBitmapBundle = wxBitmapBundle::FromSVG(ECU_ICON, wxSize(50, 50));
	wxIcon ecuIcon = ecuBitmapBundle.GetIcon(ecuBitmapBundle.GetDefaultSize());
	SetIcon(ecuIcon);

	wxMenu *menuHelp = new wxMenu();
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuHelp, "Help");

	wxToolBar *toolbar = this->CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT, wxID_ANY, "MainToolbar");

	wxBitmapBundle connectIcon = wxBitmapBundle::FromSVG(ECU_ICON_CONNECT, wxSize(30,30));

	wxToolBarToolBase* connectButton = toolbar->AddTool(wxID_ANY, "Connect", connectIcon, "Connect to ECU", wxITEM_NORMAL);

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