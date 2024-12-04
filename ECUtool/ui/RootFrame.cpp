#include "RootFrame.hpp"
#include "icons/icons.hpp"
#include <format>
#include "../communication/KWP2000_Message.hpp"


RootFrame::RootFrame(const wxString &title, const wxPoint &position, const wxSize &size)
	: wxFrame(nullptr, wxID_ANY, title, position, size)
{
	// Create sizer for root frame
	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizerAndFit(sizer);
	this->SetMinSize(wxSize(size));

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

	/*
	// Add status bar
	wxStatusBar *statusBar = new wxStatusBar();
	statusBar->SetStatusText("Status bar");
	SetStatusBar(statusBar);
	*/

	// Initialise toolbar and add connect tool
	wxToolBar *toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_DEFAULT_STYLE | wxTB_TEXT);
	sizer->Add(toolbar, 0, wxEXPAND);
	wxBitmapBundle connectIcon = wxBitmapBundle::FromSVG(ECU_ICON_CONNECT, wxSize(25, 25));
	toolbar->AddTool(wxID_ANY, "Connect", connectIcon, "Start connection to an ECU", wxITEM_NORMAL);
	toolbar->Realize();
	toolbar->Show();

	// Create main control panel and sizer
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(600,400));
	wxSizer *panelSizer = new wxBoxSizer(wxHORIZONTAL);
	panel->SetSizerAndFit(panelSizer);
	sizer->Add(panel, 1, wxEXPAND);

	// Add output textctrl for logging
	wxTextCtrl *outputTextCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxTE_CHARWRAP, wxDefaultValidator, "Log");
	panelSizer->Add(outputTextCtrl, 1, wxEXPAND);

	// Register and set textctrl as log target
	wxLogTextCtrl *logTextCtrl = new wxLogTextCtrl(outputTextCtrl);
	wxLog::SetActiveTarget(logTextCtrl);

	std::vector<uint8_t> g{ 0b11010000, 0x23, 0x24, 0xA1,0xF1,0xA1,0xA1,0xA1,0xA1,0xA1,0xB2,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1,0xA1, 0x93 };
	KWP2000Message m(g);
	std::cout << m.print();
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());

	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	wxLogStatus("%s\n", m.print().c_str());
	
}

void RootFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox(std::format("ECUtool\nVersion {}", ECUtool_VERSION_STRING),
		"About", wxOK | wxICON_INFORMATION);
}