#ifndef LLW_ERASOR_TOOL_HPP
#define LLW_ERASOR_TOOL_HPP

#include "Tool.hpp"

class ErasorTool : public Tool {
public:
	ErasorTool(WorldView* iWorldView);
	~ErasorTool();
	bool onInput(const ::lunarlady::math::vec2& movement, wxMouseEvent& event);
private:
	bool mIsDown;
};

#endif