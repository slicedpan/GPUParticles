#version 330

out vec4 out_colour;
smooth in vec2 fragTexCoord;
flat in vec4 colour;

uniform sampler2D positionTex;
uniform sampler2D maskTex;

void main()
{		
	out_colour = vec4(colour.xyz, texture(maskTex, fragTexCoord).r * colour.a);
}

