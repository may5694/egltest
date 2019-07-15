#include <iostream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
#include "openglcontext.hpp"
#include "gl46.h"
using namespace std;

GLuint compileShader(GLenum type, string source);
GLuint linkProgram(vector<GLuint> shaders);

int main(int argc, char** argv) {
	try {

		OpenGLContext ctx(4, 6);

		cout << "OpenGL " << glGetString(GL_VERSION) << endl;

		int width = 300;
		int height = 300;
		glViewport(0, 0, width, height);
		glClearColor(0.4, 0.4, 0.4, 1.0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		struct vtx {
			glm::vec2 xy;
			glm::vec3 rgb;
		};
		vector<vtx> verts = {
			{ { -0.5, -0.5 }, { 1.0, 0.0, 0.0 } },
			{ { 0.5, -0.5 }, { 0.0, 1.0, 0.0 } },
			{ { 0.0, 0.75 }, { 0.0, 0.0, 1.0 } },
		};
		GLuint vbuf;
		glGenBuffers(1, &vbuf);
		glBindBuffer(GL_ARRAY_BUFFER, vbuf);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(verts[0]), verts.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(verts[0]), (GLvoid*)sizeof(glm::vec2));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		string vsh_str = R"(
			#version 460

			layout (location = 0) in vec2 pos;
			layout (location = 1) in vec3 col;

			smooth out vec3 fragCol;

			void main() {
				gl_Position = vec4(pos, 0.0, 1.0);
				fragCol = col;
			})";
		string fsh_str = R"(
			#version 460

			smooth in vec3 fragCol;

			out vec4 outCol;

			void main() {
				outCol = vec4(fragCol, 1.0);
			})";

		vector<GLuint> shaders;
		shaders.push_back(compileShader(GL_VERTEX_SHADER, vsh_str));
		shaders.push_back(compileShader(GL_FRAGMENT_SHADER, fsh_str));
		GLuint shader = linkProgram(shaders);
		for (auto s : shaders) glDeleteShader(s);
		shaders.clear();

		glUseProgram(shader);

		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		cv::Mat img(height, width, CV_8UC4);
		glBindTexture(GL_TEXTURE_2D, tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.data);
		glBindTexture(GL_TEXTURE_2D, 0);
		cv::flip(img, img, 0);
		cv::imwrite("test.png", img);

	} catch (const exception& e) {
		cerr << e.what() << endl;
		return -1;
	}

	return 0;
}


GLuint compileShader(GLenum type, string source) {
	const char* source_cstr = source.c_str();
	GLint length = source.length();

	// Compile the shader
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source_cstr, &length);
	glCompileShader(shader);

	// Make sure compilation succeeded
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		// Compilation failed, get the info log
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		vector<GLchar> logText(logLength);
		glGetShaderInfoLog(shader, logLength, NULL, logText.data());

		// Construct an error message with the compile log
		stringstream ss;
		string typeStr = "";
		switch (type) {
		case GL_VERTEX_SHADER:
			typeStr = "vertex"; break;
		case GL_GEOMETRY_SHADER:
			typeStr = "geometry"; break;
		case GL_FRAGMENT_SHADER:
			typeStr = "fragment"; break;
		}
		ss << "Error compiling " << typeStr << " shader!" << endl << endl << logText.data() << endl;

		// Cleanup shader and throw an exception
		glDeleteShader(shader);
		throw runtime_error(ss.str());
	}

	return shader;
}

GLuint linkProgram(vector<GLuint> shaders) {
	GLuint program = glCreateProgram();

	// Attach the shaders and link the program
	for (auto s : shaders)
		glAttachShader(program, s);
	glLinkProgram(program);

	// Detach shaders
	for (auto s : shaders)
		glDetachShader(program, s);

	// Make sure link succeeded
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		// Link failed, get the info log
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		vector<GLchar> logText(logLength);
		glGetProgramInfoLog(program, logLength, NULL, logText.data());

		// Construct an error message with the compile log
		stringstream ss;
		ss << "Error linking program!" << endl << endl << logText.data() << endl;

		// Cleanup program and throw an exception
		glDeleteProgram(program);
		throw runtime_error(ss.str());
	}

	return program;
}
