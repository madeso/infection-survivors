#include "Message.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/State.hpp"
#include "lunarlady/World2.hpp"
#include "lunarlady/Font.hpp"
#include "lunarlady/Object2.hpp"
#include "lunarlady/Printer.hpp"
#include "lunarlady/Script.hpp"

namespace lunarlady {

	namespace {
		const std::size_t MAX_MESSAGES = 200;
	}

	typedef std::pair<ConsoleMessageType, std::wstring> Message;
	typedef std::vector<Message> MessageList;

	class MessageGraphics : public Object2 {
	public:
		MessageGraphics(Font& iFont) : mFont(iFont), mIndex(0) {
		}
		~MessageGraphics() {
		}

		void renderProgress(std::size_t iIndex, std::size_t iSize, real iX, real iY, real iHeight, real iWidth, real iThumbSize) {
			const real alpha = 0.5;
			const real gray = 0.4;
			const real lightGray = 0.9;
			
			// draw base
			
			const Rgba base(gray, gray, gray, alpha);
			quad(base, iX, iX+iWidth, iY, iY-iHeight-iThumbSize);

			// draw top
			if( iSize != 0 ) {
				const real size = iSize;
				const real locPos = iIndex / size;
				const real locHeight = 1.0 / size;
			
				const Rgba top(lightGray, lightGray, lightGray, alpha);
				const real y = iY - locPos*iHeight;
				const real h = math::Max(locHeight * (iHeight+iThumbSize), iThumbSize);

				quad(top, iX, iX+iWidth, y, y-h);
			}
		}

		void doRender(real iTime) {
			mFont.load(); // since this ca be on the top of the loader, this has to be here to ensure we aint use something bad
			const real spacing = 0.03;
			const std::size_t size = sMessages.size();
			renderProgress(mIndex, size, 0.01, 1 - spacing, 1 - 3*spacing, spacing, spacing);
			if( !sMessages.empty() ) {
				const real spacing = 0.01;
				const real step = Printer::GetHeight(mFont) + spacing;

				std::size_t end = size;
				real x = 0.05;
				real y = 0.88;
				std::size_t i = end -1 - mIndex;
				while( y > 0) {
					Printer(mFont, "message.echo", math::vec2(x, y), JUSTIFY_LEFT, 0).arg("message", sMessages[i].second);
					if( i == 0 ) break;
					if( i == size-1 ) {
						y -= 0.015;
					}
					y -= step;
					--i;
				}
			}
		}
		void doUpdate(real iTime) {
		}
		static void Insert(ConsoleMessageType iType, std::wstring iMessage) {
			sMessages.push_back( Message(iType, iMessage) );
		}

		void up() {
			const std::size_t size = sMessages.size();
			if( size > 1 ) {
				if( mIndex == 0 ) mIndex = size-1;
				else --mIndex;
			}
		}
		void down() {
			const std::size_t size = sMessages.size();
			if( size > 1 ) {
				mIndex++;
				if( mIndex == size ) mIndex = 0;
			}
		}

		static void ClearMessages() {
			sMessages.clear();
		}

	private:
		Font& mFont;
		std::size_t mIndex;

		static MessageList sMessages;
	};
	MessageList MessageGraphics::sMessages;

	void ConsoleMessage(ConsoleMessageType iType, const std::wstring& iMessage) {
		MessageGraphics::Insert(iType, iMessage);
	}

	void ConsoleClearMessages(FunctionArgs& iArgs) {
		MessageGraphics::ClearMessages();
	}

	SCRIPT_FUNCTION(clearMessages, ConsoleClearMessages, "Clears the messages from the message list");

	class MessageState : public State {
	public:
		MessageState() : State("messages", 700, true, true,
#ifdef NDEBUG
			false
#else
			true
#endif
			), mWorld(), mFont(FontDescriptor(mWorld, "fonts/big.fnt")) {
			mFont.load();
			mWorld.add( new MessageGraphics(mFont) );
		}

		void doFrame(real iTime) {
			mWorld.update(iTime);
		}
		void doTick(real iTime) {
		}
		void doRender(real iTime) {
			mWorld.render(iTime);
		}

		void onMouseMovement(const math::vec2& iMovement) {
			sendMouseMovement(iMovement);
		}
		void onKey(const sgl::Key& iKey, bool iDown) {
			if( iKey == sgl::Key::Page_Down ) {
				if( iDown ) {
					//MessageGraphics::GetInstance().down();
				}
			}
			else if( iKey == sgl::Key::Page_Up ) {
				if( iDown ) {
					//MessageGraphics::GetInstance().up();
				}
			}
			else {
				sendKey(iKey, iDown);
			}
		}

		World2 mWorld;
		Font mFont;
	};

	LL_STATE(MessageState)
}