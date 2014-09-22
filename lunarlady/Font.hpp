#ifndef LL_FONT_HPP
#define LL_FONT_HPP

#include "lunarlady/TemplatedMedia.hpp"
#include "lunarlady/math/vec2.hpp"
#include "lunarlady/Color.hpp"
#include "lunarlady/Loaded.hpp"

#include <sstream>
#include <map>

namespace lunarlady {
	class World2;
	class ImageLoaded;

	class FontDescriptor {
	public:
		FontDescriptor(World2& iWorld, const std::string& iFileName);

		const std::string& getFileName() const;
		World2& getWorld() const;
		bool operator<(const FontDescriptor& iOther) const;
	private:
		World2& mWorld;
		const std::string mFileName;
	};

	class FontColor {
	public:
		FontColor(real r, real g, real b);
		explicit FontColor(const Color& iColor);

		void setColor(const Color& iColor);
		void setColor(int r, int g, int b);

		real getRed() const;
		real getGreen() const;
		real getBlue() const;
	private:
		real mRed;
		real mGreen;
		real mBlue;
	};


	class FontSize {
	public:
		explicit FontSize(real size);

		real getSize() const;
	private:
		const real mSize;
	};

	class FontLoaded : public Loaded {
	public:
		FontLoaded(const FontDescriptor& iDescriptor);
		~FontLoaded();

		static FontLoaded* Load(const FontDescriptor& iDescriptor);
		static void Unload(FontLoaded* iLoadedFont);

		void begin(real iScale, const math::vec2& iFontLocation);
		FontLoaded& operator<<(const FontColor& iNewColor);
		FontLoaded& operator<<(const FontSize& iNewSize);

		template<class T>
		FontLoaded& operator<<(const T& iT) {
			mStream << iT;
			return *this;
		}
		void end();

		real getHeight() const;
		real getBottom() const;

		void beginCalculation(real iScale);
		void endCalculation();
		real tellWidth() const;
	protected:
		void renderFont();
	private:
		std::wstringstream mStream;
		ImageLoaded* mImage;
		math::vec2 mFontLocation;
		FontColor mFontColor;
		real mLineHeight;
		real mBase;
		real mBottom;
		real mSize;
		real mScale;

		bool mCalculation;
		real mCalculationTextWidth;

		struct Character {
			int image;

			real uvLeft;
			real uvRight;
			real uvTop;
			real uvBottom;

			real width;
			real height;
			real x;
			real y;

			real advance;
		};

		void calculateCharacter(const Character& iCharacter);
		void processCharacter(const Character& iCharacter);
		void selectImage(const Character& iCharacter);

		typedef std::map<int, Character> CharacterMap;
		typedef std::pair<int, Character> CharacterPair;
		CharacterMap mCharacters;
	};

	typedef TemplatedMedia<FontLoaded, FontDescriptor> Font;

#define LL_PRINT(font, scale, location, message) do { font->begin(scale, location); font.get() << message; font->end(); } while(false)
}

#endif