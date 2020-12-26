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

#endif
