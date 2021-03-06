#include "Shader.h"

Shader::Shader()
{
	programInitialized = GL_FALSE;
	vertexArrayInitialized = GL_FALSE;

	textureSlotCounter = 0;
}

Shader::~Shader()
{
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteShader(vertexHandle);
	glDeleteShader(fragmentHandle);
	glDeleteProgram(programHandle);
}

void Shader::loadShaders(std::string vertexFile, std::string fragmentFile)
{
	// Create full relative pathes
	std::string vertexPath = SHADER_PATH + vertexFile;
	std::string fragmentPath = SHADER_PATH + fragmentFile;

	// If already initialized, delete old program
	if(programInitialized)
	{
		glDeleteShader(vertexHandle);
		glDeleteShader(fragmentHandle);
		glDeleteProgram(programHandle);
	}

	// Load data for vertex shader
	vertexHandle = glCreateShader(GL_VERTEX_SHADER);
	readShaderFromFile(vertexPath, vertexHandle);

	// Load data for fragment shader
	fragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);
	readShaderFromFile(fragmentPath, fragmentHandle);

	// Bind shaders
	programHandle = glCreateProgram();
	glAttachShader(programHandle, vertexHandle);
	glAttachShader(programHandle, fragmentHandle);
	glLinkProgram(programHandle);

	// Set program as initialized
	programInitialized = GL_TRUE;

	// Reset counter
	textureSlotCounter = 0;
}

void Shader::setVertexBuffer(GLfloat vertices[], GLuint size, std::string location)
{
	// If already initialized, delete old vertex array
	if(vertexArrayInitialized)
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteVertexArrays(1, &vertexArray);
	}

	// Create vertex array
	glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

	// Create and fill buffers
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Calc vertex count
	vertexCount = (size/sizeof(GLfloat))/3;

	// Activate attribute
	glEnableVertexAttribArray(0);
	glBindAttribLocation(programHandle, 0, location.c_str());

	// Unbind
	glBindVertexArray(0);

	// Set vertex array as initialized
	vertexArrayInitialized = GL_TRUE;
}

GLint Shader::getUniformHandle(std::string location) const
{
	return glGetUniformLocation(programHandle, location.c_str());
}

void Shader::setUniformValue(GLint uniformHandle, const glm::mat4& value)
{
	glUniformMatrix4fv(uniformHandle, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniformValue(GLint uniformHandle, const glm::vec2& value)
{
	glUniform2fv(uniformHandle, 1, glm::value_ptr(value));
}

void Shader::setUniformValue(GLint uniformHandle, const glm::vec3& value)
{
	glUniform3fv(uniformHandle, 1, glm::value_ptr(value));
}

void Shader::setUniformValue(GLint uniformHandle, const glm::vec4& value)
{
	glUniform4fv(uniformHandle, 1, glm::value_ptr(value));
}

void Shader::setUniformValue(GLint uniformHandle, const GLfloat& value)
{
	glUniform1f(uniformHandle, value);
}

void Shader::setUniformValue(GLint uniformHandle, const GLint& value)
{
	glUniform1i(uniformHandle, value);
}

void Shader::setUniformTexture(GLint uniformHandle, GLint textureHandle, GLenum mode)
{
	// Bind texture to given handle and slot
	glActiveTexture(GL_TEXTURE0 + textureSlotCounter);
	glBindTexture(mode, textureHandle);
	glUniform1i(uniformHandle, textureSlotCounter);

	textureSlotCounter++;
}

void Shader::use()
{
	glUseProgram(programHandle);
}

void Shader::draw(GLenum mode)
{
	glBindVertexArray(vertexArray);
	glDrawArrays(mode, 0, vertexCount);
	glBindVertexArray(0);
}

GLboolean Shader::readShaderFromFile(std::string file, GLint handle)
{
	// Open file
	std::ifstream is (file.c_str(), std::ifstream::in);
	if(is)
	{
		// Read data
		std::string text((std::istreambuf_iterator<GLchar>(is)),
			std::istreambuf_iterator<GLchar>());

		// String of data
		std::string data;

		// Push back shader code
		data += text;

		// Source
		std::vector<GLchar> buffer(data.size() + 1);
		std::copy(data.begin(), data.end(), buffer.begin());
		const GLchar* source = &buffer[0];

		// Size
		GLint size =(GLint) strlen(source);

		// Assign to handle
		glShaderSource(handle, 1, &source, &size);
		glCompileShader(handle);

		// Get length of compiling log
		GLint log_length = 0;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);

		if(log_length > 0)
		{
			// Copy log to chars
			GLchar *log = new GLchar[log_length];
			glGetShaderInfoLog(handle, log_length, NULL, log);
			
			// Print it
			std::cout << (file + std::string(" compiler message\n \n") + std::string(log)) << std::endl;

			// Delete chars
			delete[] log;
		}

		// Close file
		is.close();

		return true;
	}
	else
	{
		std::cout << std::string(file + std::string(" could not be opened")) << std::endl;
		return false;
	}
}