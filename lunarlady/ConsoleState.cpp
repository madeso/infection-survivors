#include "lunarlady/Game.hpp"
#include "lunarlady/State.hpp"
#include "lunarlady/World2.hpp"
#include "lunarlady/Font.hpp"
#include "lunarlady/Object2.hpp"
#include "lunarlady/Printer.hpp"
#include "lunarlady/Message.hpp"
#include "sgl/sgl.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/Xml.hpp"
#include "lunarlady/StringUtils.hpp"
#include "lunarlady/StringMap.hpp"
#include "lunarlady/Script.hpp"
#include "lunarlady/Log.hpp"
#include <vector>
#include <map>

namespace lunarlady {
	typedef std::vector<std::wstring> HistoryList;

	namespace {
		const std::size_t MAX_HISTORY = 5;
	}

	class ConsoleGraphics : public Object2 {
	public:
		ConsoleGraphics(Font& iFont) : mFont(iFont), mCursorTime(0), mCursorVisible(true), mIsControlDown(false), mHistoryIndex(0), mHasHistoryIndex(false), mShowComplete(false) {
		}
		~ConsoleGraphics() {
		}

		void doRender(real iTime) {
			const real alpha = 0.95;
			const Rgba inputColor(63/255.0,63/255.0,51/255.0,alpha);
			const Rgba bkgColor(102/255.0,103/255.0,89/255.0, alpha);

			const real spacing = 0.01;
			const real height = Printer::GetHeight(mFont) + spacing;
			const real inputY = 1 - height;

			quad(inputColor, spacing, Registrator().getAspect()-spacing, inputY + Printer::GetHeight(mFont), inputY- Printer::GetBottom(mFont));

			std::wstring left = mInputLeft;
			std::wstring right = mInputRight;

			if( mHasHistoryIndex ) {
				left = getHistory();
				std::wstringstream stream;
				const std::size_t size = mHistory.size();
				stream << L"  <[" << size-mHistoryIndex << L"/" << size << L"]>";
				right = stream.str();
			}

			real width = 0;
			real start=0;

			Printer(mFont, "console.start", math::vec2(spacing+spacing, inputY), JUSTIFY_LEFT, &start).arg("input", left);
			Printer(mFont, "console.middle", math::vec2(start+spacing+spacing, inputY), JUSTIFY_LEFT, &width).arg("input", left);
			const real step = width + start;
			Printer(mFont, "console.end", math::vec2(spacing+spacing+step, inputY), JUSTIFY_LEFT, 0).arg("input", right);
			if( mCursorVisible ) {
				Printer(mFont, "console.cursor", math::vec2(spacing+spacing+step, inputY), JUSTIFY_LEFT, 0).arg("cursor", "_");
			}

			real y = inputY - height - spacing;
			bool found = false;

			{
				const std::wstring wtxt = Trim(getCurrentInput());
				const std::string txt(wtxt.begin(), wtxt.end());

				if( txt.empty() && !mShowComplete ) {
				}
				else {
					
					const std::wstring documentationText = GetDocumentation(txt);
					if( !documentationText.empty() ) {
						quad(bkgColor, start + 2*spacing, Registrator().getAspect()-spacing, y + Printer::GetHeight(mFont), y- 0.01);
						Printer(mFont, "console.help", math::vec2(start+3*spacing, y), JUSTIFY_LEFT, 0).arg("help", documentationText.c_str() );
						y -= height+spacing;
						found = true;
					}

					if( !found ) {
						std::vector<std::string> commands;
						FindCommands(txt, &commands);

						if( !commands.empty() ) {
							const std::size_t count = commands.size();
							const std::size_t max = 5;
							const std::size_t lim = std::min(count, max);
							if( count > max ) {
								quad(bkgColor, start + 2*spacing, Registrator().getAspect()-spacing, y + Printer::GetHeight(mFont), y- 0.01);
								Printer(mFont, "console.count", math::vec2(start+3*spacing, y), JUSTIFY_LEFT, 0).arg("count", count ).arg("lim", lim);
								y -= height+spacing;
							}
							{
								const real boxheight = (lim-1)*(height+spacing);
								quad(bkgColor, start + 2*spacing, Registrator().getAspect()-spacing, y + Printer::GetHeight(mFont), y- boxheight - Printer::GetBottom(mFont));
								for(std::size_t i=0; i<lim; ++i) {
									Printer(mFont, "console.cmd", math::vec2(start+3*spacing, y), JUSTIFY_LEFT, 0).arg("cmd", commands[i].c_str());
									y-= height+spacing;
								}
							}
						}
					}
				}
			}
		}
		void doUpdate(real iTime) {
			const real TIME = 0.4;
			mCursorTime += iTime;
			while( mCursorTime > TIME ) {
				mCursorTime -= TIME;
				mCursorVisible = !mCursorVisible;
			}
		}

		void execute(const std::wstring& iCommand) {
			//ConsoleMessage(CMT_ECHO, std::wstring(L"Recieved: ") + iCommand);
			std::string cmd(iCommand.begin(), iCommand.end());
			try {
				Execute(cmd, "", 0);
			}
			catch(const std::runtime_error& error) {
				LOG1( "console catched exception:" );
				LOG1( error.what() );
			}
			mShowComplete = false;
		}
		void command(const std::wstring& iCommand) {
			std::wstring command = iCommand;
			addToHistory(command);
			ConsoleMessage(CMT_ECHO, iCommand);
			execute(command);
		}

		void addToHistory(const std::wstring& iCommand) {
			mHistory.push_back(iCommand);
			if( mHistory.size() > MAX_HISTORY ) {
				mHistory.erase(mHistory.begin());
			}
		}

