/* LineShader.cpp
Michael Zahniser, 23 Oct 2013

Function definitions for the LineShader class.
*/

#include "LineShader.h"

#include "Screen.h"
#include "Shader.h"

#include <stdexcept>

using namespace std;

namespace {
	Shader shader;
	GLint scaleI;
	GLint startI;
	GLint lengthI;
	GLint widthI;
	GLint colorI;
	
	GLuint vao;
	GLuint vbo;
}



void LineShader::Init()
{
	static const char *vertexCode =
		"#version 130\n"
		"uniform vec2 scale;\t"
		"uniform vec2 start;\t"
		"uniform vec2 len;\t"
		"uniform vec2 width;\t"
		
		"in vec2 vert;\n"
		"out vec2 tpos;\n"
		"out float tscale;\n"
		
		"void main() {\n"
		"  tpos = vert;\n"
		"  tscale = length(len);\n"
		"  gl_Position = vec4((start + vert.x * len + vert.y * width) * scale, 0, 1);\n"
		"}\n";

	static const char *fragmentCode =
		"#version 130\n"
		"uniform vec4 color = vec4(1, 1, 1, 1);\t"
		
		"in vec2 tpos;\n"
		"in float tscale;\n"
		"out vec4 finalColor;\n"
		
		"void main() {\n"
		"  float alpha = min(tscale - abs(tpos.x * (2 * tscale) - tscale), 1 - abs(tpos.y));\n"
		"  finalColor = color * alpha;\n"
		"}\n";
	
	shader = Shader(vertexCode, fragmentCode);
	scaleI = shader.Uniform("scale");
	startI = shader.Uniform("start");
	lengthI = shader.Uniform("len");
	widthI = shader.Uniform("width");
	colorI = shader.Uniform("color");
	
	// Generate the vertex data for drawing sprites.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	GLfloat vertexData[] = {
		0.f, -1.f,
		1.f, -1.f,
		0.f,  1.f,
		1.f,  1.f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(shader.Attrib("vert"));
	glVertexAttribPointer(shader.Attrib("vert"), 2, GL_FLOAT, GL_FALSE,
		2 * sizeof(GLfloat), NULL);
	
	// unbind the VBO and VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void LineShader::Draw(Point from, Point to, float width, const float *color)
{
	if(!shader.Object())
		throw runtime_error("LineShader: Draw() called before Init().");
	
	glUseProgram(shader.Object());
	glBindVertexArray(vao);
	
	GLfloat scale[2] = {2.f / Screen::Width(), -2.f / Screen::Height()};
	glUniform2fv(scaleI, 1, scale);
	
	GLfloat start[2] = {static_cast<float>(from.X()), static_cast<float>(from.Y())};
	glUniform2fv(startI, 1, start);
	
	Point v = to - from;
	Point u = v.Unit() * width;
	from -= u;
	to += u;
	GLfloat length[2] = {static_cast<float>(v.X()), static_cast<float>(v.Y())};
	glUniform2fv(lengthI, 1, length);
	
	GLfloat w[2] = {static_cast<float>(u.Y()), static_cast<float>(-u.X())};
	glUniform2fv(widthI, 1, w);
	
	static const float white[4] = {1.f, 1.f, 1.f, 1.f};
	glUniform4fv(colorI, 1, color ? color : white);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	glBindVertexArray(0);
	glUseProgram(0);
}
