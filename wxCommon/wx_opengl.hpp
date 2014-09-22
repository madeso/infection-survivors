// verify opengl
#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

// include opengl
#include "wx/timer.h"
#include "wx/glcanvas.h"
#include "wx/math.h"
#if defined(__WXMAC__) || defined(__WXCOCOA__)
#   ifdef __DARWIN__
#       include <OpenGL/gl.h>
#       include <OpenGL/glu.h>
#   else
#       include <gl.h>
#       include <glu.h>
#   endif
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#endif

// disabled because this has apparently changed in OpenGL 1.2, so doesn't link correctly if this is on...
#ifdef GL_EXT_vertex_array
#undef GL_EXT_vertex_array
#endif