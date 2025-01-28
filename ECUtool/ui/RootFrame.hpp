#pragma once

#include <wx/wx.h>
#include "../communication/SerialConnection.hpp"
#include <vector>
#include <cstdint>

class RootFrame : public wxFrame 
{
public:
	RootFrame(const wxString &title, const wxPoint &position, const wxSize &size);
	void OnConnectFrameClose();
	void OnMsgRecieve(std::vector<uint8_t> *msg);

private:
	void OnAbout(wxCommandEvent &event);
	void OnSDS(wxCommandEvent &event);
	void OnTR(wxCommandEvent &event);
	void OnConnectButton(wxCommandEvent &event);

	SerialConnection *connection = nullptr;
	wxWindow *connectFrame = nullptr;
};