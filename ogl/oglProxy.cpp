#include "oglProxy.hpp"

#ifndef GL_USE_PROGRAM_PROXY_OPTIMISATION_DISABLED

void glUseProgram_proxy(GLuint id)
{
	static GLuint currentProgramId = 0;
	if (id != currentProgramId)
	{
		glUseProgram(id);
		currentProgramId = id;
	}
}

void glBindVertexArray_proxy(GLuint vao)
{
	static GLuint currentVAO = 0;
	if (vao != currentVAO)
	{
		glBindVertexArray(vao);
		currentVAO = vao;
	}
}

#endif
