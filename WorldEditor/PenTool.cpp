#include "WorldEditor/PenTool.hpp"
#include "WorldEditor/World.hpp"

PenTool::PenTool(WorldView* iWorldView) : Tool(iWorldView, TA_ALL), from(0), mOldLeftMouseState(false) {
}
PenTool::~PenTool() {
}
bool PenTool::onInput(const ::lunarlady::math::vec2& movement, wxMouseEvent& event) {
	getWorld().setLocking( event.ControlDown() );

	/*if( event.ShiftDown() ) {
		if( event.LeftDown() ) {
			TemporaryPoint* to = getWorld().placePoint();
			if( from ) {
				getWorld().addLine(from, to);
				if( event.ShiftDown() ) {
					from = to;
				}
				else {
					from = 0;
				}
			}
			else {
				from = to;
			}
		}
	}
	else {
		if( event.LeftIsDown() ) {
			if( !mOldLeftMouseState ) {
				from = getWorld().placePoint();
			}
		}
		else {
			if( from ) {
				if( mOldLeftMouseState ) {
					TemporaryPoint* to = getWorld().placePoint();
					getWorld().addLine(from, to);
				}
				from = 0;
			}
		}
	}*/

	if( event.LeftIsDown() ) {
		if( !mOldLeftMouseState ) {
			from = getWorld().placePoint();
		}
	}
	else {
		if( from ) {
			if( mOldLeftMouseState ) {
				TemporaryPoint* to = getWorld().placePoint();
				if( to ) {
					getWorld().addLine(from, to);
				}
			}
			from = 0;
		}
	}

	getWorld().setFrom(from);

	mOldLeftMouseState = event.LeftIsDown();
	return true;
}