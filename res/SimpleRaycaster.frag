#version 330 core

in vec3 position;
in vec3 direction;
out vec4 fragmentColor;

uniform sampler3D uniformVolume;
uniform sampler1D uniformTransferfunction;

const float stepSize = 0.002;
const float innerIterations = 5;
const float outerIterations = 100;

/** Main function */
void main()
{
	// Basic raycasting variables (all vectors are in model space)
	vec3 dir = normalize(direction);
	vec3 pos = position;
	float value;
	vec4 src;
	vec4 dst = vec4(0,0,0,0);

	// Sample along the ray
	for(int i = 0; i < outerIterations; i++)
	{
		// Inner iterations
		for(int j = 0; j < innerIterations; j++)
		{
			// Get value from volume
			value = texture(uniformVolume, pos).r;

			// Use value as input of transfer function
			src.rgba = texture(uniformTransferfunction, value).rgba;
			
			// Front-To-Back composition
			dst.rgb += (1.0-dst.a) * src.rgb * src.a;
			dst.a += (1.0-dst.a) * src.a;

			// Prepare for next sample
			pos += dir*stepSize;
		}

		// Check whether still in volume
		if(pos.x > 1 || pos.y > 1 || pos.z > 1 ||
			pos.x < 0 || pos.y < 0 || pos.z < 0)
		{
			break;
		}
	}

	// Output
	fragmentColor = vec4(dst.rgb, 1);
}