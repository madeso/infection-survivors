#include "lunarlady/Font.hpp"

#include "lunarlady/Xml.hpp"
#include "sgl/sgl_OpenGl.hpp"

#include "lunarlady/File.hpp"
#include "lunarlady/Error.hpp"
#include "lunarlady/Image.hpp"

#include "lunarlady/Game.hpp"

#include "lunarlady/convertColorBetweenRed.hpp"
#include "lunarlady/convertColorBetweenGreen.hpp"
#include "lunarlady/convertColorBetweenBlue.hpp"

namespace lunarlady {
	FontDescriptor::FontDescriptor(World2& iWorld, const std::string& iFileName) : mWorld(iWorld), mFileName(iFileName) {
	}
	const std::string& FontDescriptor::getFileName() const {
		return mFileName;
	}
	World2& FontDescriptor::getWorld() const {
		return mWorld;
	}
	bool FontDescriptor::operator<(const FontDescriptor& iOther) const {
		return mFileName < iOther.mFileName;
	}

	// -------------------------------------------------------------------------------

	FontColor::FontColor(real r, real g, real b) : mRed(r), mGreen(g), mBlue(b) {
	}

	FontColor::FontColor(const Color& iColor) : mRed(0), mGreen(0), mBlue(0) {
		setColor(iColor);
	}

	void FontColor::setColor(const Color& iColor) {
		mRed = convert::ColorToRed(iColor) / 255.0;
		mGreen = convert::ColorToGreen(iColor) / 255.0;
		mBlue = convert::ColorToBlue(iColor) / 255.0;
	}
	void FontColor::setColor(int r, int g, int b) {
		mRed = r / 255.0;
		mGreen = g / 255.0;
		mBlue = b / 255.0;
	}

	real FontColor::getRed() const {
		return mRed;
	}
	real FontColor::getGreen() const {
		return mGreen;
	}
	real FontColor::getBlue() const {
		return mBlue;
	}

	// ---------------------------------------------------------------------------------

	FontSize::FontSize(real iSize) : mSize(iSize) {
	}

	real FontSize::getSize() const {
		return mSize;
	}

	// ------------------------------------------------------------------------------------

