#version 130

in vec2 coord;
out vec2 texCoord;

uniform vec2 pixSize;
uniform sampler2D baseTex;

void main()
{		
	texCoord = coord / 2.0 + 0.5 + pixSize;
	gl_Position = vec4(coord.x, coord.y, 0.0, 1.0);
} 
