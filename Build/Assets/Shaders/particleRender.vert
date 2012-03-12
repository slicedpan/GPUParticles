#version 130

in vec2 coord;
out vec3 colour;

uniform sampler2D positionTex;

void main()
{	
	vec2 pos = coord;
	int x = gl_InstanceID % 2;
	int y = gl_InstanceID / 2;
	vec2 texCoord = vec2((x + 0.5) / 2.0, (y + 0.5) / 2.0);
	pos += texture(positionTex, texCoord).xy;
	colour = vec3(coord.x, coord.y, 0.0);
	gl_Position = vec4(pos.x , pos.y, 0.0, 1.0);
} 
