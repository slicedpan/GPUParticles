#version 330

layout(location = 0) in vec2 coord;

uniform int texSide;
uniform int particleNum;

flat out int currentNum;
flat out float colourValue;

void main()
{		
	currentNum = particleNum + gl_InstanceID;	
	int xCoord = currentNum % texSide;
	int yCoord = currentNum / texSide;	
	
	currentNum %= (texSide * texSide);
	
	vec2 pixSize = vec2(1.0 / texSide, 1.0 / texSide) * 2.0;
	
	vec2 pos = vec2(float(xCoord) / float(texSide), float(yCoord) / float(texSide)) * 2.0;
	pos.x -= 1.0;
	pos.y -= 1.0;	

	colourValue = float(currentNum % 64) / 64.0;	
	
	gl_Position = vec4(pos.x + coord.x * pixSize.x, pos.y + coord.y * pixSize.y, 0.0, 1.0);
} 