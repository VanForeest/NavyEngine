#version 430 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// Textura de salida: Imagen de escritura en punto flotante de 32 bits
layout(rgba32f, binding = 0) writeonly uniform image2D NoiseOutput;

// Uniforms globales de control
uniform uint Resolution;
uniform float Seed;

// Función hash para generar números pseudoaleatorios basados en la posición
vec2 Hash2D(vec2 p){
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453123);
}

// Transformación de Box-Muller para obtener ruido Gaussiano
vec2 BoxMuller(vec2 u){

    // Evitamos log(0) usando max con un valor muy pequeño
    float r = sqrt(-2.0 * log(max(u.x, 1e-6)));
    float theta = 2.0 * 3.14159265359 * u.y;
    return vec2(r * cos(theta), r * sin(theta));
}

void main(){

    // gl_GlobalInvocationID nos da la coordenada (x, y) del píxel actual
    ivec2 texelCoords = ivec2(gl_GlobalInvocationID.xy);

    // Validar límites de la textura
    if(texelCoords.x >= int(Resolution) || texelCoords.y >= int(Resolution)){
        return;
    }

    // Convertimos la posición a float para alimentar el generador de ruido
    vec2 pos = vec2(texelCoords);

    // 1. Crear dos muestras aleatorias independientes (distribución uniforme entre 0 y 1)
    vec2 uv1 = Hash2D(pos + vec2(Seed, Seed * 1.5));
    vec2 uv2 = Hash2D(pos - vec2(Seed * 2.3, Seed * 0.7));

    // 2. Aplicar Box-Muller para obtener dos pares gaussianos independientes
    vec2 gauss1 = BoxMuller(uv1);
    vec2 gauss2 = BoxMuller(uv2);

    // 3. Almacenar en la textura:
    // RG = Primer número complejo (h0)
    // BA = Segundo número complejo (h0 conjugado base)
    vec4 pixelData = vec4(gauss1.x, gauss1.y, gauss2.x, gauss2.y);
    
    imageStore(NoiseOutput, texelCoords, pixelData);
}