#version 330

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec4 out_lastPosition;
smooth in vec2 fragTexCoord;

uniform vec2 pixSize;
uniform sampler2D baseTex;
uniform sampler2D velocityTex;

void main()
{	
	out_position.xyz = texture(baseTex, fragTexCoord).xyz;
	out_position.a = mod(length(out_position.xyz), 1.0);
	vec3 p = out_position.xyz;
	vec3 vel = texture(velocityTex, fragTexCoord).xyz;
	out_lastPosition.xyz = out_position.xyz - normalize(cross(p, vel)) * 0.01;
	out_lastPosition.a = 30.0;
}

