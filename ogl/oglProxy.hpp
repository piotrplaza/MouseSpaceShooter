#pragma once

#include <GL/glew.h>

#ifdef GL_USE_PROGRAM_PROXY_OPTIMISATION_DISABLED

inline void glUseProgram_proxy(GLuint id)
{
	glUseProgram(id);
}

inline void glBindVertexArray_proxy(GLuint vao)
{
	glBindVertexArray(vao);
}

#else

void glUseProgram_proxy(GLuint id);
void glBindVertexArray_proxy(GLuint vao);

#endif
