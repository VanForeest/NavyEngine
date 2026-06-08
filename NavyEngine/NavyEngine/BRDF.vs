#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexUV;

out vec2 TexUV;

void main(){

	TexUV = aTexUV;
	gl_Position = vec4(aPos, 1.0);
}