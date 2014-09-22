#include "WorldView.hpp"
#include "World.hpp"
#include "Tool.hpp"
#include "MainFrame.hpp"

#include <sstream>

WorldView::WorldView(wxPanel* parent, MainFrame *iMainFrame, int *gl_attrib) :
	wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxFULL_REPAINT_ON_RESIZE, _T("WorldView"), gl_attrib), mMain(iMainFrame), mWorld(new World()), mRenderMode(RM_HALOED_LINE) {
}
void WorldView::newWorld() {
	mWorld.reset( new World() );
	mTool.reset();
	Refresh(false);
}

void WorldView::selectRenderMode(RenderMode iRenderMode) {
	mRenderMode = iRenderMode;
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
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
	
	setupOpenGl();
	doPaint();

    glFlush();
    SwapBuffers();
}

void WorldView::setupOpenGl() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(1, 1, 1, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
		const GLfloat aspect = (GLfloat)w/(GLfloat)h;
        gluPerspective(45.0f,aspect,0.1f,100.0f);
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

	const ::lunarlady::real x = 2.0f*(::lunarlady::math::limitRange(0.0f, (::lunarlady::real)event.GetX() / (::lunarlady::real)w, 1.0f)-0.5f);
	const ::lunarlady::real y = 2.0f*(::lunarlady::math::limitRange(0.0f, (::lunarlady::real)event.GetY() / (::lunarlady::real)h, 1.0f)-0.5f);
	const ::lunarlady::math::vec2 movement(x, y);
	
	SetCurrent();
	
	
	/*std::stringstream str;
	str << "location: " << event.GetX() << "/" << w << ", " << event.GetY() << "/" << h;
	mMain->setInfoText( str.str() );*/

	mWorld->setMousePosition(::lunarlady::math::vec2(event.GetX(),h-event.GetY()));
	if( updateCurrentTool(movement, event) ) {
		Refresh(false);
	}
	mWorld->setMousePosition(::lunarlady::math::vec2(event.GetX(),h-event.GetY()));
}

void WorldView::selectTool(Tool* iTool) {
	mTool.reset( iTool );
	mWorld->selectToolAction( iTool->getToolAction() );
	Refresh(false);
}

World& WorldView::getWorld() {
	assert( mWorld.get() );
	return *(mWorld.get());
}

bool WorldView::updateCurrentTool(const ::lunarlady::math::vec2& movement, wxMouseEvent& event) {
	static bool mDragging = false;
	static ::lunarlady::math::vec2 last(0,0);

	bool refresh = getWorld().defaultRefreshValue();

	{
		int rotation = event.GetWheelRotation();
		const int delta = event.GetWheelDelta();
		const ::lunarlady::real power = event.ShiftDown()? 1.0f : 0.5f;
		if( rotation > 0 ) {
			for(; rotation >= delta; rotation -= delta) {
				mWorld->increaseZoom( power );
				refresh = true;
			}
		}
		else if( rotation < 0 ) {
			for(; rotation <= -delta; rotation += delta) {
				mWorld->increaseZoom( -power );
				refresh = true;
			}
		}
	}

	if( event.MiddleIsDown() ) {
		if(!mDragging ) {
			mDragging = true;
		}
		else {
			::lunarlady::math::vec2 move = movement - last;
			if( event.ShiftDown() ) {
				mWorld->changePosition(move * 6);
			}
			else {
				mWorld->changeRotation(-move * 2);
			}
			refresh = true;
		}
		last = movement;;
	}
	else {
		mDragging = false;
	}

	mWorld->mRotating = mDragging;

	if( mTool.get() ) {
		if( mTool->onInput(movement, event) ) {
			refresh = true;
		}
	}

	return refresh;
}

void WorldView::doPaint() {
	mWorld->render(mRenderMode);
}