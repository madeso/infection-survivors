#include "make_shape/Tool.hpp"
#include "make_shape/WorldView.hpp"

Tool::Tool(WorldView* iWorldView) : mWorldView(iWorldView) {
}
WorldView& Tool::getWorldView() {
	return *mWorldView;
}
World& Tool::getWorld() {
	return mWorldView->getWorld();
}