		std::wstring getHistory() const {
			if( mHasHistoryIndex ) {
				return mHistory[mHistoryIndex];
			}
			else {
				return L"";
			}
		}

		void historyPrevious() {
			if( mHistory.empty() ) return;
			if( !mHasHistoryIndex ) {
				mHistoryIndex = mHistory.size()-1;
				mHasHistoryIndex = true;
			}
			else {
				if( mHistoryIndex == 0 ) {
					mHasHistoryIndex = false;
				}
				else {
					--mHistoryIndex;
				}
			}
		}
		void historyNext() {
			if( mHistory.empty() ) return;
			if( !mHasHistoryIndex  ) {
				mHasHistoryIndex = true;
				mHistoryIndex = 0;
			}
			else {
				if( mHistoryIndex == mHistory.size()-1 ) {
					mHasHistoryIndex = false;
				}
				else {
					++mHistoryIndex;
				}
			}
		}

		void makeHistoryCurrentInput() {
			if( mHasHistoryIndex ) {
				mInputLeft = getHistory();
				mInputRight = L"";
				mHasHistoryIndex = false;
			}
		}

		std::wstring getCurrentInput() const {
			if( mHasHistoryIndex ) {
				return getHistory();
			}
			else return mInputLeft + mInputRight;
		}


		void onChar(const wchar_t iChar) {
			if( !mIsControlDown ) {
				mCursorVisible = false;
				mCursorTime = 0;

				makeHistoryCurrentInput();
				
				if( iChar == 8 ) {// backspace
					std::size_t length = mInputLeft.length();
					mInputLeft = mInputLeft.substr(0, length-1);
				}
				else if( iChar == 13 ) {
					const std::wstring input = mInputLeft + mInputRight;
					command(input);
					mInputLeft = L"";
					mInputRight = L"";
				}
				else {
					mInputLeft += iChar;
				}
			}
			else {
				if( iChar == ' ') {
					mShowComplete = true;
				}
			}
		}

		void onKey(const sgl::Key& iKey, bool iDown) {
			if( iKey == sgl::Key::Control ) {
				mIsControlDown = iDown;
			}
			if( iDown ) {
				if( mIsControlDown ) {
					if( iKey == sgl::Key::V ) {
						paste();
					}
					else if( iKey == sgl::Key::C ) {
						copy();
					}
				}
				else {
					if( iKey == sgl::Key::Arrow_Left ) {
						makeHistoryCurrentInput();
						left();
					}
					else if( iKey == sgl::Key::Arrow_Right ) {
						makeHistoryCurrentInput();
						right();
					}
					else if( iKey == sgl::Key::Home ) {
						makeHistoryCurrentInput();
						home();
					}
					else if( iKey == sgl::Key::End ) {
						makeHistoryCurrentInput();
						end();
					}
					else if( iKey == sgl::Key::Delete ) {
						makeHistoryCurrentInput();
						del();
					}
					else if( iKey == sgl::Key::Arrow_Up ) {
						historyPrevious();
					}
					else if( iKey == sgl::Key::Arrow_Down ) {
						historyNext();
					}
				}
			}
		}

		void copy() {
			std::wstring input = mInputLeft + mInputRight;
			std::string str(input.begin(), input.end());
			sgl::SetClipboardText( str );
		}
		void paste() {
			std::string input = sgl::GetClipboardText();;
			std::wstring winput(input.begin(), input.end());
			mInputLeft = winput;
			mInputRight = L"";
		}
		void end() {
			mInputLeft += mInputRight;
			mInputRight = L"";
		}
		void home() {
			mInputRight = mInputLeft + mInputRight;
			mInputLeft = L"";
		}

		void del() {
			const std::size_t rightLength = mInputRight.length();
			if( rightLength > 0 ) {
				mInputRight = mInputRight.substr(1, rightLength-1);
			}
		}

		void right() {
			const std::size_t leftLength = mInputLeft.length();
			const std::size_t rightLength = mInputRight.length();
			if( rightLength > 0 ) {
				mInputLeft += mInputRight[0];
				mInputRight = mInputRight.substr(1, rightLength-1);
			}
		}
		void left() {
			const std::size_t leftLength = mInputLeft.length();
			const std::size_t rightLength = mInputRight.length();
			if( leftLength > 0 ) {
				mInputRight = mInputLeft[leftLength-1] + mInputRight;
				mInputLeft = mInputLeft.substr(0, leftLength-1);
			}
		}

	private:
		Font& mFont;

		real mCursorTime;
		bool mCursorVisible;

		std::wstring mInputLeft;
		std::wstring mInputRight;

		bool mIsControlDown;
		HistoryList mHistory;
		std::size_t mHistoryIndex;
		bool mHasHistoryIndex;

		bool mShowComplete;
	};

	class ConsoleState : public State {
	public:
		ConsoleState() : State("console", 800, false, true, false), mWorld(), mFont(FontDescriptor(mWorld, "fonts/big.fnt")), mConsoleGraphics(0) {
			mFont.load();
			mConsoleGraphics = new ConsoleGraphics(mFont);
			mWorld.add( mConsoleGraphics );
		}

		void doFrame(real iTime) {
			mWorld.update(iTime);
		}
		void doTick(real iTime) {
		}
		void doRender(real iTime) {
			mWorld.render(iTime);
		}

		void onChar(const wchar_t iChar) {
			mConsoleGraphics->onChar(iChar);
		}

		void onMouseMovement(const math::vec2& iMovement) {
		}
		void onKey(const sgl::Key& iKey, bool iDown) {
			mConsoleGraphics->onKey(iKey, iDown);
		}

		World2 mWorld;
		Font mFont;
		ConsoleGraphics* mConsoleGraphics;
	};

	LL_STATE(ConsoleState)
}