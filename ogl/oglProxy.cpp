#include "oglProxy.hpp"

#include <iostream>

#ifndef GL_USE_PROGRAM_PROXY_OPTIMISATION_DISABLED

namespace
{
	GLuint currentProgramId = 0;
	GLuint currentVAO = 0;
	bool blend = false;
	bool depthTest = false;
	bool cullFace = false;
	bool pointSmooth = false;
	bool lineSmooth = false;
	GLfloat pointSize = 0.0f;
	GLfloat lineWidth = 0.0f;
	GLenum debugOutputMinSeverity = 0;
}

void glProxyUseProgram(GLuint id)
{
	if (id != currentProgramId)
	{
		glUseProgram(id);
		currentProgramId = id;
	}
}

void glProxyBindVertexArray(GLuint vao)
{
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

void glProxySetPointSmooth(bool enabled)
{
	if (enabled != pointSmooth)
	{
		if (enabled)
			glEnable(GL_POINT_SMOOTH);
		else
			glDisable(GL_POINT_SMOOTH);
		pointSmooth = enabled;
	}
}

void glProxySetLineSmooth(bool enabled)
{
	if (enabled != lineSmooth)
	{
		if (enabled)
			glEnable(GL_LINE_SMOOTH);
		else
			glDisable(GL_LINE_SMOOTH);
		lineSmooth = enabled;
	}
}

void glProxyPointSize(GLfloat size)
{
	if (size != pointSize)
	{
		glPointSize(size);
		pointSize = size;
	}
}

void glProxyLineWidth(GLfloat width)
{
	if (width != lineWidth)
	{
		glLineWidth(width);
		lineWidth = width;
	}
}

GLuint glProxyGetCurrentProgramId()
{
	return currentProgramId;
}

GLuint glProxyGetCurrentVertexArray()
{
	return currentVAO;
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

bool glProxyIsPointSmoothEnabled()
{
	return pointSmooth;
}

bool glProxyIsLineSmoothEnabled()
{
	return lineSmooth;
}

GLfloat glProxyGetPointSize()
{
	return pointSize;
}

GLfloat glProxyGetLineWidth()
{
	return lineWidth;
}

#endif

void glProxyEnableDebugOutput(GLenum minSeverity)
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	debugOutputMinSeverity = minSeverity;
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		if ((severity == GL_DEBUG_SEVERITY_MEDIUM && debugOutputMinSeverity == GL_DEBUG_SEVERITY_HIGH) ||
			(severity == GL_DEBUG_SEVERITY_LOW && (debugOutputMinSeverity == GL_DEBUG_SEVERITY_MEDIUM || debugOutputMinSeverity == GL_DEBUG_SEVERITY_HIGH)) ||
			(severity == GL_DEBUG_SEVERITY_NOTIFICATION && (debugOutputMinSeverity == GL_DEBUG_SEVERITY_LOW || debugOutputMinSeverity == GL_DEBUG_SEVERITY_MEDIUM || debugOutputMinSeverity == GL_DEBUG_SEVERITY_HIGH)))
			return;

		std::cout << std::hex << "glDebug: source=0x" << source << "; type=0x" << type << "; id=0x" << id << "; severity=0x" << severity << "; message=\"" << message << "\"" << std::endl;
	}, 0);
}

void glProxyDisableDebugOutput()
{
	glDisable(GL_DEBUG_OUTPUT);
	glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	debugOutputMinSeverity = 0;
}
