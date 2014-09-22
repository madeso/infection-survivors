#include "sgl/sgl_opengl.hpp"

#include "lunarlady/System.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Log.hpp"

namespace lunarlady {

	void SetupGL() {
		GLenum err = glewInit();
		if (GLEW_OK != err) {
			std::ostringstream str;
			str << "GLEW Error: " << glewGetErrorString(err);
			LOG1(str.str());
			throw str.str();
		}
		fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_TEXTURE_2D);
		//DataPointer<Resolution> resolution("resolution");
		//glViewport(0, 0, resolution.get().width, resolution.get().height);
		
		LOG1( "Open GL Version: " << glGetString(GL_VERSION) );
		LOG1( "Open GL Vendor: " << glGetString(GL_VENDOR) );
		LOG1( "Open GL Renderer: " << glGetString(GL_RENDERER) );
		/*{
			int mtxu;
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &mtxu);
			LOG( "Max multi-texturess: " <<  mtxu );
		}*/
	}

	class OpenglSystem : public System {
	public:
		OpenglSystem() : System("OpenGL") {
			SetupGL();
		}
		~OpenglSystem() {
		}

		void step(real iTime) {
		}
	};
	LL_SYSTEM(OpenglSystem, 300);
}