	FontLoaded::FontLoaded(const FontDescriptor& iDescriptor) : mFontColor(1,1,1), mFontLocation(0,0), mSize(1), mBottom(0) {
		ReadFile file(iDescriptor.getFileName());
		TiXmlDocument doc(iDescriptor.getFileName());
		doc.Parse( file.getBuffer() );

		if ( doc.Error() ) {
			throw XmlError( iDescriptor.getFileName() + std::string(", error in ") + std::string(doc.Value()) + std::string(": ") + std::string(doc.ErrorDesc()) );
		}
		TiXmlHandle docHandle( &doc );
		const int size = GetIntAttribute(docHandle.FirstChild("font").FirstChild("info").ToElement(), "font::info", "size", iDescriptor.getFileName());
		const int lineHeight = GetIntAttribute(docHandle.FirstChild("font").FirstChild("common").ToElement(), "font::common", "lineHeight", iDescriptor.getFileName());
		const int base = GetIntAttribute(docHandle.FirstChild("font").FirstChild("common").ToElement(), "font::common", "base", iDescriptor.getFileName());
		const real scaleW = GetIntAttribute(docHandle.FirstChild("font").FirstChild("common").ToElement(), "font::common", "scaleW", iDescriptor.getFileName());
		const real scaleH = GetIntAttribute(docHandle.FirstChild("font").FirstChild("common").ToElement(), "font::common", "scaleH", iDescriptor.getFileName());
		const std::string textureFile = GetStringAttribute(docHandle.FirstChild("font").FirstChild("pages").FirstChild("page").ToElement(), "font::pages::page", "file", iDescriptor.getFileName());
		mImage = ImageLoaded::Load(ImageDescriptor(textureFile) );

		mLineHeight = lineHeight / scaleH;

		mBase = base / scaleH;

		unsigned int charsAdded = 0;
		for(TiXmlElement* chars = docHandle.FirstChild("font").FirstChild("chars").FirstChild("char").ToElement();
			chars; chars = chars->NextSiblingElement("char") ) {
#define INT(id) GetIntAttribute(chars, "font::chars::char", id, iDescriptor.getFileName())
				const int id = INT("id");
				const int x = INT("x");
				const int y = INT("y");
				const int width = INT("width");
				const int height = INT("height");
				const int xoffset = INT("xoffset");
				const int yoffset = INT("yoffset");
				const int xadvance  = INT("xadvance");
				const int page = INT("page");
#undef INT
				Character c;

				c.image = page;

				c.uvLeft = x / scaleW;
				c.uvRight = c.uvLeft + width / scaleW;
				c.uvTop = y / scaleH;
				c.uvBottom = c.uvTop + height / scaleH;

				c.width = width / scaleW;
				c.height = height / scaleH;
				c.x = xoffset / scaleW;
				c.y = yoffset / scaleH;

				c.advance = xadvance / scaleW;

				mCharacters.insert( CharacterPair(id, c) );
				++charsAdded;

				const real top    = - c.y + mBase;
				const real bottom = top - c.height;
				mBottom = math::Max(mBottom, -bottom);
		}

		if( charsAdded==0 ) throw XmlError( iDescriptor.getFileName() + " has no characters");
	}
	FontLoaded::~FontLoaded() {
		ImageLoaded::Unload( mImage );
		mImage = 0;
	}
	real FontLoaded::getHeight() const {
		return mLineHeight;
	}
	real FontLoaded::getBottom() const {
		return mBottom;
	}
	FontLoaded* FontLoaded::Load(const FontDescriptor& iDescriptor) {
		//return new FontLoaded(iDescriptor);
		return ::lunarlady::Load(iDescriptor);
	}
	void FontLoaded::Unload(FontLoaded* iLoadedFont) {
		//delete iLoadedFont;
		::lunarlady::Unload(iLoadedFont);
	}
	real FontLoaded::tellWidth() const {
		return mCalculationTextWidth;
	}
	void FontLoaded::beginCalculation(real iScale) {
		mCalculation = true;
		mCalculationTextWidth = 0;

		mScale = iScale;
		mSize = mScale;
	}
	void FontLoaded::begin(real iScale, const math::vec2& iFontLocation) {
		mFontLocation = iFontLocation;
		mCalculation = false;
		mCalculationTextWidth = 0;

		glBindTexture(GL_TEXTURE_2D, mImage->getId());
		glBegin(GL_QUADS);

		mScale = iScale;
		mSize = mScale;
		mFontColor.setColor( Color(Color::White) );
	}
	FontLoaded& FontLoaded::operator<<(const FontSize& iNewSize) {
		renderFont();
		mSize = iNewSize.getSize() * mScale;
		return *this;
	}
	FontLoaded& FontLoaded::operator<<(const FontColor& iNewColor) {
		renderFont();
		mFontColor = iNewColor;
		return *this;
	}
	void FontLoaded::end() {
		renderFont();
		glEnd();
	}
	void FontLoaded::endCalculation() {
		renderFont();
	}
	void FontLoaded::renderFont() {
		const std::wstring toPrint = mStream.str();
		if( toPrint.empty() ) return;
		mStream.str( L"" );
		const std::size_t length = toPrint.length();

		if( mCalculation ) {
			for(std::size_t charIndex=0; charIndex<length; ++charIndex) {
				int c = toPrint[charIndex];
				unsigned int i = c;
				const CharacterMap::iterator res = mCharacters.find(c);
				if( res != mCharacters.end() ) {
					calculateCharacter(res->second);
				}
			}
		}
		else {
			glColor3d(mFontColor.getRed(), mFontColor.getGreen(), mFontColor.getBlue());
			for(std::size_t charIndex=0; charIndex<length; ++charIndex) {
				int c = toPrint[charIndex];
				unsigned int i = c;
				const CharacterMap::iterator res = mCharacters.find(c);
				if( res != mCharacters.end() ) {
					processCharacter(res->second);
				}
			}
		}
	}
	void FontLoaded::calculateCharacter(const Character& iCharacter) {
		mCalculationTextWidth+=iCharacter.advance*mSize;
	}
	void FontLoaded::processCharacter(const Character& iCharacter) {
		selectImage(iCharacter);

		const real left   = mFontLocation.getX() + iCharacter.x*mSize;
		const real top    = mFontLocation.getY() - iCharacter.y*mSize + mBase*mSize;
		const real right  = left + iCharacter.width*mSize;
		const real bottom = top - iCharacter.height*mSize;
		
		const math::vec2 lowerLeft(left, bottom);
		const math::vec2 lowerRight(right, bottom);

		const math::vec2 upperLeft(left, top);
		const math::vec2 upperRight(right, top);

		const math::vec2 uv_upperLeft(iCharacter.uvLeft, iCharacter.uvTop);
		const math::vec2 uv_upperRight(iCharacter.uvRight, iCharacter.uvTop);
		const math::vec2 uv_lowerRight(iCharacter.uvRight, iCharacter.uvBottom);
		const math::vec2 uv_lowerLeft(iCharacter.uvLeft, iCharacter.uvBottom);

		mFontLocation += math::vec2(iCharacter.advance*mSize, 0);

#define Vertex(v) do { glTexCoord2d(uv_##v.getX(), 1-uv_##v.getY()); glVertex2d(v.getX(), v.getY()); } while(false)
		
		Vertex(lowerLeft);
		Vertex(lowerRight);
		Vertex(upperRight);
		Vertex(upperLeft);
#undef Vertex
	}
	void FontLoaded::selectImage(const Character& iCharacter) {
	}
	
	/*
	void PrintWrappedString(char* pStr, int consoleWidth) {
		int lastSpacePos = 0;
		int numCharsOnLineSoFar = 0;
		int numCharsAfterSpace = 0;

		int strLength = static_cast<int>(strlen(pStr));
		for(int i = 0; i < strLength; ++i)
		{
			if(numCharsOnLineSoFar == consoleWidth - 1)
			{
				pStr[lastSpacePos] = '\n';
				numCharsOnLineSoFar = numCharsAfterSpace;
			}
			if(pStr[i] == ' ')
			{
				lastSpacePos = i;
				numCharsAfterSpace = 0;
			}
			++numCharsAfterSpace;
			++numCharsOnLineSoFar;
		}
		printf(pStr);
	}
	*/
}