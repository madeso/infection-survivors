#include <list>
#include <fstream>

#include "wx.hpp"
#include "wx_opengl.hpp"
#include <cmath>
#include "make_shape/World.hpp"

#include "IL/il.h"

void WriteToFile(std::ofstream& file, ::lunarlady::real x, ::lunarlady::real y, ::lunarlady::real u, ::lunarlady::real v) {
	file.write((char*) &x, sizeof(::lunarlady::real));
	file.write((char*) &y, sizeof(::lunarlady::real));

	file.write((char*) &u, sizeof(::lunarlady::real));
	file.write((char*) &v, sizeof(::lunarlady::real));
}

void World::exportToFile(const std::string& iFileName, const std::string& iTextureName, unsigned int iHeight, unsigned int iWidth) {
	std::ofstream file(iFileName.c_str(), std::ios::out | std::ios::binary);
	if( !file ) return;
	file.write((char*) &mRed, sizeof(::lunarlady::real));
	file.write((char*) &mGreen, sizeof(::lunarlady::real));
	file.write((char*) &mBlue, sizeof(::lunarlady::real));
	file.write((char*) &mAlpha, sizeof(::lunarlady::real));

 	const unsigned int textureSize = iTextureName.length();
	file.write((char*) &textureSize, sizeof(unsigned int));
	file.write( iTextureName.c_str(), sizeof(char) * iTextureName.length() );

	const unsigned int triangleCount = 2;
	file.write((char*) &triangleCount, sizeof(unsigned int));

	const ::lunarlady::real top = ::lunarlady::math::Max(mUp, mDown);
	const ::lunarlady::real bottom = ::lunarlady::math::Min(mUp, mDown);
	const ::lunarlady::real left = ::lunarlady::math::Min(mRight, mLeft);
	const ::lunarlady::real right =::lunarlady::math::Max(mRight, mLeft);
	const ::lunarlady::real size = ::lunarlady::math::Max(right-left, top-bottom);

	const ::lunarlady::real raspect = (::lunarlady::real) iWidth / (::lunarlady::real) iHeight;
	const ::lunarlady::real aspect = (right-left) / (top-bottom);

	::lunarlady::real width = iWidth;
	::lunarlady::real height = iHeight;

	if( ::lunarlady::math::equal(aspect, raspect) ) {
	}
	else {
		if( aspect < raspect ) {
			// current width is greater than supported
			// base new on current height
			width = iHeight * aspect;
		}
		else {
			// current height is greater than supported
			// base new on current width
			height = iWidth / aspect;
		}
	}

	const ::lunarlady::real uv_top = height/iHeight;
	const ::lunarlady::real uv_bottom = 0;
	const ::lunarlady::real uv_left = 0;
	const ::lunarlady::real uv_right = width/iWidth;

#define Write(y, x) WriteToFile(file, x-mNullLocation.getX(), y-mNullLocation.getY(), uv_##x, uv_##y)
	Write(top, right);
	Write(top, left);
	Write(bottom, left);
	Write(top, right);
	Write(bottom, left);
	Write(bottom, right);
#undef WRITE

	file.close();
}

void World::saveImage(const std::string& iFileName, unsigned int iWidth, unsigned int iHeight) const {
	ILuint ImageName;
    ilGenImages(1, &ImageName);
	ilBindImage(ImageName);
	{
		const ::lunarlady::real top = ::lunarlady::math::Max(mUp, mDown);
		const ::lunarlady::real bottom = ::lunarlady::math::Min(mUp, mDown);
		const ::lunarlady::real left = ::lunarlady::math::Min(mRight, mLeft);
		const ::lunarlady::real right =::lunarlady::math::Max(mRight, mLeft);
		const ::lunarlady::real size = ::lunarlady::math::Max(right-left, top-bottom);

		const ::lunarlady::real raspect = (::lunarlady::real) iWidth / (::lunarlady::real) iHeight;
		const ::lunarlady::real aspect = (right-left) / (top-bottom);

		::lunarlady::real width = iWidth;
		::lunarlady::real height = iHeight;

		if( ::lunarlady::math::equal(aspect, raspect) ) {
		}
		else {
			if( aspect < raspect ) {
				// current width is greater than supported
				// base new on current height
				width = iHeight * aspect;
			}
			else {
				// current height is greater than supported
				// base new on current width
				height = iWidth / aspect;
			}
		}

		const unsigned int xmax = width;
		const unsigned int ymax = height;

		boost::scoped_array<unsigned char> buffer( new unsigned char[iWidth*iHeight*3] );
		unsigned int index = 0;
		for(unsigned int y=0; y<iHeight; ++y) {
			for(unsigned int x=0; x<iWidth; ++x) {
				unsigned char c = 0;
				if( x <= xmax && y <= ymax ) {
					buffer[index + 0] = (unsigned char)(mRed * 255);
					buffer[index + 1] = (unsigned char)(mGreen * 255);
					buffer[index + 2] = (unsigned char)(mBlue * 255);
				}
				else {
					buffer[index + 0] = 0;
					buffer[index + 1] = 0;
					buffer[index + 2] = 0;
				}
				index += 3;
			}
		}
		ilTexImage(iWidth, iHeight, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, buffer.get());
	}
	ilEnable(IL_FILE_OVERWRITE);
	ilSaveImage(iFileName.c_str());
	ilDeleteImages(1, &ImageName);
}

