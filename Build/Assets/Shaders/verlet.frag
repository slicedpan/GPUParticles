#version 330

layout(location = 0) out vec4 newPosition;
layout(location = 1) out vec4 newLastPosition;

smooth in vec2 fragTexCoord;

uniform sampler2D posTex;
uniform sampler2D lastPosTex;
uniform sampler2D noiseTex;
uniform float timeElapsed;
uniform float timeElapsedSquared;

uniform mat4 viewProj;

uniform vec3 normal = vec3(0.0, 1.0, 0.0);

void main()
{	
	vec4 currentPos = texture(posTex, fragTexCoord);	
	vec4 lastPos = texture(lastPosTex, fragTexCoord);
	
	if (lastPos.a < 0)
	{
		newPosition.xyz = vec3(0.0, -1000000.0, -1000000.0);
		newLastPosition.xyz = vec3(0.0, -1000000.0, -1000000.0);
	}
	else
	{	
		newLastPosition.xyz = currentPos.xyz;
		newPosition.a = currentPos.a;	//this is the particles colour value
		
		vec3 vel = (currentPos.xyz - lastPos.xyz);
		vec3 spotDist = vec3(0.0, 0.0, 0.0);
		float groundPen;
		if (abs(currentPos.x) < 10.0 && abs(currentPos.z) < 10.0)
		{
			groundPen = max(-currentPos.y, 0.0);
			spotDist.y = groundPen;
			groundPen = min(groundPen * 10000000, 1.0);
			vel -= normal * dot(vel, normal) * groundPen * 1.1 + (texture(noiseTex, fragTexCoord + newPosition.xy).xyz * 0.0001 - 0.000024) * groundPen;	
		}
		
		//vec3 d = currentPos.xyz - vec3(0, 0.5, 0);
		//vec3 accel = -(10 * d) / length(d);
		
		vec3 accel = vec3(0, -1, 0);
		//accel -= accel * dot(accel, -normal) * groundPen;
		
		newPosition.xyz = currentPos.xyz + vel + accel * timeElapsedSquared + spotDist;	
		newLastPosition.a = timeElapsed + lastPos.a;	//lifetime
	}
}

