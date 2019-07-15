#include "openglcontext.hpp"
#include <map>
using namespace std;

OpenGLContext::OpenGLContext(int major, int minor) {
	// Configuration attributes
	static const EGLint configAttribs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};

	// Context attributes
	const EGLint contextAttribs[] = {
		EGL_CONTEXT_MAJOR_VERSION, major,
		EGL_CONTEXT_MINOR_VERSION, minor,
		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
		EGL_NONE
	};

	// Get default display
	eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	checkEGLError("eglGetDisplay");
	if (eglDpy == EGL_NO_DISPLAY)
		throw runtime_error("Failed to get default EGL display");

	// Initialize EGL
	EGLint eglMajor, eglMinor;
	EGLBoolean ret = eglInitialize(eglDpy, &eglMajor, &eglMinor);
	checkEGLError("eglInitialize");
	if (ret != EGL_TRUE)
		throw runtime_error("Failed to initialize EGL");

	// Select an appropriate configuration
	EGLint numConfigs;
	EGLConfig eglCfg;
	eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);
	checkEGLError("eglChooseConfig");

	// Bind the API
	eglBindAPI(EGL_OPENGL_API);
	checkEGLError("eglBindAPI");

	// Create the context
	eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, contextAttribs);
	checkEGLError("eglCreateContext");
	if (eglCtx == EGL_NO_CONTEXT)
		throw runtime_error("Failed to create context");

	// Make the context current
	makeCurrent();
}

OpenGLContext::~OpenGLContext() {
	// Make not current anymore
	eglMakeCurrent(eglDpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//	checkEGLError("eglMakeCurrent");

	// Terminate display connection
	eglTerminate(eglDpy);
//	checkEGLError("eglTerminate");
}

void OpenGLContext::makeCurrent() {
	// Make this context current
	eglMakeCurrent(eglDpy, EGL_NO_SURFACE, EGL_NO_SURFACE, eglCtx);
	checkEGLError("eglMakeCurrent");
}

bool OpenGLContext::isCurrent() {
	// Get current context
	EGLContext ctx = eglGetCurrentContext();
	checkEGLError("eglGetCurrentContext");
	return (ctx == eglCtx);
}

// Reads the EGL error state and throws an exception if not EGL_SUCCESS
void OpenGLContext::checkEGLError(string what) {
	string prepend;
	if (!what.empty())
		prepend = what + ": ";

	// Map error constants to strings
	static const map<EGLint, string> errStrMap = {
		{ EGL_NOT_INITIALIZED, "EGL_NOT_INITIALIZED" },
		{ EGL_BAD_ACCESS, "EGL_BAD_ACCESS" },
		{ EGL_BAD_ALLOC, "EGL_BAD_ALLOC" },
		{ EGL_BAD_ATTRIBUTE, "EGL_BAD_ATTRIBUTE" },
		{ EGL_BAD_CONTEXT, "EGL_BAD_CONTEXT" },
		{ EGL_BAD_CONFIG, "EGL_BAD_CONFIG" },
		{ EGL_BAD_CURRENT_SURFACE, "EGL_BAD_CURRENT_SURFACE" },
		{ EGL_BAD_DISPLAY, "EGL_BAD_DISPLAY" },
		{ EGL_BAD_SURFACE, "EGL_BAD_SURFACE" },
		{ EGL_BAD_MATCH, "EGL_BAD_MATCH" },
		{ EGL_BAD_PARAMETER, "EGL_BAD_PARAMETER" },
		{ EGL_BAD_NATIVE_PIXMAP, "EGL_BAD_NATIVE_PIXMAP" },
		{ EGL_BAD_NATIVE_WINDOW, "EGL_BAD_NATIVE_WINDOW" },
		{ EGL_CONTEXT_LOST, "EGL_CONTEXT_LOST" },
	};

	// Get the error, return if no error
	EGLint err = eglGetError();
	if (err == EGL_SUCCESS) return;

	// Find the error code in the string map
	auto errIt = errStrMap.find(err);
	if (errIt != errStrMap.end())
		throw runtime_error(prepend + errIt->second);
	else
		// Error code not found,
		throw runtime_error(prepend + "unknown error: " + to_string(err));
}
