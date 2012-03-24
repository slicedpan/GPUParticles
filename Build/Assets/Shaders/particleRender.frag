#version 330

out vec4 out_colour;
in vec2 fragTexCoord;
in float colour;
in float baseAlpha;

uniform sampler2D positionTex;
uniform sampler2D maskTex;
uniform vec3 colour1 = vec3(1.0, 0.0, 0.0);
uniform vec3 colour2 = vec3(0.6, 0.9, 0.1);

void main()
{	
	vec3 baseColour = mix(colour1, colour2, colour);
	if (baseAlpha < 0)
		discard;
	out_colour = vec4(baseColour, texture(maskTex, fragTexCoord).r * baseAlpha);
}

