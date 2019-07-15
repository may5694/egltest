#ifndef OPENGLCONTEXT_HPP
#define OPENGLCONTEXT_HPP

#include <EGL/egl.h>
#include <string>

// Uses EGL to create a windowless OpenGL context for offscreen rendering.
class OpenGLContext {
public:
	OpenGLContext(int major, int minor);
	~OpenGLContext();
	// Disallow copy, move, and assignment
	OpenGLContext(const OpenGLContext& o) = delete;
	OpenGLContext(OpenGLContext&& o) = delete;
	OpenGLContext& operator=(const OpenGLContext& o) = delete;
	OpenGLContext& operator=(OpenGLContext&& o) = delete;

	void makeCurrent();		// Make this context current
	bool isCurrent();		// Check if this context is current

private:
	EGLDisplay eglDpy;	// EGL context state
	EGLContext eglCtx;	// OpenGL context

	// Throws if EGL has encountered an error
	void checkEGLError(std::string what = {});
};

#endif
