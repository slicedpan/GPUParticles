#version 130

layout(location = 0) in vec3 position;

uniform mat4 View;
uniform mat4 Projection;

void main()
{		
	gl_Position = Projection * View * position;
} 
