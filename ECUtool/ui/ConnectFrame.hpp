#pragma once

#include <wx/wx.h>
#include "../communication/SerialConnection.hpp"
#include "RootFrame.hpp"

class ConnectFrame : public wxFrame
{
public:
	ConnectFrame(RootFrame *parent, wxWindowID id, SerialConnection **connection);

private:
	RootFrame *parent = nullptr;
	wxChoice *comSelect = nullptr;
	SerialConnection **connection = nullptr;

	void OnClose(wxEvent &event);
	void OnConnectButton(wxEvent &event);
};