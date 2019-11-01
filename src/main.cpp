/** includes */
#include "ext/OGL/gl_core_3_3.h"
#include "ext/GLFW/include/GLFW/glfw3.h"
#include "ext/GLM/glm/glm.hpp"
#include "ext/GLM/glm/gtc/matrix_transform.hpp"
#include "ext/GLM/glm/gtc/type_ptr.hpp"
#include "ext/picoPNG/picopng.h"

#include "Shader.h"
#include "Cube.h"
#include "Camera.h"

#include <string>

/** Variables */
GLFWwindow* pWindow;
Shader shader;
Camera camera;

GLfloat prevTime;
GLfloat deltaTime;
GLint prevWidth;
GLint prevHeight;

glm::mat4 uniformModel;
glm::mat4 uniformView;
glm::mat4 uniformProjection;
GLuint uniformVolume;
GLuint uniformTransferfunction;

GLint uniformModelHandle;
GLint uniformViewHandle;
GLint uniformProjectionHandle;
GLint uniformCameraPositionHandle;
GLint uniformVolumeHandle;
GLint uniformTransferfunctionHandle;

GLboolean buttonPressed = GL_FALSE;
GLfloat cursorX;
GLfloat cursorY;
GLfloat cursorDeltaX;
GLfloat cursorDeltaY;
GLfloat prevCursorX;
GLfloat prevCursorY;

/** GLFW callback for cursor */
static void cursorCallback(GLFWwindow* pWindow, GLdouble xpos, GLdouble ypos)
{
	cursorX = (GLfloat)xpos;
	cursorY = (GLfloat)ypos;
}

/** GLFW callback for mouse buttons */
static void buttonsCallback(GLFWwindow* pWindow, GLint button, GLint action, GLint mods)
{
	if(button == GLFW_MOUSE_BUTTON_1)
	{
		if(action == GLFW_PRESS)
		{
			buttonPressed = GL_TRUE;
		}
		else if(action == GLFW_RELEASE)
		{
			buttonPressed = GL_FALSE;
		}
	}
}

/** GLFW callback for mouse scrolling */
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.setRadius(camera.getRadius() - 0.1f * (float)yoffset);
}


