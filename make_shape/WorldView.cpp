#include "make_shape/WorldView.hpp"
#include "make_shape/World.hpp"
#include "make_shape/Tool.hpp"
#include "make_shape/MainFrame.hpp"

#include "lunarlady/math/vec2.hpp"

#include <sstream>

const int spaceWidth = 30;
const int spaceWidth_half = spaceWidth/2;

const int spaceHeight = 30;
const int spaceHeight_half = spaceHeight/2;

WorldView::WorldView(wxPanel* parent, MainFrame *iMainFrame, int *gl_attrib) :
	wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxFULL_REPAINT_ON_RESIZE, _T("WorldView"), gl_attrib), mMain(iMainFrame), mWorld(new World()) {
}
void WorldView::newWorld() {
	mWorld.reset( new World() );
	mTool.reset();
	Refresh(false);
}

void WorldView::OnPaint(wxPaintEvent& event) {
	// This is a dummy, to avoid an endless succession of paint messages
	// OnPaint handlers must always create a wxPaintDC.
    wxPaintDC dc(this);
    
#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif
    SetCurrent(); // make opengl calls happen on this canvas
	glLoadIdentity();
	
	setupOpenGl();
	doPaint();

    glFlush();
    SwapBuffers();
}

void WorldView::setupOpenGl() {
	glClearColor(0, 0, 0, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
}

void WorldView::OnSize(wxSizeEvent& event) {
	// this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);
	int w, h;
    GetClientSize(&w, &h);
    if( h==0 ) {
    	h=1;
    }
#ifndef __WXMOTIF__
    if (GetContext())
#endif
    {
        SetCurrent();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
        glViewport(0, 0, (GLint) w, (GLint) h);
        glMatrixMode(GL_MODELVIEW);

		setupOpenGl();
    }
}
void WorldView::OnEraseBackground(wxEraseEvent& event) {
	// Do nothing, to avoid flashing.
}

WorldView* WorldView::build(wxPanel* parent, MainFrame *iMainFrame) {
		#ifdef __WXMSW__
			//int *gl_attrib = NULL;
			int gl_attrib[] = { WX_GL_RGBA, WX_GL_MIN_RED, 1, WX_GL_MIN_GREEN, 1,
				WX_GL_MIN_BLUE, 1, WX_GL_DEPTH_SIZE, 32,
				WX_GL_DOUBLEBUFFER, NULL};
		#else
			int gl_attrib[20] = { WX_GL_RGBA, WX_GL_MIN_RED, 1, WX_GL_MIN_GREEN, 1,
				WX_GL_MIN_BLUE, 1, WX_GL_DEPTH_SIZE, 1,
				WX_GL_DOUBLEBUFFER,
			#  ifdef __WXMAC__
				GL_NONE };
			#  else
				None };
			#  endif
		#endif
		return new WorldView(parent, iMainFrame, gl_attrib);
}

BEGIN_EVENT_TABLE(WorldView, wxGLCanvas)
    EVT_SIZE(WorldView::OnSize)
    EVT_PAINT(WorldView::OnPaint)
    EVT_ERASE_BACKGROUND(WorldView::OnEraseBackground)
	EVT_MOUSE_EVENTS(WorldView::OnMouseEvent)
END_EVENT_TABLE()

void WorldView::OnMouseEvent(wxMouseEvent& event) {
	int w, h;
    GetClientSize(&w, &h);

	const ::lunarlady::real x = event.GetX();
	const ::lunarlady::real y = event.GetY();
	const ::lunarlady::math::vec2 movement(x-spaceWidth_half, (h-y) - spaceHeight_half);
	
	SetCurrent();
	mWorld->setCursorLocation( w-spaceWidth, h-spaceHeight, movement );

	if( event.ButtonUp(wxMOUSE_BTN_LEFT) ) {
		mWorld->onLeft();
	}

	if( event.ButtonUp(wxMOUSE_BTN_RIGHT) ) {
		mWorld->onRight();
	}
	
	/*std::stringstream str;
	str << "location: " << event.GetX() << "/" << w << ", " << event.GetY() << "/" << h;
	mMain->setInfoText( str.str() );*/
	if( updateCurrentTool(movement, event) ) {
		Refresh(false);
	}
}

void WorldView::selectTool(Tool* iTool) {
	mTool.reset( iTool );
	Refresh(false);
}

World& WorldView::getWorld() {
	assert( mWorld.get() );
	return *(mWorld.get());
}

bool WorldView::updateCurrentTool(const ::lunarlady::math::vec2& movement, wxMouseEvent& event) {
	static bool mDragging = false;
	static ::lunarlady::math::vec2 last(0,0);

	bool refresh = true; //getWorld().defaultRefreshValue();

	if( mTool.get() ) {
		if( mTool->onInput(movement, event) ) {
			refresh = true;
		}
	}

	return refresh;
}

void WorldView::doPaint() {
	int w, h;
    GetClientSize(&w, &h);
	mWorld->renderBorder(spaceWidth_half, spaceHeight_half, w-spaceWidth, h-spaceHeight);
	mWorld->render();
}