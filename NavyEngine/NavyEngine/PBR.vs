#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexUV;

out vec3 WorldPos;
out vec3 Normal;
out vec2 TexUV;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 NormalMatrix;

void main(){

	WorldPos = vec3(model * vec4(aPos, 1.0));
	Normal = NormalMatrix * aNormal;
	TexUV = aTexUV;

	gl_Position = projection * view * vec4(WorldPos, 1.0);

}