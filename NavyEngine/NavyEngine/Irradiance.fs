#version 410 core

in vec3 WorldPos;

out vec4 FragColor;

uniform samplerCube SkyBoxMap;

const float PI = 3.14159265359;

void main(){

	vec3 N = normalize(WorldPos);
	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = normalize(cross(N, right));

	float sampleDelta = 0.025f;
	float nrSamples = 0.0f;

	vec3 tangentSample;
    vec3 sampleVec;

	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta){
	
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta){
		
			tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += texture(SkyBoxMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}

	irradiance = PI * irradiance * (1.0 / nrSamples);

	FragColor = vec4(irradiance, 1.0);

}

