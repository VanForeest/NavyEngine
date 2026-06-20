#version 430 core

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0) uniform sampler2D u_DisplacementMap; 

// Pendientes analíticas de la altura obtenidas de las IFFTs de derivadas
layout(binding = 1) uniform sampler2D u_SlopeXMap; // Contiene dDy/dx
layout(binding = 2) uniform sampler2D u_SlopeZMap; // Contiene dDy/dz

layout(binding = 3, rgba32f) writeonly uniform image2D u_NormalMap; // R=Nx, G=Ny, B=Nz
layout(binding = 4, r32f) writeonly uniform image2D u_FoamMap;   // R=Intensidad de espuma

uniform float u_GridSpacing;   // Distancia entre vértices en el mundo (ej. 1.0)
uniform float u_FoamThreshold; // Umbral del Jacobiano (típicamente entre 0.5 y 0.9)

void main(){

    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec2 texSize = vec2(textureSize(u_DisplacementMap, 0));

    if (texelCoord.x >= texSize.x || texelCoord.y >= texSize.y) return;

    // Coordenadas UV con el centro del píxel
    vec2 texelSize = 1.0 / texSize;
    vec2 uv = (vec2(texelCoord) + 0.5) * texelSize;

    // 1. OBTENER LAS DERIVADAS DE ALTURA (Y) ANALÍTICAS
    // Ya vienen escaladas correctamente de la IFFT de frecuencias
    float dYdx = texture(u_SlopeXMap, uv).r;
    float dYdz = texture(u_SlopeZMap, uv).r;

    // 2. MUESTREO DE VECINOS HORIZONTALES (Para derivadas de X y Z)
    // Usamos textureOffset con Wrapping (GL_REPEAT configurado en C++)
    vec3 dispL = textureOffset(u_DisplacementMap, uv, ivec2(-1,  0)).rgb; // Izquierda
    vec3 dispR = textureOffset(u_DisplacementMap, uv, ivec2( 1,  0)).rgb; // Derecha
    vec3 dispB = textureOffset(u_DisplacementMap, uv, ivec2( 0, -1)).rgb; // Abajo
    vec3 dispT = textureOffset(u_DisplacementMap, uv, ivec2( 0,  1)).rgb; // Arriba

    // Distancia matemática para diferencias centrales (2 * paso de malla)
    float deltaX = 2.0 * u_GridSpacing;
    float deltaZ = 2.0 * u_GridSpacing;

    // Derivadas del desplazamiento X respecto a x y z
    float dXdx = (dispR.x - dispL.x) / deltaX;
    float dXdz = (dispT.x - dispB.x) / deltaZ;

    // Derivadas del desplazamiento Z respecto a x y z
    float dZdx = (dispR.z - dispL.z) / deltaX;
    float dZdz = (dispT.z - dispB.z) / deltaZ;

    // =================================================================
    // 3. CÁLCULO DE LA NORMAL CON CORRECCIÓN POR CHOPPINESS
    // =================================================================
    // Basado en el producto cruz de los vectores tangentes reales de la superficie:
    // T_x = [1 + dXdx, dYdx, dZdx]
    // T_z = [dXdz,     dYdz, 1 + dZdz]
    // N = T_z x T_x (Da una normal apuntando hacia +Y)
    
    vec3 normal;
    normal.x = -dYdx * (1.0 + dZdz) + dYdz * dZdx;
    normal.y = (1.0 + dXdx) * (1.0 + dZdz) - dXdz * dZdx;
    normal.z = -dYdz * (1.0 + dXdx) + dYdx * dXdz; 
    normal = normalize(normal);
    vec3 packedNormal = vec3(normal.x, normal.z, normal.y);

    //vec3 normal = normalize(vec3(-dYdx, 1.0, -dYdz));
    //vec3 normal = vec3(-dYdx, -dYdz, 1.0);
    //normal = normalize(normal);
    //vec3 packedNormal = vec3(normal.x, normal.z, normal.y) * 0.5 + 0.5;
    
    // Guardar el resultado 
    imageStore(u_NormalMap, texelCoord, vec4(packedNormal, 1.0));

    // =================================================================
    // 4. CÁLCULO DEL JACOBIANO (DETECCIÓN DE ESPUMA)
    // =================================================================
    // El determinante Jacobiano mide la distorsión del área. 
    // Si J == 1, no hay distorsión horizontal.
    // Si J < 1, los vértices se están comprimiendo (cresta de la ola).
    // Si J <= 0, la ola se está cruzando sobre sí misma (rompimiento severo).
    
    float jacobian = (1.0 + dXdx) * (1.0 + dZdz) - (dXdz * dZdx);

    // Mapear el Jacobiano a la intensidad de la espuma.
    // Si el Jacobiano es menor que el umbral, generamos espuma.
    float foamIntensity = 0.0;
    if (jacobian < u_FoamThreshold) {
        // La espuma es proporcional a qué tan fuerte es la compresión
        foamIntensity = (u_FoamThreshold - jacobian) / u_FoamThreshold;
    }
    
    // Sujetar valores por seguridad
    foamIntensity = clamp(foamIntensity, 0.0, 1.0);

    // Guardar la intensidad de la espuma (Textura R32F de un solo canal)
    imageStore(u_FoamMap, texelCoord, vec4(foamIntensity, 0.0, 0.0, 0.0));

    //se que jode mucho tener comentarios asi de grandes y especificos, pero solo tuve una semana para estudiar
    //aveces se me olvida lo que tengo que hacer
}