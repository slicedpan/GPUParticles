#version 330

in vec2 coord;
smooth out vec2 fragTexCoord;

uniform sampler2D positionTex;
uniform sampler2D lastPositionTex;
uniform sampler2D particleTypeTex;
uniform int rows;
uniform mat4 viewProj;
uniform float quadSize = 0.05;

uniform float maxLifeTime = 30.0;
uniform float numTypes = 256.0;

flat out vec4 colour;

void main()
{	
	fragTexCoord = coord;
	vec2 pos = coord * quadSize;
	int x = gl_InstanceID  % rows;
	int y = gl_InstanceID  / rows;
	vec2 texCoord = vec2(float(x) / rows, float(y) / rows);
	vec4 position = texture(positionTex, texCoord);
	vec4 offset = vec4(position.xyz, 1.0);
	
	float lifeTime = texture(lastPositionTex, texCoord).a;
	float type = position.a;
	vec2 typeTexCoord = vec2(0.0, type / numTypes);
	
	vec4 times = texture(particleTypeTex, typeTexCoord);	
	typeTexCoord.x = 0.25;
	vec4 colour1 = texture(particleTypeTex, typeTexCoord);
	typeTexCoord.x = 0.5;
	vec4 colour2 = texture(particleTypeTex,typeTexCoord);
	typeTexCoord.x = 0.75;
	vec4 colour3 = texture(particleTypeTex, typeTexCoord);
	vec4 colour4 = vec4(colour3.xyz, 0.0);
	lifeTime /= times.w;	
	if (lifeTime > 1.0)
		colour = vec4(0.0, 0.0, 0.0, 0.0);
	else	
		colour = mix(mix(mix(colour1, colour2, lifeTime), mix(colour2, colour3, lifeTime), lifeTime), mix(mix(colour2, colour3, lifeTime), mix(colour3, colour4, lifeTime), lifeTime), lifeTime);
	
	//colour = colour1 * (realTimes.x / times.x) + colour2 * (realTimes.y / (times.y - times.x)) + colour3 * (realTimes.z / (times.z - times.y - times.x)) + vec4(colour3.xyz, 0.0) * (realTimes.w / (times.w - times.z - times.y -times.x));	
	
	offset = viewProj * offset;		
	gl_Position = offset + vec4(pos.x, pos.y, 0.0, 0.0);
} 
