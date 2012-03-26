#version 330

layout(location = 0) out vec4 newPosition;
layout(location = 1) out vec4 newLastPosition;

smooth in vec2 fragTexCoord;

uniform sampler2D posTex;
uniform sampler2D lastPosTex;
uniform float timeElapsed;
uniform float timeElapsedSquared;

uniform vec3 normal = vec3(0.0, 1.0, 0.0);

void main()
{	
	vec4 currentPos = texture(posTex, fragTexCoord);	
	vec4 lastPos = texture(lastPosTex, fragTexCoord);
	
	if (lastPos.a <= 0)
		discard;		
	
	newLastPosition.xyz = currentPos.xyz;
	newPosition.a = currentPos.a;	//this is the particles colour value	
		
	vec3 vel = (currentPos.xyz - lastPos.xyz);
	float groundPen = max(-currentPos.y, 0.0);
	vec3 spotDist = vec3(0, groundPen, 0);
	groundPen = min(groundPen * 100000, 1.0);
	vel -= normal * dot(vel, normal) * groundPen;
	
	
	/*vec3 d = currentPos.xyz - vec3(0, 0.5, 0);
	vec3 accel = -(10 * d) / length(d);*/
	
	vec3 accel = vec3(0, -10, 0);
	accel -= accel * groundPen;
	
	newPosition.xyz = currentPos.xyz + vel + accel * timeElapsedSquared + spotDist;	
	newLastPosition.a = lastPos.a - timeElapsed;	//lifetime
}

