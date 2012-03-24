#version 330

out vec4 out_colour;
smooth in vec2 fragTexCoord;

uniform vec2 pixSize;
uniform sampler2D baseTex;
uniform sampler2D addTex;

void main()
{	
	out_colour = texture(baseTex, fragTexCoord + pixSize / 2.0);// + texture(addTex, fragTexCoord + pixSize / 2.0);	
}

