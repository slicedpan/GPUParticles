#version 330

out vec3 outVel;
smooth in vec2 fragTexCoord;

uniform vec2 pixSize;
uniform sampler2D baseTex;
uniform sampler2D addTex;

void main()
{	
	vec3 currentPosition = texture(baseTex, fragTexCoord).xyz;
	vec3 p = currentPosition;
	p.y = 0;
	vec3 vel = vec3(0, 1, 0);
	outVel = currentPosition - normalize(cross(p, vel)) * 0.1;
}

