#version 430 core

// Tamaño de grupo local estándar
layout (local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba32f) uniform readonly image2D u_ButterflyTexture;

layout(binding = 1, rg32f) uniform image2D u_SourceTexture;
layout(binding = 2, rg32f) uniform image2D u_DestinationTexture;

// Uniforms de control
uniform int u_Stage;       // Etapa actual (0 a log2(N) - 1)
uniform int u_Log2N;       // Total de etapas (ej. 8)
uniform int u_Direction;   // 0 = Horizontal, 1 = Vertical
uniform int u_N;           // Tamaño de la textura (ej. 256) PUESTO EXPLICITAMENTE

// Multiplicación de números complejos
vec2 complexMultiply(vec2 c1, vec2 c2) {
    return vec2(c1.x * c2.x - c1.y * c2.y, c1.x * c2.y + c1.y * c2.x);
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    
    // Usamos u_N directamente en lugar de imageSize() para evitar fallos de driver
    if (pixelCoord.x >= u_N || pixelCoord.y >= u_N) {
        return;
    }

    int k = (u_Direction == 0) ? pixelCoord.x : pixelCoord.y;
    
    // 1. Muestrear la textura Butterfly
    vec4 butterfly = imageLoad(u_ButterflyTexture, ivec2(u_Stage, k));
    vec2 twiddle = butterfly.xy;
    
    ivec2 pCoord = pixelCoord;
    ivec2 qCoord = pixelCoord;
    
    if (u_Direction == 0) {
        pCoord.x = int(butterfly.z);
        qCoord.x = int(butterfly.w);
    } else {
        pCoord.y = int(butterfly.z);
        qCoord.y = int(butterfly.w);
    }
    
    // 2. Muestrear componentes complejas
    vec4 pData = imageLoad(u_SourceTexture, pCoord);
    vec4 qData = imageLoad(u_SourceTexture, qCoord);
    
    vec2 p = pData.rg;
    vec2 q = qData.rg;
    
    // 3. Operación Mariposa
    vec2 twiddleTimesQ = complexMultiply(twiddle, q);
    vec2 result = p + twiddleTimesQ;
    
    // 5. Guardar resultado
    imageStore(u_DestinationTexture, pixelCoord, vec4(result, 0.0, 0.0));
}