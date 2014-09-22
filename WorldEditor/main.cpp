#include "MainFrame.hpp"

class WorldEditor : public wxApp {
public:
	virtual bool OnInit();
};

DECLARE_APP(WorldEditor)
IMPLEMENT_APP(WorldEditor)

bool WorldEditor::OnInit() {
	MainFrame* frame = new MainFrame();
	//frame->init();
	frame->Show(true);
	SetTopWindow(frame);
	return true;
}