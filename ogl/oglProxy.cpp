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
	bool debugOutputPerformance = false;
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

void glProxyEnableDebugOutput(GLenum minSeverity, bool performance)
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	debugOutputMinSeverity = minSeverity;
	debugOutputPerformance = performance;
	glDebugMessageCallback([](GLenum, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
		if ((severity == GL_DEBUG_SEVERITY_MEDIUM && debugOutputMinSeverity == GL_DEBUG_SEVERITY_HIGH) ||
			(severity == GL_DEBUG_SEVERITY_LOW && (debugOutputMinSeverity == GL_DEBUG_SEVERITY_MEDIUM || debugOutputMinSeverity == GL_DEBUG_SEVERITY_HIGH)) ||
			(severity == GL_DEBUG_SEVERITY_NOTIFICATION && (debugOutputMinSeverity == GL_DEBUG_SEVERITY_LOW || debugOutputMinSeverity == GL_DEBUG_SEVERITY_MEDIUM || debugOutputMinSeverity == GL_DEBUG_SEVERITY_HIGH)))
			return;

		if (!debugOutputPerformance && type == GL_DEBUG_TYPE_PERFORMANCE)
			return;

		const char* typeStr = [&]() {
			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR: return "ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
			case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER: return "MARKER";
			case GL_DEBUG_TYPE_PUSH_GROUP: return "PUSH_GROUP";
			case GL_DEBUG_TYPE_POP_GROUP: return "POP_GROUP";
			case GL_DEBUG_TYPE_OTHER: return "OTHER";
			default: return "UNKNOWN";
			}
		}();

		const char* severityStr = [&]() {
			switch (severity)
			{
			case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
			case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
			case GL_DEBUG_SEVERITY_LOW: return "LOW";
			case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
			default: return "UNKNOWN";
			}
		}();

		std::cout << std::hex << "glDebug: type=" << typeStr << "; id=0x" << id << "; severity=" << severityStr << "; message=\"" << message << "\"" << std::endl;
	}, 0);
}

void glProxyDisableDebugOutput()
{
	glDisable(GL_DEBUG_OUTPUT);
	glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	debugOutputMinSeverity = 0;
	debugOutputPerformance = false;
}
