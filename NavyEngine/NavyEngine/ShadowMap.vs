#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexUV;
layout (location = 7) in vec3 aPosTarget;

out vec2 TexUV;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform float time;

void main()
{
    // Calculamos el viento de la misma manera que WindPBR.vs para que la sombra coincida con la malla
    float phaseOffset = aPos.x * 0.5 + aPos.z * 0.5;
    float windWeight = (sin(time * 2.0 + phaseOffset) + 1.0) / 2.0;
    vec3 finalPos = mix(aPos, aPosTarget, windWeight);

    TexUV = aTexUV;
    gl_Position = lightSpaceMatrix * model * vec4(finalPos, 1.0);
}