World::World() : mAspect(14.0 / 9.0), mCursorLocation(0.5, 0.5), mRed(0.5), mGreen(0.5), mBlue(1.0), mAlpha(1),
	mLeft(0.0), mRight(0.0), mUp(0.0), mDown(0.0), mSelectedLine(0), mVerticalLine(false), mWorldBuildingState(Up) {
}

void World::onLeft() {
	switch( mWorldBuildingState ) {
		case Up:
			mWorldBuildingState = Down;
			mDown = mCursorLocation.getY();
			break;
		case Left:
			mWorldBuildingState = Right;
			mRight = mCursorLocation.getX();
			break;
		case Down:
			mWorldBuildingState = Left;
			mLeft = mCursorLocation.getX();
			break;
		case Right:
			mWorldBuildingState = Finished;
			break;
		case Finished:
			if( mSelectedLine ) {
				mSelectedLine = 0;
			}
			else {
#define TEST_LINE(GET, VAL, TYPE) if(mSelectedLine==0 && ::lunarlady::math::equal2(mCursorLocation.GET(), VAL) ) {mSelectedLine = &VAL; mVerticalLine = TYPE;}
				TEST_LINE(getX, mLeft, true);
				TEST_LINE(getX, mRight, true);
				TEST_LINE(getY, mUp, false);
				TEST_LINE(getY, mDown, false);
#undef TEST_LINE
			}
			break;
	}
}
void World::onRight() {
	if( mWorldBuildingState == Finished ) {
		mNullLocation = mCursorLocation;
	}
}

void World::setAspect(::lunarlady::real iAspect) {
	mAspect = iAspect;
}
void World::render() const {
	switch( mWorldBuildingState ) {
		case Up:
			glLineWidth(3.0f);
			glColor3d(0,0,0);
			glBegin(GL_LINES);
				renderHorizontalLine(mUp);
			glEnd();
			glLineWidth(1.0f);
			break;
		case Down:
			glColor3d(0.7, 0.7, 0.7);
			glBegin(GL_LINES);
				renderHorizontalLine(mUp);
			glEnd();
			glLineWidth(3.0f);
			glColor3d(0,0,0);
			glBegin(GL_LINES);
				renderHorizontalLine(mDown);
			glEnd();
			glLineWidth(1.0f);
			break;
		case Left:
			glColor3d(0.7, 0.7, 0.7);
			glBegin(GL_LINES);
				renderHorizontalLine(mUp);
				renderHorizontalLine(mDown);
			glEnd();
			glLineWidth(3.0f);
			glColor3d(0,0,0);
			glBegin(GL_LINES);
				renderVerticalLine(mLeft);
			glEnd();
			glLineWidth(1.0f);
			break;
		case Right:
			glColor3d(0.7, 0.7, 0.7);
			glBegin(GL_LINES);
				renderHorizontalLine(mUp);
				renderVerticalLine(mLeft);
				renderHorizontalLine(mDown);
			glEnd();
			glLineWidth(3.0f);
			glColor3d(0,0,0);
			glBegin(GL_LINES);
				renderVerticalLine(mRight);
			glEnd();
			glLineWidth(1.0f);
			break;
		case Finished:
			{
				const ::lunarlady::real top = ::lunarlady::math::Max(mUp, mDown);
				const ::lunarlady::real bottom = ::lunarlady::math::Min(mUp, mDown);
				const ::lunarlady::real left = ::lunarlady::math::Min(mRight, mLeft);
				const ::lunarlady::real right =::lunarlady::math::Max(mRight, mLeft);
				sendColorToOpenGL();
				glBegin(GL_QUADS);
					glVertex2d(right, top);
					glVertex2d(left, top);
					glVertex2d(left, bottom);
					glVertex2d(right, bottom);
				glEnd();
				glColor4d(0,0,0,1);
			}

			glColor3d(0.7, 0.7, 0.7);
			glBegin(GL_LINES);
				renderHorizontalLine(mUp);
				renderHorizontalLine(mDown);
				renderVerticalLine(mRight);
				renderVerticalLine(mLeft);
			glEnd();

			if( mSelectedLine ) {
				glLineWidth(3.0f);
				glColor3d(0,0,0);
				glBegin(GL_LINES);
				if( mVerticalLine )
					renderVerticalLine(*mSelectedLine);
				else
					renderHorizontalLine(*mSelectedLine);
				glEnd();
				glLineWidth(1.0f);
			}
			else {
				const ::lunarlady::real* selectedLine = 0;
				bool verticalLine = false;
		#define TEST_LINE(GET, VAL, TYPE) if(selectedLine==0 && ::lunarlady::math::equal2(mCursorLocation.GET(), VAL) ) {selectedLine = &VAL; verticalLine = TYPE;}
				TEST_LINE(getX, mLeft, true);
				TEST_LINE(getX, mRight, true);
				TEST_LINE(getY, mUp, false);
				TEST_LINE(getY, mDown, false);
		#undef TEST_LINE

				if( selectedLine ) {
					glColor3d(0,0,0);
					glBegin(GL_LINES);
					if( verticalLine )
						renderVerticalLine(*selectedLine);
					else
						renderHorizontalLine(*selectedLine);
					glEnd();
				}
			}
			break;
	}

	{
		glColor3d(0,0,0);
		const ::lunarlady::real disp = 0.025;
		glLineWidth(3);
		glBegin(GL_LINES);
			glVertex2d( mNullLocation.getX()-disp, mNullLocation.getY()-disp);
			glVertex2d( mNullLocation.getX()+disp, mNullLocation.getY()+disp);

			glVertex2d( mNullLocation.getX()-disp, mNullLocation.getY()+disp);
			glVertex2d( mNullLocation.getX()+disp, mNullLocation.getY()-disp);
		glEnd();
		glLineWidth(1);
	}
}

