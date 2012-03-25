#version 330

out vec4 out_colour;
in vec2 fragTexCoord;
in float colour;
in float baseAlpha;

uniform sampler2D positionTex;
uniform sampler2D maskTex;
uniform vec3 colour1 = vec3(0.0, 0.8, 0.3);
uniform vec3 colour2 = vec3(0.0, 0.0, 1.0);

void main()
{	
	vec3 baseColour = mix(colour1, colour2, colour);
	if (baseAlpha < 0)
		discard;
	out_colour = vec4(baseColour, texture(maskTex, fragTexCoord).r * baseAlpha);
}

