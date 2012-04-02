#version 130

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 View;
uniform mat4 Projection;
uniform mat4 World;

smooth out vec3 out_normal;
smooth out float depth;

void main()
{			
	gl_Position = Projection * View * World * vec4(position, 1.0);
	out_normal = normal;
	depth = gl_Position.z;
} 