void World::setCursorLocation( const int iWidth, const int iHeight, const ::lunarlady::math::vec2& iMovement ) {
	::lunarlady::real x = iMovement.getX();
	::lunarlady::real y = iMovement.getY();

	const ::lunarlady::real aspect = (::lunarlady::real) iWidth / (::lunarlady::real) iHeight;
	const ::lunarlady::real raspect = mAspect;
	if( ::lunarlady::math::equal(aspect, mAspect) ) {
		y /= iHeight;
		x /= iWidth;
	}
	else {
		if( aspect > mAspect ) {
			// current width is greater than supported
			// base new on current height
			const ::lunarlady::real width = iHeight * mAspect;
			const ::lunarlady::real theRest = iWidth - width;
			const ::lunarlady::real borderSize = theRest / 2.0;

			x = (x - borderSize)/ width;
			y /= iHeight;
		}
		else {
			// current height is greater than supported
			// base new on current width
			const ::lunarlady::real height = iWidth / mAspect;
			const ::lunarlady::real theRest = iHeight - height;
			const ::lunarlady::real borderSize = theRest / 2.0;

			x /= iWidth;
			y = (y - borderSize)/ height;
		}
	}

	if( y > 1 ) y = 1;
	if( y < 0 ) y = 0;

	if( x > 1 ) x = 1;
	if( x < 0 ) x = 0;

	mCursorLocation = ::lunarlady::math::vec2(x*mAspect, y);

	switch( mWorldBuildingState ) {
		case Up:
			mUp = mCursorLocation.getY();
			break;
		case Left:
			mLeft = mCursorLocation.getX();
			break;
		case Down:
			mDown = mCursorLocation.getY();
			break;
		case Right:
			mRight = mCursorLocation.getX();
			break;
		case Finished:
			if( mSelectedLine ) {
				*mSelectedLine = (mVerticalLine) ? mCursorLocation.getX() : mCursorLocation.getY();
			}
			break;
	}
}

void World::setColor( ::lunarlady::real iRed, ::lunarlady::real iGreen, ::lunarlady::real iBlue, ::lunarlady::real iAlpha) {
	mRed = iRed;
	mGreen = iGreen;
	mBlue = iBlue;
	mAlpha = iAlpha;
}

void World::renderBorder(int x, int y, int iWidth, int iHeight) const {
	const ::lunarlady::real aspect = (::lunarlady::real) iWidth / (::lunarlady::real) iHeight;
	glViewport(x, y, iWidth, iHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	const ::lunarlady::real raspect = mAspect;
	if( ::lunarlady::math::equal(aspect, mAspect) ) {
		// equal, dont draw anything, draw in fullscreen
	}
	else {
		if( aspect > mAspect ) {
			// current width is greater than supported
			// base new on current height
			const ::lunarlady::real width = iHeight * mAspect;
			const ::lunarlady::real theRest = iWidth - width;
			const ::lunarlady::real borderSize = theRest / 2.0;

			glViewport(x+borderSize, y, width, iHeight);
		}
		else {
			// current height is greater than supported
			// base new on current width
			const ::lunarlady::real height = iWidth / mAspect;
			const ::lunarlady::real theRest = iHeight - height;
			const ::lunarlady::real borderSize = theRest / 2.0;
			glViewport(x, y+borderSize, iWidth, height);
		}
	}

	// background
	const ::lunarlady::real width = mAspect;
	const ::lunarlady::real height = 1;

	// setup 2d
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

	glColor3d(1, 1, 1);
	glBegin(GL_QUADS);
		glVertex2d(0, 0);
		glVertex2d(width, 0);
		glVertex2d(width, height);
		glVertex2d(0, height);
	glEnd();
}

void World::renderVerticalLine(::lunarlady::real x) const {
	glVertex2d(x, 0);
	glVertex2d(x, 1);
}
void World::renderHorizontalLine(::lunarlady::real y) const {
	glVertex2d(0, y);
	glVertex2d(mAspect, y);
}

void World::sendColorToOpenGL() const {
	glColor4d(mRed, mGreen, mBlue, mAlpha);
}