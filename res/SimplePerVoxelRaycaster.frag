#version 330 core

in vec3 position;
in vec3 direction;
out vec4 fragmentColor;

uniform sampler3D uniformVolume;
uniform sampler1D uniformTransferfunction;

const float standardStepSize = 0.008;
const vec3 res = vec3(128, 256, 256);
const int innerIterations = 5;

/** Main function */
void main()
{
	// Basic raycasting variables (all vectors are in model/texture space)
	vec3 dir = normalize(direction);
	vec3 currPos = position;
	float value;
	vec4 src;
	vec4 dst = vec4(0,0,0,0);
	vec3 delta = vec3(0,0,0);
	vec3 indDelta = vec3(0,0,0);
	vec3 samplePos;
	bool exit = false;
	float stepSize;
	vec3 nextGridPos;
	vec3 nextPos;
	vec3 deltaPos;
	vec3 expandOfDirection;
	vec3 helpPos;

	// Get initial position in grid (round up)
	vec3 currGridPos = floor(currPos * res);
	if(dir.x < 0 && currPos.x !=1)
	{
		currGridPos.x += 1; 
	}
	if(dir.y < 0 && currPos.y !=1)
	{
		currGridPos.y += 1; 
	}
	if(dir.z < 0 && currPos.z !=1)
	{
		currGridPos.z += 1; 
	}

	// Do sampling
	float outerIterations = max(max(res.x, res.y), res.z) * 1.74 / innerIterations;
	for(int i = 0; i < outerIterations; i++)
	{
		for(int j = 0; j < innerIterations; j++)
		{
			// Calc next grid position
			nextGridPos = currGridPos;

			dir.x > 0 ? nextGridPos.x += 1 : nextGridPos.x -= 1;
			dir.y > 0 ? nextGridPos.y += 1 : nextGridPos.y -= 1;
			dir.z > 0 ? nextGridPos.z += 1 : nextGridPos.z -= 1;

			// Calc texture space of next grid positions
			nextPos = nextGridPos / res;

			// Calc delta pos
			deltaPos = nextPos - currPos;

			// Calc necessary expansion of direction to reach next grid positions
			expandOfDirection = vec3(1000, 1000, 1000);

			// Case when direction parallel to volume
			dir.x != 0 ? expandOfDirection.x = deltaPos.x / dir.x : 0;
			dir.y != 0 ? expandOfDirection.y = deltaPos.y / dir.y : 0;
			dir.z != 0 ? expandOfDirection.z = deltaPos.z / dir.z : 0;

			// Calc minimum and extract step size
			if(expandOfDirection.x < expandOfDirection.y && expandOfDirection.x < expandOfDirection.z)
			{
				// x
				currGridPos.x = nextGridPos.x;
				stepSize = expandOfDirection.x;
			}
			else if(expandOfDirection.y < expandOfDirection.x && expandOfDirection.y < expandOfDirection.z)
			{
				// y
				currGridPos.y = nextGridPos.y;
				stepSize = expandOfDirection.y;
			}
			else
			{
				// z
				currGridPos.z = nextGridPos.z;
				stepSize = expandOfDirection.z;
			}

			// Get median of position
			helpPos = currPos;
			currPos += stepSize * dir;
			samplePos = (helpPos + currPos) / 2.0f;

			// Get value from volume
			value = texture(uniformVolume, samplePos).r;

			// Use value as input of transfer function
			src.rgba = texture(uniformTransferfunction, value).rgba;

			// Take step size into account of alpha value
			src.a = min(src.a, 0.99999f);
			src.a = 1-pow(1-src.a, stepSize/standardStepSize);

			// Front-To-Back composition
			dst.rgb += (1.0-dst.a) * src.rgb * src.a;
			dst.a += (1.0-dst.a) * src.a; 
		}

		// Test for break
		if(currPos.x > 1 || currPos.y > 1 || currPos.z > 1 ||
			currPos.x < 0 || currPos.y < 0 || currPos.z < 0)
		{
			break;
		}
	}

	// Output
	fragmentColor = vec4(dst.rgb, 1);
}