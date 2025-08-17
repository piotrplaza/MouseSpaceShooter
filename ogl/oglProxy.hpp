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

inline void glProxySetPointSmooth(bool enabled)
{
	if (enabled)
		glEnable(GL_POINT_SMOOTH);
	else
		glDisable(GL_POINT_SMOOTH);
}

inline void glProxySetLineSmooth(bool enabled)
{
	if (enabled)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);
}

inline void glProxyPointSize(GLfloat size)
{
	glPointSize(size);
}

inline void glProxyLineWidth(GLfloat width)
{
	glLineWidth(width);
}

inline GLuint glProxyGetCurrentProgramId()
{
	GLint id;
	glGetIntegerv(GL_CURRENT_PROGRAM, &id);
	return static_cast<GLuint>(id);
}

inline GLuint glProxyGetCurrentVertexArray()
{
	GLint id;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &id);
	return static_cast<GLuint>(id);
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

inline bool glProxyIsPointSmoothEnabled()
{
	return glIsEnabled(GL_POINT_SMOOTH);
}

inline bool glProxyIsLineSmoothEnabled()
{
	return glIsEnabled(GL_LINE_SMOOTH);
}

inline GLfloat glProxyGetPointSize()
{
	GLfloat size;
	glGetFloatv(GL_POINT_SIZE, &size);
	return size;
}

inline GLfloat glProxyGetLineWidth()
{
	GLfloat width;
	glGetFloatv(GL_LINE_WIDTH, &width);
	return width;

#else

void glProxyUseProgram(GLuint id);
void glProxyBindVertexArray(GLuint vao);
void glProxySetBlend(bool enabled);
void glProxySetDepthTest(bool enabled);
void glProxySetCullFace(bool enabled);
void glProxySetPointSmooth(bool enabled);
void glProxySetLineSmooth(bool enabled);
void glProxyPointSize(GLfloat size);
void glProxyLineWidth(GLfloat width);

GLuint glProxyGetCurrentProgramId();
GLuint glProxyGetCurrentVertexArray();
bool glProxyIsBlendEnabled();
bool glProxyIsDepthTestEnabled();
bool glProxyIsCullFaceEnabled();
bool glProxyIsPointSmoothEnabled();
bool glProxyIsLineSmoothEnabled();
GLfloat glProxyGetPointSize();
GLfloat glProxyGetLineWidth();

#endif

void glProxyEnableDebugOutput(GLenum minSeverity = GL_DEBUG_SEVERITY_LOW, bool performance = true);
void glProxyDisableDebugOutput();
