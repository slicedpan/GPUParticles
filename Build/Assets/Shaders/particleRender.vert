#version 330

in vec2 coord;
out float colour;
out vec2 fragTexCoord;
out float baseAlpha;

uniform sampler2D positionTex;
uniform sampler2D lastPositionTex;
uniform int rows;
uniform mat4 viewProj;
uniform float quadSize = 0.05;

uniform float maxLifeTime = 30.0;

void main()
{	
	fragTexCoord = coord;
	vec2 pos = coord * quadSize;
	int x = gl_InstanceID  % rows;
	int y = gl_InstanceID  / rows;
	vec2 texCoord = vec2(float(x) / rows, float(y) / rows);
	vec4 position = texture(positionTex, texCoord);
	vec4 offset = vec4(position.xyz, 1.0);
	colour = position.a;	
	offset = viewProj * offset;	
	baseAlpha = texture(lastPositionTex, texCoord).a / maxLifeTime;
	gl_Position = offset + vec4(pos.x, pos.y, 0.0, 0.0);
} 
