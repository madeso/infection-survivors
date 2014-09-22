#ifndef LLMS_TOOL_HPP
#define LLMS_TOOL_HPP

#include "wx.hpp"

#include "lunarlady/math/vec2.hpp"

class WorldView;
class World;

class Tool {
public:
	Tool(WorldView* iWorldView);
	virtual ~Tool() {}
	virtual bool onInput(const ::lunarlady::math::vec2& movement, wxMouseEvent& event) = 0;
protected:
	WorldView& getWorldView();
	World& getWorld();
private:
	WorldView* mWorldView;
};

#endif