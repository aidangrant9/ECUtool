#include "RootFrame.hpp"
#include "icons/icons.hpp"
#include <format>
#include "../communication/KWP2000_Message.hpp"


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

	// Create command menu
	wxMenu *menuCommands = new wxMenu();
	wxMenuItem *exampleCommand = new wxMenuItem(menuCommands, -2, "Command example", wxEmptyString, wxITEM_NORMAL, nullptr);

	Bind(wxEVT_MENU, &RootFrame::OnAbout, this, wxID_ABOUT);

	// Add menu bar
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menuCommands, "Commands");
	menuBar->Append(menuHelp, "Help");
	SetMenuBar(menuBar);

	// Initialise toolbar and add connect tool
	wxToolBar *toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_DEFAULT_STYLE | wxTB_TEXT);
	wxBitmapBundle connectIcon = wxBitmapBundle::FromSVG(ECU_ICON_CONNECT, wxSize(25, 25));
	toolbar->AddTool(wxID_ANY, "Connect", connectIcon, "Start connection to an ECU", wxITEM_NORMAL);
	toolbar->Realize();
	toolbar->Show();


	wxLogWindow *window = new wxLogWindow(this, "Log", true, true);
	std::vector<uint8_t> g{ 0b11010000, 0x23, 0x24, 0xA1,0xF1,0xA1,0xA1,0xA1,0xA1,0xA1,0xB2,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1, 0x93 };
	KWP2000Message m(g);
	std::cout << m.print();
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%d\n", m.calcChecksum());
	
}

void RootFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox(std::format("ECUtool\nVersion {}", ECUtool_VERSION_STRING),
		"About", wxOK | wxICON_INFORMATION);
}