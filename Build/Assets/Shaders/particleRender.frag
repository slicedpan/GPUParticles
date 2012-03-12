varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;


void main()
{
	vec3 n,halfV;
	float NdotL,NdotHV;

	vec4 color;
	color = vec4(0.0, 0.0, 0.0, 1.0);

	n = normalize(normal);	

	NdotL = max(dot(n,lightDir),0.0);

	if (NdotL > 0.0) 
	{
		color = diffuse * NdotL;
		halfV = normalize(halfVector);
		NdotHV = max(dot(n,halfV),0.0);
		color += vec4(1.0, 1.0, 1.0, 1.0) * pow(NdotHV, 8.0);
	}
	
	gl_FragColor = color;
}

