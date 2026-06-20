#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rg32f) uniform readonly image2D u_InputTexture;
layout(binding = 1, rgba32f) uniform writeonly image2D u_OutputTexture; 

uniform int u_N;

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    
    if (pixelCoord.x >= u_N || pixelCoord.y >= u_N) {
        return;
    }

    // 1. Leer los datos puramente calculados por la IFFT
    vec2 ifftResult = imageLoad(u_InputTexture, pixelCoord).rg;
    
    // 2. Aplicar el escalado total bidimensional
    float scale = 1.0 / float(u_N * u_N);
    
    // 3. Permutación espacial para centrar el espectro (-1)^(x + y)
    float perms = ((pixelCoord.x + pixelCoord.y) % 2 == 1) ? -1.0 : 1.0;
    
    // 4. Resultado final (asumiendo que quieres el valor real en el canal R, o el complejo completo)
    vec2 finalData = ifftResult * scale * perms;

   
    // Guardamos en la textura final que usará tu NavyEngine para el renderizado
    imageStore(u_OutputTexture, pixelCoord, vec4(finalData, 0.0, 1.0));
    
}