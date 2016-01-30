/*
 * Camera
 *--------------
 * Camera which rotates around center.
 *
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "ext/OGL/gl_core_3_3.h"
#include "ext/GLFW/include/GLFW/glfw3.h"
#include "ext/GLM/glm/glm.hpp"
#include "ext/GLM/glm/gtc/matrix_transform.hpp"
#include "ext/GLM/glm/gtc/type_ptr.hpp"

const GLfloat CAMERA_BETA_BIAS = 0.0001f;

class Camera
{
public:
	Camera();
	~Camera();

	void init(glm::vec3 center, GLfloat alpha, GLfloat beta, GLfloat radius, GLfloat minRadius, GLfloat maxRadius);

	void reset(glm::vec3 center, GLfloat alpha, GLfloat beta, GLfloat radius);

	void setCenter(glm::vec3 center);
	void setAlpha(GLfloat alpha);
	void setBeta(GLfloat beta);
	void setRadius(GLfloat radius);

	glm::mat4 getViewMatrix();
	glm::vec3 getPosition();
	glm::vec3 getCenter() const;
	GLfloat getAlpha() const;
	GLfloat getBeta() const;
	GLfloat getRadius() const;

protected:
	void calcPosition();
	void clampValues();
	void clampAlpha();
	void clampBeta();
	void clampRadius();

	GLfloat alpha;
	GLfloat beta;
	GLfloat radius;
	GLfloat minRadius;
	GLfloat maxRadius;
	glm::vec3 center;
	glm::vec3 position;
};

#endif