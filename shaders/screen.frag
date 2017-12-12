#version 430 core

in vec3 newPos;
in vec2 texCoord;

out vec4 outColor;

uniform sampler2D screenTexture;
uniform vec3 camPos;
uniform float time;

mat4 rotationMatrix(vec3 axis, float angle);
highp float rand(vec2 co);

void main(void)
{

	vec4 screen = texture(screenTexture, texCoord);
	float depth = screen.w;

	outColor = vec4(screen.xyz, 1.0);
	return;

	float h = 0.002;

	float sample1 = texture(screenTexture, texCoord + vec2(h,0)).w;
	float sample2 = texture(screenTexture, texCoord + vec2(-h,0)).w;
	float sample3 = texture(screenTexture, texCoord + vec2(0,h)).w;
	float sample4 = texture(screenTexture, texCoord + vec2(0,-h)).w;

	float sample5 = texture(screenTexture, texCoord + vec2(2*h,0)).w;
	float sample6 = texture(screenTexture, texCoord + vec2(-2*h,0)).w;
	float sample7 = texture(screenTexture, texCoord + vec2(0,2*h)).w;
	float sample8 = texture(screenTexture, texCoord + vec2(0,-2*h)).w;


	float avgerageDepth = (sample1 + sample2 + sample3 + sample4 + sample5 + sample6 + sample7 + sample8) * 0.125;

	float occlusion = depth - avgerageDepth;

	//occlusion = clamp(occlusion, 0.0, 1.0);
	
	//outColor = vec4(vec3(1.0 - occlusion * 1000), 1.0);
	//outColor = vec4(screen.xyz * (1.0 - occlusion * 300), 1.0);
	//outColor = vec4(vec3(1.0 -  depth), 1.0);
/*
	float nSamples = 10;
	float radiusPixels = 100;

	// Get normal and depth from texture.
	vec4 colorVal = texture(screenTexture, texCoord);
	float depth = colorVal.w;
	vec3 normal = vec3(colorVal);
	
	//If the sample is part of the backgrond, we don't need to calculate the AO.
	// if(depth > 0.99f)
	// {
	// 	outColor = vec4(1.0f);
	// 	return;
	// }
	
	// Initiate 4 sampling vectors.
	vec2[4] sampleVectors;
	sampleVectors[0] = normalize(vec2(0,1));
	sampleVectors[1] = normalize(vec2(1,0));
	sampleVectors[2] = normalize(vec2(0,-1));
	sampleVectors[3] = normalize(vec2(-1,0));

	// Set the radius and sample-length depending on the resolution of the image and the number of samples defined by the user.
	float windowResolution = 800.0f;
	float radius = (radiusPixels / windowResolution) / (depth * (5.0f - 0.1f));
	float sampleLength = radius / nSamples;

	// Rotate sampling-vectors around z-axis equally by a random amount.
	if(true)
	{
		float randomAngle = rand(texCoord) * 3.14f;
		for(int i = 0; i < 4; i++)
			sampleVectors[i] = vec2(vec4(sampleVectors[i], 0, 0) * rotationMatrix(vec3(0,0,1), randomAngle));
	}

	// Set the startpoint, the point of our current fragment.
	vec3 startPoint = vec3(texCoord, 1.0f - depth);
	float ambientOcclusion = 0.0f;


	// Loop through the sample vectors.
	for(int i = 0; i < 4; i++)
	{
		vec3 highestPointVec = startPoint;
		float sampleWeight = 1;
		float ambientOcclusionOneRayWeighted = 0.0f; // Contains the AO for one sample-ray, with weight.

		// Axises that represent the screens xyz in world coordinates.
		vec3 zAxis = normalize(camPos);
		vec3 xAxis = normalize(cross(vec3(0,1,0), zAxis));

		// Get the normal and camPos in screenspace.
		vec3 normalInScreenspace = normalize(vec3(dot(xAxis, normal), normal.y, dot(zAxis, normal)));
		
		// Compute tangent vector.
		vec3 orthogonalToTangent = normalize(cross(vec3(0,0,1), vec3(sampleVectors[i], 0.0f)));
		vec3 tangentVector = normalize(cross(orthogonalToTangent, normalInScreenspace));

		// Compute tangent angle.
		float tangentAngle = atan(tangentVector.z / length(tangentVector.xy));

		// Initialize horizonAngle with the value of tangentAngle.
		float horizonAngle = tangentAngle;

		// Calculate the AO of the first sample-point, use it to initialize the weighted AO-sum.
		float ambientOcclusionOneRay =  sin(horizonAngle) - sin(tangentAngle); //AO
		ambientOcclusionOneRayWeighted +=  ambientOcclusionOneRay * sampleWeight; //WAO


		// Walk along the vector.
		for(float j = sampleLength; j < radius; j += sampleLength)
		{
			vec2 texturePos = texCoord + sampleVectors[i] * j;
			vec4 samplePoint = texture(screenTexture, texturePos);
			vec3 sampleVector = vec3(texturePos, 1.0f - samplePoint.w);

			if(abs(sampleVector.z - startPoint.z) > radius / 2 || samplePoint.w > 0.99f)
				continue; 

			// If the horizonAngle of the sampled point is higher than the previous one, save it!
			vec3 horizonVector = sampleVector - startPoint;
			float sampledHorizonAngle = atan(horizonVector.z / length(horizonVector.xy));

			if(sampledHorizonAngle > horizonAngle)
			{
				horizonAngle = sampledHorizonAngle;
				sampleWeight = max(0.0f, 1.0f - pow((j / radius),2));
				ambientOcclusionOneRayWeighted += sampleWeight * (sin(horizonAngle) - sin(tangentAngle) - ambientOcclusionOneRay);
				ambientOcclusionOneRay =  sin(horizonAngle) - sin(tangentAngle);

			}
		}
		// Add the weighted sum of the sample-ray to the final AO.
		ambientOcclusion += ambientOcclusionOneRayWeighted;

	}

	// Divide the result by the number of sample-vectors.
	ambientOcclusion = ambientOcclusion / 4;

	outColor = vec4( vec3(1.0f - ambientOcclusion), 1.0f);
	*/
}

// Function for creating a rotation matrix for rotation around a given axis and angle.
// taken from http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

// Function for generating a random number depending on texture coordinates.
// taken from http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}
