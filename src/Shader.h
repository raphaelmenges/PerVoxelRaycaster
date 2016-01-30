/*
 * Shader
 *--------------
 * Class for reading, compiling and using OpenGL shader.
 * Only vertex and fragment shader are supported. Mesh data is
 * bound to shader (not really good decision, but working for this project).
 *
 */

#ifndef SHADER_H_
#define SHADER_H_

#include "ext/OGL/gl_core_3_3.h"
#include "ext/GLFW/include/GLFW/glfw3.h"
#include "ext/GLM/glm/glm.hpp"
#include "ext/GLM/glm/gtc/matrix_transform.hpp"
#include "ext/GLM/glm/gtc/type_ptr.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

const std::string SHADER_PATH = std::string(RESOURCES_PATH) + "/";

class Shader
{
public:
	Shader();
	~Shader();

	void loadShaders(std::string vertexFile, std::string fragmentFile);

	void setVertexBuffer(GLfloat vertices[], GLuint size, std::string location);

	GLint getUniformHandle(std::string location) const;
	void setUniformValue(GLint uniformHandle, const glm::mat4& value);
	void setUniformValue(GLint uniformHandle, const glm::vec2& value);
	void setUniformValue(GLint uniformHandle, const glm::vec3& value);
	void setUniformValue(GLint uniformHandle, const glm::vec4& value);
	void setUniformValue(GLint uniformHandle, const GLfloat& value);
	void setUniformValue(GLint uniformHandle, const GLint& value);
	void setUniformTexture(GLint uniformHandle, GLint textureHandle, GLenum mode);

	void use();

	void draw(GLenum mode);

protected:
	GLboolean readShaderFromFile(std::string file, GLint handle);

	GLuint programHandle;
	GLuint vertexHandle;
	GLuint fragmentHandle;
	GLuint vertexArray;
	GLuint vertexBuffer;
	GLint vertexCount;
	GLboolean programInitialized;
	GLboolean vertexArrayInitialized;
	GLint textureSlotCounter;
};

#endif