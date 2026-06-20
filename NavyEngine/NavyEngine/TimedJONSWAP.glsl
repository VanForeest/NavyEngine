#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

// Texturas de entrada (Espectro JONSWAP estático)
layout(binding = 0, rgba32f) uniform readonly image2D img_h0_k;

// Texturas de salida (Los 3 componentes de desplazamiento para la IFFT)
layout(binding = 1, rg32f) uniform writeonly image2D img_H_t_X; // Desplazamiento Horizontal (X)
layout(binding = 2, rg32f) uniform writeonly image2D img_H_t_Y; // Desplazamiento Vertical (Y)
layout(binding = 3, rg32f) uniform writeonly image2D img_H_t_Z; // Desplazamiento Horizontal (Z)

//Texturas de salida (Los 2 espectros para las normales)
layout(binding = 4, rg32f) uniform writeonly image2D SlopeSpectrumX; //Espectro para normales en X
layout(binding = 5, rg32f) uniform writeonly image2D SlopeSpectrumZ; //Espectro para normales en Z

uniform float u_time;
uniform uint Resolution;
uniform float u_L_X;
uniform float u_L_Z;

const float GRAVITY = 9.81;
const float PI = 3.14159265359;

void main() {
    ivec2 texel_coord = ivec2(gl_GlobalInvocationID.xy);
    
    if (texel_coord.x >= Resolution || texel_coord.y >= Resolution) {
        return;
    }

    // 1. Calcular el vector de onda k

    //Metodo 1
    //int halfRes = int(Resolution) / 2;
    //float n = (texel_coord.x < halfRes) ? float(texel_coord.x) : float(texel_coord.x) - float(Resolution);
    //float m = (texel_coord.y < halfRes) ? float(texel_coord.y) : float(texel_coord.y) - float(Resolution);  
    //vec2 k = vec2(2.0 * PI * n / u_L_X, 2.0 * PI * m / u_L_Z);
    //float k_len = length(k);

  
    //Metodo 2
    //float n = (texel_coord.x < Resolution / 2) ? float(texel_coord.x) : float(texel_coord.x) - float(Resolution);
    //float m = (texel_coord.y < Resolution / 2) ? float(texel_coord.y) : float(texel_coord.y) - float(Resolution);
    float n = texel_coord.x - (Resolution / 2.0);
    float m = texel_coord.y - (Resolution / 2.0);
    vec2 k = vec2(2.0 * PI * n / u_L_X, 2.0 * PI * m / u_L_Z); //Equivale a: kx = k.x ; kz = k.y
    float k_len = length(k);

    // Si el vector de onda es casi cero (frecuencia cero/DC), vaciamos las texturas y salimos
    if (k_len < 0.0001) {
        imageStore(img_H_t_Y, texel_coord, vec4(0.0));
        imageStore(img_H_t_X, texel_coord, vec4(0.0));
        imageStore(img_H_t_Z, texel_coord, vec4(0.0));
        imageStore(SlopeSpectrumX, texel_coord, vec4(0.0));
        imageStore(SlopeSpectrumZ, texel_coord, vec4(0.0));
        return;
    }

    // 2. Relación de dispersión y rotación de fase temporal
    float omega = sqrt(GRAVITY * k_len);
    float omegat = omega * u_time;
    float cos_ot = cos(omegat);
    float sin_ot = sin(omegat);

    vec2 exp_iwt = vec2(cos_ot, sin_ot);
    vec2 exp_minus_iwt = vec2(cos_ot, -sin_ot);

    // 3. Leer componentes estáticos
    vec2 h0 = imageLoad(img_h0_k, texel_coord).rg;
    vec2 h0_conj = imageLoad(img_h0_k, texel_coord).ba;

    // 4. Multiplicación compleja para h(k, t)
    vec2 term1 = vec2(h0.x * exp_iwt.x - h0.y * exp_iwt.y, 
                      h0.x * exp_iwt.y + h0.y * exp_iwt.x);
                      
    vec2 term2 = vec2(h0_conj.x * exp_minus_iwt.x - h0_conj.y * exp_minus_iwt.y, 
                      h0_conj.x * exp_minus_iwt.y + h0_conj.y * exp_minus_iwt.x);

    // Componente Vertical Final: h_y(k, t)
    vec2 h_y = term1 + term2;

    // 5. Calcular los Desplazamientos Horizontales con Choppy Waves
    // k normalizado
    vec2 k_norm = k / k_len; 

    // Multiplicación por 'i': i * (Real + i*Imag) = -Imag + i*Real
    // h_x = i * (k_x / |k|) * h_y
    vec2 h_x = vec2(-h_y.y * k_norm.x, h_y.x * k_norm.x);
    
    // h_z = i * (k_z / |k|) * h_y
    vec2 h_z = vec2(-h_y.y * k_norm.y, h_y.x * k_norm.y);

    // 6. Multiplicación analítica por 'i * kx' -> i * (a + bi) = (-b + ai)
    vec2 slopeX = vec2(-h_y.y * k.x, h_y.x * k.x);
    
    // Multiplicación analítica por 'i * kz' -> i * (a + bi) = (-b + ai)
    vec2 slopeZ = vec2(-h_y.y * k.y, h_y.x * k.y);

    // 7. Almacenar los resultados en sus mapas espectrales correspondientes
    imageStore(img_H_t_Y, texel_coord, vec4(h_y, 0.0, 0.0));
    imageStore(img_H_t_X, texel_coord, vec4(h_x, 0.0, 0.0));
    imageStore(img_H_t_Z, texel_coord, vec4(h_z, 0.0, 0.0));
    imageStore(SlopeSpectrumX, texel_coord, vec4(slopeX, 0.0, 0.0));
    imageStore(SlopeSpectrumZ, texel_coord, vec4(slopeZ, 0.0, 0.0));
}