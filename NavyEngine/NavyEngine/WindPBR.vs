#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
// location 5 and 6 are BoneIDs and Weights
layout (location = 7) in vec3 aPosTarget;

out vec3 WorldPos;
out vec3 Normal;
out vec2 TexUV;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 NormalMatrix;
uniform float time;

void main(){

    // Calculamos el desfase usando la coordenada X y Z original del vértice
    // Esto hace que cada árbol tenga un ciclo de viento ligeramente distinto
    float phaseOffset = aPos.x * 0.5 + aPos.z * 0.5;
    
    // Calculamos el peso del Morph Target usando una funcion senoidal suave
    // time * 2.0 define la velocidad del viento
    // (sin(...) + 1.0) / 2.0 mapea el seno de [-1, 1] a [0, 1]
    float windWeight = (sin(time * 2.0 + phaseOffset) + 1.0) / 2.0;

    // Mezclamos la posición base con la del Shape Key (Morph Target)
    vec3 finalPos = mix(aPos, aPosTarget, windWeight);

	WorldPos = vec3(model * vec4(finalPos, 1.0));
	Normal = NormalMatrix * aNormal;
	TexUV = aTexUV;

	gl_Position = projection * view * vec4(WorldPos, 1.0);
}
