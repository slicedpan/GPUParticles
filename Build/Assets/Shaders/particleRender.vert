#version 130

layout(location = 0) in vec2 coord;
out vec3 colour;

uniform sampler2D positionTex;
uniform int rows;
uniform mat4 View;
uniform mat4 Projection;

void main()
{	
	vec2 pos = coord * 0.005;
	int x = gl_InstanceID  % rows;
	int y = gl_InstanceID  / rows;
	vec2 texCoord = vec2((x + 0.5) / rows, (y + 0.5) / rows);
	vec4 offset = vec4(texture(positionTex, texCoord).xyz, 1.0);
	colour = offset.xyz;	
	offset = Projection * View * offset;	
	gl_Position = offset + vec4(pos.x, pos.y, 0.0, 0.0);
} 
