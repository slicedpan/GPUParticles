#version 330

in vec2 coord;
out float colour;
out vec2 fragTexCoord;
out float baseAlpha;

uniform sampler2D positionTex;
uniform sampler2D lastPositionTex;
uniform int rows;
uniform mat4 View;
uniform mat4 Projection;

uniform float maxLifeTime = 30.0;

void main()
{	
	fragTexCoord = coord;
	vec2 pos = coord * 0.05;
	int x = gl_InstanceID  % rows;
	int y = gl_InstanceID  / rows;
	vec2 texCoord = vec2((x + 0.5) / rows, (y + 0.5) / rows);
	vec4 position = texture(positionTex, texCoord);
	vec4 offset = vec4(position.xyz, 1.0);
	colour = position.a;	
	offset = Projection * View * offset;	
	baseAlpha = texture(lastPositionTex, texCoord).a / maxLifeTime;
	gl_Position = offset + vec4(pos.x, pos.y, 0.0, 0.0);
} 
