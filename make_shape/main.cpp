#include "make_shape/MainFrame.hpp"
#include "IL/il.h"

class WorldEditor : public wxApp {
public:
	virtual bool OnInit();
};

DECLARE_APP(WorldEditor)
IMPLEMENT_APP(WorldEditor)

bool WorldEditor::OnInit() {

	ilInit();

	MainFrame* frame = new MainFrame();
	//frame->init();
	frame->Show(true);
	SetTopWindow(frame);
	return true;
}