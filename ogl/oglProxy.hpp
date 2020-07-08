#include <GL/glew.h>

#ifdef GL_USE_PROGRAM_PROXY_OPTIMISATION_DISABLED

inline void glUseProgram_proxy(GLuint id)
{
	glUseProgram(id);
}

#else

void glUseProgram_proxy(GLuint id);

#endif
