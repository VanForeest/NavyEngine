#version 410 core

in vec3 WorldPos;

out vec4 FragColor;

uniform samplerCube SkyBoxMap; // Es lo que en otros lados llaman EnviromentMap, que npcs

void main(){

	vec3 SkyBoxColor = textureLod(SkyBoxMap, WorldPos, 0.0).rgb;

	//HDR ToneMap
	SkyBoxColor = SkyBoxColor / (SkyBoxColor + vec3(1.0));
	//Gamma Correction
	SkyBoxColor = pow(SkyBoxColor, vec3(1.0 / 2.2));

	FragColor = vec4(SkyBoxColor, 1.0);
}