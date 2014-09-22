#ifndef WORLD_VIEW_HPP
#define WORLD_VIEW_HPP

#include "wx.hpp"
#include "wx_opengl.hpp"

#include "lunarlady/math/vec2.hpp"
#include "WorldEditor/RenderMode.hpp"
#include <memory>

class World;
class Tool;
class MainFrame;

class WorldView : public wxGLCanvas {
public:
	WorldView(wxPanel* parent, MainFrame *iMainFrame, int *gl_attrib);

	static WorldView* build(wxPanel* parent, MainFrame *iMainFrame);

	void setupOpenGl();

	void selectTool(Tool* iTool);

	World& getWorld();

	void newWorld();

	void selectRenderMode(RenderMode iRenderMode);
protected:
	void doPaint();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	bool updateCurrentTool(const ::lunarlady::math::vec2& movement, wxMouseEvent& event);
private:
	DECLARE_EVENT_TABLE()

	MainFrame* mMain;

	RenderMode mRenderMode;

	std::auto_ptr<World> mWorld;
	std::auto_ptr<Tool> mTool;
};

#endif