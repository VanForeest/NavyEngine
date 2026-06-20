#version 430 core

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

layout(binding = 0) uniform sampler2D MovementTexture;
layout(binding = 1) uniform sampler2D NormalMap;

vec3 getNormalFromMap(){

    vec3 packedNormal = texture(NormalMap, aTexUV).xyz;
   
    // Recordando cómo lo guardamos en el Compute Shader:
    // packedNormal.r (X) -> Era normal.x
    // packedNormal.g (Y) -> Era normal.z
    // packedNormal.b (Z) -> Era normal.y (Cielo)
    
    vec3 worldNormal;
    worldNormal.x = packedNormal.x; // Eje X del mundo
    worldNormal.y = packedNormal.z; // Eje Y del mundo (El componente que apunta al cielo)
    worldNormal.z = packedNormal.y; // Eje Z del mundo
    
    // 4. Asegurar que sea un vector unitario tras cualquier filtrado de textura
    return normalize(worldNormal);
}

void main(){

	vec3 PositionFromTexture = texture(MovementTexture, aTexUV).rgb;

	//WorldPos = vec3(model * vec4(PositionFromTexture, 1.0));
    //vec3 PreNormal = getNormalFromMap();
    //Normal = NormalMatrix * PreNormal;

    WorldPos = PositionFromTexture + aPos;
    Normal = getNormalFromMap();
	TexUV = aTexUV;

	gl_Position = projection * view * vec4(WorldPos, 1.0);
		
}