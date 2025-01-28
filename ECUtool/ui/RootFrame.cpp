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
	wxWindowID sdsID = wxWindow::NewControlId();
	wxMenuItem *sds = new wxMenuItem(menuCommands, sdsID, "StartDiagnosticSession", "Example", wxITEM_NORMAL, nullptr);
	menuCommands->Append(sds);

	wxWindowID trID = wxWindow::NewControlId();
	wxMenuItem *tr = new wxMenuItem(menuCommands, trID, "TesterPresent", "Example", wxITEM_NORMAL, nullptr);
	menuCommands->Append(tr);


	Bind(wxEVT_MENU, &RootFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &RootFrame::OnSDS, this, sdsID);
	Bind(wxEVT_MENU, &RootFrame::OnTR, this, trID);

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

void RootFrame::OnSDS(wxCommandEvent &event)
{
	if (this->connection != nullptr)
	{
		connection->writeMessage(std::vector<uint8_t>{0b10000000, 0x20, 0xf7, 0x02, 0x10, 0x00, 0xd6});
	}
}

void RootFrame::OnTR(wxCommandEvent &event)
{
	if (this->connection != nullptr)
	{
		connection->writeMessage(std::vector<uint8_t>{0b10000000, 0x20, 0xf7, 0x01, 0x3E, 0xe9});
	}
}
