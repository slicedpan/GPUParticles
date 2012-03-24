#version 330

out vec3 newPosition;
smooth in vec2 fragTexCoord;

uniform vec2 pixSize;
uniform sampler2D posTex;
uniform sampler2D lastPosTex;
uniform float timeElapsed;

void main()
{	
	vec3 currentPosition = texture(posTex, fragTexCoord + pixSize / 2.0).xyz;
	vec3 lastPosition = texture(lastPosTex, fragTexCoord + pixSize / 2.0).xyz;	
	vec3 vel = (currentPosition - lastPosition);	
	newPosition = currentPosition + vel + vec3(0, -10, 0) * 0.000256 ;
}

