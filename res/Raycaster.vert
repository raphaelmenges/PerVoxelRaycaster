#version 330 core

layout (location = 0) in vec4 positionAttribute;
out vec3 position;
out vec3 direction;

uniform mat4 uniformModel;
uniform mat4 uniformView;
uniform mat4 uniformProjection;

uniform vec3 uniformCameraPosition;

void main()
{
	// Output for rasterization
	gl_Position = uniformProjection * uniformView * uniformModel * positionAttribute;

	// Startposition for rays
	position = positionAttribute.xyz;

	// Direction in model space
	direction = position - uniformCameraPosition;
}