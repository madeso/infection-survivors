#include "sgl/sgl_OpenGl.hpp"

#include "lunarlady/OpenGL_util.hpp"
#include "lunarlady/Types.hpp"
#include "lunarlady/math/Math.hpp"
#include "lunarlady/Game.hpp"
#include "lunarlady/Error.hpp"
#include "sgl/sgl_Assert.hpp"
#include <fstream>

#include "lunarlady/Object2.hpp"

namespace lunarlady {
	void Object2::quad(Rgba iColor, real iLeft, real iRight, real iTop, real iBottom) {
		glDisable(GL_TEXTURE_2D);
		glColor4d(iColor.getRed(), iColor.getGreen(), iColor.getBlue(), iColor.getAlpha());
		glBegin(GL_QUADS);
			glVertex2d(iRight, iTop);
			glVertex2d(iLeft, iTop);
			glVertex2d(iLeft, iBottom);
			glVertex2d(iRight, iBottom);
		glEnd();
		glColor4d(iColor.getRed(), iColor.getGreen(), iColor.getBlue(), 1);
		glEnable(GL_TEXTURE_2D);
	}

	void DrawBorders() {
		const Resolution resolution = GetResolution();
		const real aspect = (real) resolution.width / (real) resolution.height;
		glViewport(0, 0, resolution.width, resolution.height);
		glClear(GL_COLOR_BUFFER_BIT);
		const real raspect = Registrator().getAspect();
		if( ::lunarlady::math::equal(aspect, Registrator().getAspect()) ) {
			// equal, dont draw anything, draw in fullscreen
		}
		else {
			if( aspect > Registrator().getAspect() ) {
				// current width is greater than supported
				// base new on current height
				const real width = resolution.height * Registrator().getAspect();
				const real theRest = resolution.width - width;
				const real borderSize = theRest / 2.0;

				glViewport(static_cast<GLsizei>(borderSize), 0, static_cast<GLsizei>(width), resolution.height);
			}
			else {
				// current height is greater than supported
				// base new on current width
				const real height = resolution.width / Registrator().getAspect();
				const real theRest = resolution.height - height;
				const real borderSize = theRest / 2.0;
				glViewport(0, static_cast<GLsizei>(borderSize), resolution.width, static_cast<GLsizei>(height));
			}
		}
	}
	void SetDisplay2d() {
		// setup 2d
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, Registrator().getAspect(), 0, 1, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
	}

	void SetDisplay3d() {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		// http://www.sjbaker.org/steve/omniv/love_your_z_buffer.html
		gluPerspective(45.0f, Registrator().getAspect(), 0.5f, 1000.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glEnable(GL_DEPTH_TEST);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	
	void testGl() {
		GLenum error = glGetError();
		if( error != GL_NO_ERROR ) {
			std::fstream file("glerr.txt", std::ios::app);
			std::stringstream message;

			switch(error) {
	#define CASE(value, desc) case value: message << #value << ": " << desc; break;
				CASE(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument. The offending command	is ignored, and	has no other side effect than to set the error flag.")
				CASE(GL_INVALID_VALUE, "A numeric argument is out of range.	The offending command is ignored, and has no other side effect than to set the error flag.")
				CASE(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state. The offending command is ignored, and has no other side effect than to set the error flag.")
				CASE(GL_STACK_OVERFLOW, "This command would cause a stack overflow. The offending command	is ignored, and	has no other side effect than to set the error flag.")
				CASE(GL_STACK_UNDERFLOW, "This command would cause a stack underflow. The offending command is ignored, and has	no other side effect than to set the error flag.")
				CASE(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.")
	#undef CASE
				default: message << "Undefined error code: #" << error;
			}
			const std::string errormessage = message.str();
			file << errormessage << std::endl;
#ifdef _DEBUG
			_asm { int 3 }
#else
		throw OpenGLError(errormessage);
#endif
		}
	}
}