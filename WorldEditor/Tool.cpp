#include "WorldEditor/Tool.hpp"
#include "WorldEditor/WorldView.hpp"

Tool::Tool(WorldView* iWorldView, ToolAction iToolAction) : mWorldView(iWorldView), mToolAction(iToolAction) {
}
WorldView& Tool::getWorldView() {
	return *mWorldView;
}
World& Tool::getWorld() {
	return mWorldView->getWorld();
}

ToolAction Tool::getToolAction() const {
	return mToolAction;
}