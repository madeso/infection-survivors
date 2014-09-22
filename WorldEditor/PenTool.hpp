#ifndef LLW_PENTOOL_HPP
#define LLW_PENTOOL_HPP

#include "WorldEditor/Tool.hpp"

struct TemporaryPoint;

class PenTool : public Tool {
public:
	PenTool(WorldView* iWorldView);
	~PenTool();
	bool onInput(const ::lunarlady::math::vec2& movement, wxMouseEvent& event);
private:
	TemporaryPoint* from;
	bool mOldLeftMouseState;
};

#endif