/** main */
int main()
{
	// GLFW and GLEW initialization
	glfwInit();

	// Core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Window creation
	pWindow = glfwCreateWindow(1280, 720, "PerVoxelRaycaster", NULL, NULL);
	if (!pWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(pWindow);
	ogl_LoadFunctions();
	glfwSetCursorPosCallback(pWindow, cursorCallback);
	glfwSetMouseButtonCallback(pWindow, buttonsCallback);
	glfwSetScrollCallback(pWindow, scrollCallback);

	// OpenGL initialization
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1);

	// Setup shader

	// ### CHOOSE FROM ONE OF THESE THREE SHADERS ###
	//shader.loadShaders("Raycaster.vert", "SimpleRaycaster.frag");
	//shader.loadShaders("Raycaster.vert", "SimplePerVoxelRaycaster.frag");
	shader.loadShaders("Raycaster.vert", "PerVoxelRaycaster.frag");

	shader.setVertexBuffer(cube::vertices, sizeof(cube::vertices), "positionAttribute");
	uniformModelHandle= shader.getUniformHandle("uniformModel");
	uniformViewHandle= shader.getUniformHandle("uniformView");
	uniformProjectionHandle = shader.getUniformHandle("uniformProjection");
	uniformCameraPositionHandle= shader.getUniformHandle("uniformCameraPosition");
	uniformVolumeHandle= shader.getUniformHandle("uniformVolume");
	uniformTransferfunctionHandle= shader.getUniformHandle("uniformTransferfunction");
	shader.use();

	// Initialize camera
	camera.init(glm::vec3(0.5f), glm::radians(-135.0f), glm::radians(80.0f), 2, 0.5f, 5);

	// Other initializations
	prevCursorX = cursorX;
	prevCursorY = cursorY;

	// Read volume
	const GLuint volumeX = 256;
	const GLuint volumeY = 256;
	const GLuint volumeZ = 256;

	GLuint voxelCount = volumeX * volumeY * volumeZ;
	GLubyte* volumeData = new GLubyte[voxelCount];
	std::ifstream rawDataFile(std::string(RESOURCES_PATH) + "/bonsai.raw", std::ios::in|std::ios::binary);
	rawDataFile.read((GLchar*)volumeData, static_cast<GLuint>(voxelCount) * sizeof(GLubyte));

	// Create volume 3D texture
	glGenTextures(1, &uniformVolume);
	glBindTexture(GL_TEXTURE_3D, uniformVolume);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volumeX, volumeY, volumeZ, 0, GL_RED, GL_UNSIGNED_BYTE, volumeData);
	glBindTexture(GL_TEXTURE_3D, 0);
	delete[] volumeData;
	
	// Set volume in shader
	shader.setUniformTexture(uniformVolumeHandle, uniformVolume, GL_TEXTURE_3D);

	// Read transfer function
	std::vector<GLubyte> transferfunction;
	unsigned long transferfunctionX;
	unsigned long transferfunctionY;

	std::ifstream in(std::string(std::string(RESOURCES_PATH) + "/Transferfunction.png").c_str(), std::ios::in|std::ios::binary);
	in.seekg(0, std::ios::end);
	std::streamsize size = in.tellg();
	in.seekg(0, std::ios::beg);
	std::vector<GLchar> buffer(static_cast<GLuint>(size));
	in.read(&(buffer[0]), static_cast<size_t>(size));
	in.close();
	decodePNG(transferfunction, transferfunctionX, transferfunctionY, reinterpret_cast<GLubyte*>(&(buffer[0])), static_cast<size_t>(size), GL_FALSE);

	// Create transfer function 1D texture
	glGenTextures(1, &uniformTransferfunction);
	glBindTexture(GL_TEXTURE_1D, uniformTransferfunction);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, transferfunctionX, 0, GL_RGBA, GL_UNSIGNED_BYTE, &transferfunction[0]);
	glBindTexture(GL_TEXTURE_1D, 0);

	// Set transfer function in shader
	shader.setUniformTexture(uniformTransferfunctionHandle, uniformTransferfunction, GL_TEXTURE_1D);

	// Loop
	while(!glfwWindowShouldClose(pWindow))
	{
		// Clear buffers
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		// Get window resolution
		GLint width, height;
		glfwGetWindowSize(pWindow, &width, &height);

		// Give OpenGL the window resolution
		if(width != prevWidth || height != prevHeight)
		{
			glViewport(0, 0, width, height);
			prevWidth = width;
			prevHeight = height;
		}

		// Calc time per frame
		GLfloat currentTime = (GLfloat)glfwGetTime();
		deltaTime = currentTime - prevTime;
		prevTime = currentTime;

		std::cout << 1.0f / deltaTime << std::endl;

		// Calculate cursor movement
		cursorDeltaX = cursorX - prevCursorX;
		cursorDeltaY = cursorY - prevCursorY;
		prevCursorX = cursorX;
		prevCursorY = cursorY;

		// Model matrix
		uniformModel = glm::mat4(1.0f);

		// View matrix
		if(buttonPressed)
		{
			camera.setAlpha(camera.getAlpha() + 0.005f * cursorDeltaX);
			camera.setBeta(camera.getBeta() - 0.005f * cursorDeltaY);
		}
		uniformView = camera.getViewMatrix();

		// Projection matrix
		uniformProjection = glm::perspective(glm::radians(30.f), ((GLfloat)width/(GLfloat)height), 0.1f, 100.f);

		// Set updated uniforms in shader
		shader.setUniformValue(uniformModelHandle, uniformModel);
		shader.setUniformValue(uniformViewHandle, uniformView);
		shader.setUniformValue(uniformProjectionHandle, uniformProjection);
		shader.setUniformValue(uniformCameraPositionHandle, camera.getPosition());

		// Draw cube
		shader.draw(GL_TRIANGLES);

		// GLFW updates
		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}

	// Clean up
	glfwDestroyWindow(pWindow);
	glfwTerminate();

	return 0;
}