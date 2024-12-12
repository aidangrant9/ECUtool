#include "ConnectFrame.hpp"
#include <format>
#include <Windows.h>
#include "../communication/KLineConnection.hpp"

#define PORT_COUNT 255

ConnectFrame::ConnectFrame(RootFrame *parent, wxWindowID id, SerialConnection **connection)
	: wxFrame(parent, id, "Start a new connection", wxDefaultPosition, wxSize(300,100)), parent(parent), connection(connection)
{
	// Create panel and sizer for the controls
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *panelSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizerAndFit(panelSizer);

	
	// Get available com ports
	std::vector<ULONG> portNumbers;
	portNumbers.resize(PORT_COUNT);
	ULONG portsFound = 0;
	GetCommPorts(portNumbers.data(), PORT_COUNT, &portsFound);
	portNumbers.resize(portsFound);

	std::vector<wxString> comChoices;
	comChoices.reserve(portsFound);

	for (ULONG &i : portNumbers)
	{
		wxString str(std::format("COM{}", i).c_str());
		comChoices.push_back(str);
	}

	// Create choicebox
	this->comSelect = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, comChoices.size(), comChoices.data(), 0, wxDefaultValidator, "COM port");
	panelSizer->Add(comSelect, 1, wxEXPAND | wxALL, 5);	

	wxWindowID connectButtonID = wxWindow::NewControlId();
	wxButton *connectButton = new wxButton(panel, connectButtonID, "Connect", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Connect");
	panelSizer->Add(connectButton, 3, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	Bind(wxEVT_CLOSE_WINDOW, &ConnectFrame::OnClose, this);
	Bind(wxEVT_BUTTON, &ConnectFrame::OnConnectButton, this, connectButtonID);
}

void ConnectFrame::OnConnectButton(wxEvent &event)
{
	/*
		For now always return Kline with 9600baud
	*/

	std::string name(this->comSelect->GetStringSelection());
	
	if (!name.empty())
	{
		*this->connection = new KLineConnection(std::string(this->comSelect->GetStringSelection()), 9600);
	}

	parent->OnConnectFrameClose();
	Destroy();
}

void ConnectFrame::OnClose(wxEvent &event)
{
	parent->OnConnectFrameClose();
	Destroy();
}