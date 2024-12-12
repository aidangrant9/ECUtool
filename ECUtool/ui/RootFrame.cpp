#include "RootFrame.hpp"
#include "icons/icons.hpp"
#include <format>
#include "../communication/KWP2000_Message.hpp"
#include "ConnectFrame.hpp"
#include <functional>
#include <sstream>
#include<iomanip>



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

	wxWindowID connectButtonID = wxWindow::NewControlId();
	toolbar->AddTool(connectButtonID, "Connect", connectIcon, "Start connection to an ECU", wxITEM_NORMAL);
	toolbar->Realize();
	toolbar->Show();

	Bind(wxEVT_TOOL, &RootFrame::OnConnectButton, this, connectButtonID);



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
}

void RootFrame::OnConnectButton(wxCommandEvent &event)
{
	if (this->connectFrame == nullptr)
	{
		this->connectFrame = new ConnectFrame(this, wxID_ANY, &this->connection);
		this->connectFrame->Show();
	}
	else
	{
		this->connectFrame->Show();
	}
}

void RootFrame::OnConnectFrameClose()
{
	this->connectFrame = nullptr;

	if (this->connection != nullptr)
	{
		connection->connect();
		if (connection->connected)
		{
			connection->registerCallback(std::bind(&RootFrame::OnMsgRecieve, this, std::placeholders::_1));
			connection->writeMessage(std::vector<uint8_t>{0b10000000, 0x14, 0x00, 0x20, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0x06, 0x07, 0x08, 0x09
				,0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
				0x28, 0x29, 0x30, 0x31, 0x32, 0xce });
		}
	}
}

void RootFrame::OnMsgRecieve(std::vector<uint8_t> *msg)
{
	std::stringstream str;

	str << std::uppercase << std::setw(2) << std::setfill('0') << std::hex;

	for (uint8_t i : *msg)
	{
		str << +i << " ";
	}

	KWP2000Message m(*msg);
	wxLogStatus("%s", m.print().c_str());
	wxLogStatus("RAW %s", str.str().c_str());
	wxLogStatus("CHECKSUM %x\n", m.calcChecksum());
}

void RootFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox(std::format("ECUtool\nVersion {}", ECUtool_VERSION_STRING),
		"About", wxOK | wxICON_INFORMATION);
}