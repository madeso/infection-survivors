#ifndef LLMS_WORLD_HPP
#define LLMS_WORLD_HPP

#include <vector>

#include "boost/smart_ptr.hpp"

#include "lunarlady/math/vec2.hpp"

class World {
public:
	World();

	void setAspect(::lunarlady::real iAspect);

	void render() const;
	void renderBorder(int x, int y, int w, int h) const;

	void setCursorLocation( const int iWidth, const int iHeight, const ::lunarlady::math::vec2& iMovement );

	void setColor( ::lunarlady::real iRed, ::lunarlady::real iGreen, ::lunarlady::real iBlue, ::lunarlady::real iAlpha);

	void exportToFile(const std::string& iFileName, const std::string& iTextureName, unsigned int iHeight, unsigned int iWidth);

	void saveImage(const std::string& iFileName, unsigned int iWidth, unsigned int iHeight) const;

	void onLeft();
	void onRight();
private:
	::lunarlady::real mLeft;
	::lunarlady::real mRight;
	::lunarlady::real mUp;
	::lunarlady::real mDown;
	::lunarlady::real* mSelectedLine;
	bool mVerticalLine;

	::lunarlady::math::vec2 mNullLocation;
	
	::lunarlady::math::vec2 mCursorLocation;
	::lunarlady::real mRed;
	::lunarlady::real mGreen;
	::lunarlady::real mBlue;
	::lunarlady::real mAlpha;

	::lunarlady::real mAspect;

	void sendColorToOpenGL() const;

	void renderVerticalLine(::lunarlady::real x) const;
	void renderHorizontalLine(::lunarlady::real y) const;

	enum {
		Up,
		Left,
		Down,
		Right,
		Finished
	} mWorldBuildingState;
};

#endif