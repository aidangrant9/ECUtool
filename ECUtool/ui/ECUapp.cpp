#include "ECUapp.hpp"
#include "RootFrame.hpp"

bool ECUapp::OnInit()
{
	RootFrame* rootFrame = new RootFrame();
	rootFrame->Show(true);
	return true;
}