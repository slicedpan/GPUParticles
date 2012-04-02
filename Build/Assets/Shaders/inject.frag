#version 330

uniform vec3 position;
uniform vec3 velocity;
uniform float maxLifeTime;
uniform int noiseSize;

uniform sampler2D noiseTex;

flat in int currentNum;
flat in float colourValue;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec4 out_lastPosition;

void main()
{	
	float noiseCoordx = currentNum % noiseSize;
	float noiseCoordy = currentNum / noiseSize;
	vec3 velNoise = texture(noiseTex, vec2(noiseCoordx / noiseSize, noiseCoordy / noiseSize)).xyz;
	
	out_position.xyz = position + velNoise * 0.06;
	out_lastPosition.xyz = position - velocity + velNoise * 0.05;
	out_position.a = mod(colourValue * velNoise.x + velNoise.y * 10.0, 256.0) ;
	out_lastPosition.a = 0.0;
}