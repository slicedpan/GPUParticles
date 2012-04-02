#version 330

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec4 out_lastPosition;
smooth in vec2 fragTexCoord;

uniform vec2 pixSize;
uniform sampler2D baseTex;
uniform sampler2D velocityTex;
uniform sampler2D noiseTex;

void main()
{	
	out_position.xyz = texture(baseTex, fragTexCoord).xyz;
	vec3 noise = texture(noiseTex, fragTexCoord).xyz - 0.5;
	out_position.a = mod(length(out_position.xyz + noise) * 256.0, 256.0);
	vec3 p = out_position.xyz;
	vec3 vel = texture(velocityTex, fragTexCoord).xyz + noise * 0.01;
	out_lastPosition.xyz = out_position.xyz - vel;
	out_lastPosition.a = 0.0;
}

