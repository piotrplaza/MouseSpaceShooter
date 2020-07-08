#include "oglProxy.hpp"

#ifndef GL_USE_PROGRAM_PROXY_OPTIMISATION_DISABLED

static GLuint currentProgramId;
void glUseProgram_proxy(GLuint id)
{
	if (id != currentProgramId)
	{
		glUseProgram(id);
		currentProgramId = id;
	}
}

#endif
