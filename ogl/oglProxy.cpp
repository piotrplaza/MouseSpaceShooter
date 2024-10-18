#include "oglProxy.hpp"

#ifndef GL_USE_PROGRAM_PROXY_OPTIMISATION_DISABLED

namespace
{
	bool blend = false;
	bool depthTest = false;
	bool cullFace = false;
}

void glProxyUseProgram(GLuint id)
{
	static GLuint currentProgramId = 0;
	if (id != currentProgramId)
	{
		glUseProgram(id);
		currentProgramId = id;
	}
}

void glProxyBindVertexArray(GLuint vao)
{
	static GLuint currentVAO = 0;
	if (vao != currentVAO)
	{
		glBindVertexArray(vao);
		currentVAO = vao;
	}
}

void glProxySetBlend(bool enabled)
{
	if (enabled != blend)
	{
		if (enabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
		blend = enabled;
	}
}

void glProxySetDepthTest(bool enabled)
{
	if (enabled != depthTest)
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		depthTest = enabled;
	}
}

void glProxySetCullFace(bool enabled)
{
	if (enabled != cullFace)
	{
		if (enabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		cullFace = enabled;
	}
}

bool glProxyIsBlendEnabled()
{
	return blend;
}

bool glProxyIsDepthTestEnabled()
{
	return depthTest;
}

bool glProxyIsCullFaceEnabled()
{
	return cullFace;
}

#endif
