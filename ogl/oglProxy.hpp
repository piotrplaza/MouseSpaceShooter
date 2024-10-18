#pragma once

#include <GL/glew.h>

#ifdef GL_USE_PROGRAM_PROXY_OPTIMISATION_DISABLED

inline void glProxyUseProgram(GLuint id)
{
	glUseProgram(id);
}

inline void glProxyBindVertexArray(GLuint vao)
{
	glBindVertexArray(vao);
}

inline void glProxySetBlend(bool enabled)
{
	if (enabled)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

inline void glProxySetDepthTest(bool enabled)
{
	if (enabled)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

inline void glProxySetCullFace(bool enabled)
{
	if (enabled)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

inline bool glProxyIsBlendEnabled()
{
	return glIsEnabled(GL_BLEND);
}

inline bool glProxyIsDepthTestEnabled()
{
	return glIsEnabled(GL_DEPTH_TEST);
}

inline bool glProxyIsCullFaceEnabled()
{
	return glIsEnabled(GL_CULL_FACE);
}

#else

void glProxyUseProgram(GLuint id);
void glProxyBindVertexArray(GLuint vao);

void glProxySetBlend(bool enabled);
void glProxySetDepthTest(bool enabled);
void glProxySetCullFace(bool enabled);

bool glProxyIsBlendEnabled();
bool glProxyIsDepthTestEnabled();
bool glProxyIsCullFaceEnabled();

#endif
