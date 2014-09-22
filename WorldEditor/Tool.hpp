#ifndef LLW_TOOL_HPP
#define LLW_TOOL_HPP

#include "lunarlady/math/vec2.hpp"
#include "wx.hpp"
#include "WorldEditor/ToolAction.hpp"

class WorldView;
class World;

class Tool {
public:
	Tool(WorldView* iWorldView, ToolAction iToolAction);
	virtual ~Tool() {}
	virtual bool onInput(const ::lunarlady::math::vec2& movement, wxMouseEvent& event) = 0;
	ToolAction getToolAction() const;
protected:
	WorldView& getWorldView();
	World& getWorld();
private:
	WorldView* mWorldView;
	const ToolAction mToolAction;
};

#endif