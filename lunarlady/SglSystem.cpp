#include "sgl/sgl.hpp"

#include "lunarlady/System.hpp"
#include "lunarlady/Game.hpp"

#include "lunarlady/Script.hpp"
#include "lunarlady/Xml.hpp"
#include "lunarlady/File.hpp"
#include "lunarlady/Media.hpp"
#include "lunarlady/OpenGL_util.hpp"
#include "sgl/sgl_Assert.hpp"

namespace lunarlady {

	class SglSystem;

	namespace {
		SglSystem* gSystem = 0;
	}

	class SglSystem : public System {
	public:
		SglSystem() : System("SGL"), mShiftDown(false) {
			Assert(!gSystem, "System already initialized");
			gSystem = this;
			sgl::SetCaption( Registrator().getGameName().c_str() );
			setupVideoMode();
		}
		~SglSystem() {
			Assert(gSystem, "System not initialized, bug?");
			gSystem = 0;
		}

		void step(real iTime) {
			real x = 0;
			real y = 0;

			sgl::ProcessAxis();
			sgl::Event event;
			while( sgl::PollEvent(&event) ) {
				switch( event.type ) {
					case sgl::EVENT_EXIT:
						ExitGame();
						break;
					case sgl::EVENT_KEYDOWN:
						if( event.k == sgl::Key::Escape ) {
							if( mShiftDown ) {
								ExitGame();
							}
							else {
								OnKey(event.k, true);
							}
						} else
						if( event.k == sgl::Key::F1 ) {
							ToggleState("console");
						}
						else if( event.k == sgl::Key::F2 ) {
							ToggleState("messages");
						}
						else if( event.k == sgl::Key::F3 ) {
							ToggleState("display");
						}
						else if( event.k == sgl::Key::F4 ) {
							ToggleState("tweakers");
						}
						else {
							if( event.k == sgl::Key::Shift ) {
								mShiftDown = true;
							}
							OnKey(event.k, true);
						}
						break;
					case sgl::EVENT_KEYUP:
						if( event.k == sgl::Key::Shift ) {
							mShiftDown = false;
						}
						OnKey(event.k, false);
						break;
					case sgl::EVENT_MOUSE_AXIS_X:
						x = event.delta;
						break;
					case sgl::EVENT_MOUSE_AXIS_Y:
						y = event.delta;
						break;
					case sgl::EVENT_CHAR:
						OnChar(event.c);
						break;
				}
			}

			Resolution resolution = GetResolution();
			OnMouseMovement(math::vec2(x/resolution.width,y/resolution.height) );
		}

		void setupVideoMode() {
			Resolution resolution = GetResolution();
			sgl::VideoMode mode;
			if( IsFullscreen() ) {
				mode = mode.setFullscreen();
			}
			else {
				mode = mode.setWindowed();
			}
			const int depth = GetDepth();
			sgl::SetVideoMode( mode.setResolution(resolution.width, resolution.height).setColorBits(depth) );
		}

		bool mShiftDown;
	};
	LL_SYSTEM(SglSystem, 200);

	void ReloadVideoScriptFunction(FunctionArgs& iArgs) {
		Media::UnloadAll();
		ClearCache();
		sgl::KillVideoMode();
		gSystem->setupVideoMode();
		SetupGL();
		EnableState("loader");
		DisableRender();
		DisableState("console");
	}
	SCRIPT_FUNCTION(reloadVideo, ReloadVideoScriptFunction, "Reboots the video, unloads, changes videomode and then reload everything");

	void DumpKeysScriptFunction(FunctionArgs& iArgs) {
		if( ArgCount(iArgs) != 1 ) {
			ArgReportError(iArgs, "Function needs 1 string file to dump the keyvalues to");
		}
		ArgVarString(fileName, iArgs, 0);

		TiXmlDocument doc(fileName);
		TiXmlElement* root = (TiXmlElement*) doc.InsertEndChild( TiXmlElement("keys") );
		
		for(sgl::Key::LookupMap::const_iterator key = sgl::Key::sStringToDataMap.begin();
			key != sgl::Key::sStringToDataMap.end();
			++key) {
				TiXmlElement* child = (TiXmlElement*) root->InsertEndChild( TiXmlElement("key") );
				child->SetAttribute("id", key->first);
		}
		
		TiXmlPrinter printer;
		doc.Accept(&printer);
		const std::string& file = printer.Str();
		WriteFile(fileName, file.c_str(), file.length());
	}
	SCRIPT_FUNCTION(dumpKeys, DumpKeysScriptFunction, "Dumps all the possible key values");
}