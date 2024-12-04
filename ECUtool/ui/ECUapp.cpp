#include "ECUapp.hpp"
#include "RootFrame.hpp"

bool ECUapp::OnInit()
{
	RootFrame *rootFrame = new RootFrame("ECUtool", wxDefaultPosition, wxSize(600,350));
	rootFrame->Show(true);
	return true;
}