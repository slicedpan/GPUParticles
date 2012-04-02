#version 330

layout(location = 0) out vec4 newPosition;
layout(location = 1) out vec4 newLastPosition;

void main()
{
	newPosition = vec4(0.0, -1000000.0, -1000000.0, 0.0);
	newLastPosition = vec4(0.0, -1000000.0, -1000000.0, 0.0);
}