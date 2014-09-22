
#include <sstream>

#include "boost/lexical_cast.hpp"

#include "sgl/sgl_Assert.hpp"
#include "lunarlady/PrintJob.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/StringFormat.hpp"
#include "lunarlady/Language.hpp"
#include "lunarlady/Error.hpp"
#include <strstream>

namespace lunarlady {

	PrintArguments::PrintArguments(Font& iFont, const ArgumentMap& iArgumentMap) : font(iFont), argumentMap(iArgumentMap) {
	}

	// ----------------------------------------------------------------

	class PrintStaticText : public PrintComponent {
	public:
		PrintStaticText(const std::string iText) : mText( Trim(iText) ) {
		}
		void apply(PrintArguments& iArguments) {
			iArguments.font.get() << String(mText);
		}
	private:
		const std::string mText;
	};

	// ----------------------------------------------------------------

	namespace {
		int HexadecimalToInt(const std::string& iString) {
			::std::stringstream str(iString.c_str());
			unsigned int result;
			str >> ::std::hex >> result;
			if( str.fail() ) {
				throw FileDataError("Failed to read hexadecimal value");
			}
			return result;
		}
	}

	class PrintColor : public PrintComponent {
	public:
		PrintColor(const std::string& iColorName) : mFontColor(Color::Red)  {
			const std::string colorName = Trim(iColorName);
			const std::size_t length = colorName.length();
			if( length > 0 ) {
				if( length==7 && colorName[0] == '#' ) {
					const int red = HexadecimalToInt( colorName.substr(1, 2) );
					const int green = HexadecimalToInt( colorName.substr(3, 2) );
					const int blue = HexadecimalToInt( colorName.substr(5, 2) );
					mFontColor.setColor(red, green, blue);
				}
				else {
					mFontColor.setColor(Color::Color( colorName ) );
				}
			}
			else {
				throw FileDataError("Missing color value");
			}
		}
		void apply(PrintArguments& iArguments) {
			iArguments.font.get() << mFontColor;
		}
	private:
		FontColor mFontColor;
	};

	// ----------------------------------------------------------------

	class PrintSize : public PrintComponent {
	public:
		PrintSize(real iSize) : mFontSize(iSize) {
		}
		void apply(PrintArguments& iArguments) {
			iArguments.font.get() << mFontSize;
		}
	private:
		FontSize mFontSize;
	};

	// ----------------------------------------------------------------

	class PrintVariable : public PrintComponent {
	public:
		PrintVariable(const std::string& iName) : mName(Trim(iName) ) {
		}

		void apply(PrintArguments& iArguments) {
			ArgumentMap::const_iterator res = iArguments.argumentMap.find(mName);
			Assert(res != iArguments.argumentMap.end(), "Missing argument value"); // check mName for variable name
			if( res != iArguments.argumentMap.end() ) {
				iArguments.font.get() << res->second;
			}
		}
	private:
		const std::string mName;
	};

	// ----------------------------------------------------------------

	PrintJob::PrintJob(const std::string& iText) {
		const std::string result = iText;
		const std::size_t length = result.length();

		bool argument = false;
		bool parseControl = true;

		std::ostringstream buffer;
		const char ARGUMENT_CHAR = '@';

		for(std::size_t charIndex=0; charIndex<length; ++charIndex) {
			const char c = result[charIndex];
			if( parseControl ) {
				if( c == '{' ) {
					parseControl = false;
					std::vector<std::string> commands;
					SplitString(";", buffer.str(), &commands);
					for(std::size_t index=0; index<commands.size(); ++index) {
						std::vector<std::string> args;
						SplitString(":", Trim(commands[index]), &args);
							if( !args.empty() ) {
							const std::string name = Trim(args[0]);
							if( name == "color" ) {
								insert( new PrintColor( ToLower(args[1]) ) );
							}
							else if( name == "size" ) {
								insert( new PrintSize( boost::lexical_cast<real>( Trim(args[1]) ) ) );
							}
							else {
								Assert(false, "Unknown name");
							}
						}
					}
					buffer.str( "" );
				}
				else {
					buffer << c;
				}
			}
			else {
				if( c == '}' ) {
					parseControl = true;
					if( !buffer.str().empty() ) {
						if( argument ) {
							insert( new PrintVariable( buffer.str() ) );
						}
						else {
							insert( new PrintStaticText( buffer.str() ) );
						}
					}
					argument = false;
					buffer.str("");
				}
				else if( c == ARGUMENT_CHAR ) {
					argument = true;
					if( !buffer.str().empty() )
					insert( new PrintStaticText( buffer.str() ) );
					buffer.str( "" );
				}
				else if( c == ' ' ) {
					if( !buffer.str().empty() ) {
						if( argument ) {
							insert( new PrintVariable( buffer.str() ) );
						}
						else {
							insert( new PrintStaticText( buffer.str() ) );
						}
						argument = false;
						buffer.str( "" );
					}
				}
				else {
					buffer << c;
				}
			}
		}
	}
	void PrintJob::insert(PrintComponent* iComponent) {
		PrintComponentPtr ptr(iComponent);
		mPrintComponents.push_back( ptr );
	}

	struct ApplyComponent {
		ApplyComponent(PrintArguments& iArguments) : arguments(iArguments) {
		}
		void operator()(const PrintComponentPtr& iComponent) {
			iComponent->apply(arguments);
		}
		PrintArguments& arguments;
	};

	#define LOOP_FONT(arguments) std::for_each(mPrintComponents.begin(), mPrintComponents.end(), ApplyComponent(arguments) )

	real PrintJob::getWidth(Font& iFont, real iSize, const ArgumentMap& iArguments) const {
		PrintArguments arguments(iFont, iArguments);

		iFont->beginCalculation(iSize);
		LOOP_FONT(arguments);
		iFont->endCalculation();
		return iFont->tellWidth();
	}
	void PrintJob::print(Font& iFont, real iSize, const math::vec2 iLocation, Justification iJustification, real* oWidth, real* iCashedWidth, const ArgumentMap& iArguments) const {
		PrintArguments arguments(iFont, iArguments);

		math::vec2 location = iLocation;
		if( iJustification != JUSTIFY_LEFT || oWidth != 0) {

			const real width = (iCashedWidth) ? *iCashedWidth : getWidth(iFont, iSize, iArguments);

			if( oWidth != 0 ) {
				*oWidth = width;
			}

			if( iJustification != JUSTIFY_LEFT ) {
				real xstep = 0;
				real ystep = 0;
				if( iJustification == JUSTIFY_CENTER ) {
					xstep -= width/2;
				}
				else if( iJustification == JUSTIFY_RIGHT ) {
					xstep -= width;
				}

				location += math::vec2(xstep, ystep);
			}
		}

		iFont->begin(iSize, location);
		LOOP_FONT(arguments);
		iFont->end();
	}
}