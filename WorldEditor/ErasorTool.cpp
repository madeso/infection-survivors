#include "WorldEditor/ErasorTool.hpp"
#include "WorldEditor/World.hpp"

ErasorTool::ErasorTool(WorldView* iWorldView) : Tool(iWorldView, TA_LINES), mIsDown(false) {
}
ErasorTool::~ErasorTool() {
}
bool ErasorTool::onInput(const ::lunarlady::math::vec2& movement, wxMouseEvent& event) {
	if( event.LeftIsDown() ) {
		Line* line = getWorld().gatherLine();
		if( line ) {
			getWorld().markForDeletion(line);
		}
		mIsDown = true;
	}
	else {
		if( mIsDown ) {
			getWorld().deleteLines();
			mIsDown = false;
		}
	}
	
	return true;
}