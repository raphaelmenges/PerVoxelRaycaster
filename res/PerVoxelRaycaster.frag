#version 330 core

in vec3 position;
in vec3 direction;
out vec4 fragmentColor;

uniform sampler3D uniformVolume;
uniform sampler1D uniformTransferfunction;

const float standardStepSize = 0.002f;
const vec3 res = vec3(128, 256, 256);
const int innerIterations = 10;
const int downscale = 1;

/** Main function */
void main()
{
	vec4 src;
	vec4 dst = vec4(0,0,0,0);
	float value;

	vec3 dir = normalize(direction);
	vec3 currPos = position;

	vec3 scaledRes = res / downscale;
	float stepSize;
	vec3 expandOfRay;
	float minimalExpansion;

	vec3 samplePos;	
	vec3 deltaPos;
	vec3 helpPos;

	vec3 currGridPos;
	vec3 nextGridPos;
	vec3 nextGridPosOffset;
	vec3 gridPosExpansionMask;
	vec3 t_nextGridPos;
	vec3 t_nextGridPosOffset;

	// Calc offset of grid positon
	if(dir.x > 0)
	{
		nextGridPosOffset.x = 1;
	}
	else
	{
		if(dir.x < 0)
		{
			nextGridPosOffset.x = -1;
		}	
		else
		{
			nextGridPosOffset.x = 10;
		}
	}
	
	if(dir.y > 0)
	{
		nextGridPosOffset.y = 1;
	}
	else
	{
		if(dir.y < 0)
		{
			nextGridPosOffset.y = -1;
		}	
		else
		{
			nextGridPosOffset.y = 10;
		}
	}
	
	if(dir.z > 0)
	{
		nextGridPosOffset.z = 1;
	}
	else
	{
		if(dir.z < 0)
		{
			nextGridPosOffset.z = -1;
		}	
		else
		{
			nextGridPosOffset.z = 10;
		}
	}
	
	// Calc offset of next pos
	t_nextGridPosOffset = nextGridPosOffset / scaledRes;

	// Get initial position in grid (round up)
	currGridPos = floor(currPos * scaledRes);
	if(dir.x < 0 && currPos.x != 1)
	{
		currGridPos.x += 1; 
	}
	if(dir.y < 0 && currPos.y != 1)
	{
		currGridPos.y += 1; 
	}
	if(dir.z < 0 && currPos.z != 1)
	{
		currGridPos.z += 1; 
	}

	// Initial next texture space position in grid
	t_nextGridPos = currGridPos / scaledRes;

	// Do sampling
	int outerIterations = int(max(max(scaledRes.x, scaledRes.y), scaledRes.z) * 1.74f / innerIterations);
	for(int i = 0; i < outerIterations; i++)
	{
		for(int j = 0; j < innerIterations; j++)
		{
			// Calc next grid position
			nextGridPos = currGridPos + nextGridPosOffset;

			// Calc texture space of next grid positions
			t_nextGridPos += t_nextGridPosOffset;

			// Calc delta pos
			deltaPos = t_nextGridPos - currPos;

			// Calc necessary expansion of ray to reach next grid positions
			expandOfRay = deltaPos / dir;

			// Calc minimum and extract step size
			minimalExpansion = min(min(expandOfRay.x, expandOfRay.y), expandOfRay.z);
			stepSize = minimalExpansion;

			// Use minimum to decide which component of grid pos has to be advanced
			gridPosExpansionMask = 1.0 - clamp(1000 * (expandOfRay - minimalExpansion), vec3(0, 0, 0), vec3(1, 1, 1));
			currGridPos += nextGridPosOffset * gridPosExpansionMask;

			// Calc new texture spaced positons
			currPos += stepSize * dir;
			samplePos = currPos - 0.5f * stepSize * dir;

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