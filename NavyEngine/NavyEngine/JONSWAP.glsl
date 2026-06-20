#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Texturas de entrada y salida
layout(binding = 0, rgba32f) uniform readonly image2D u_GaussianNoiseTex;
layout(binding = 1, rgba32f) uniform writeonly image2D u_OutputH0Tex;

// Parámetros compartidos desde C++
layout(std140, binding = 0) uniform OceanParameters{
    vec2  u_WindDirection;
    float u_WindSpeed;     // U_10
    float u_Fetch;         // F Distancia en metros
    float u_Gravity;       // g (9.81)
    float u_Gamma;         // gamma (típicamente 3.3)
    float u_L;             // Escala espacial del océano (ej. 1000.0)
    float A_Scale;         // Amplitud Scale
    int   u_N;             // Resolución (ej. 256 en nuestro caso)
};

const float PI = 3.14159265359;

// Función para calcular el espectro JONSWAP unidimensional combinado con dispersión direccional
float JonswapSpectrum(vec2 k_vec){

    float k = length(k_vec);
    if (k < 0.0001) return 0.0;

    // 1. Relación de dispersión para obtener la frecuencia angular
    float omega = sqrt(u_Gravity * k);

    // 2. Calcular parámetros base de JONSWAP
    // Frecuencia pico (omega_p)
    float omega_p = 22.0 * pow((u_Gravity * u_Gravity) / (u_WindSpeed * u_Fetch), 1.0 / 3.0);
    
    // Parámetro alpha de escala
    float alpha = 0.076 * pow((u_Gravity * u_Fetch) / (u_WindSpeed * u_WindSpeed), -0.22);

    // 3. Componente PM (Pierson-Moskowitz) base
    float sigma = (omega <= omega_p) ? 0.07 : 0.09;
    float r = exp(-pow(omega - omega_p, 2.0) / (2.0 * sigma * sigma * omega_p * omega_p));
    
    // Espectro JONSWAP unidimensional S(omega)
    float S_js = (alpha * u_Gravity * u_Gravity / pow(omega, 5.0)) * exp(-1.25 * pow(omega_p / omega, 4.0)) * pow(u_Gamma, r);

    // 4. Esparcimiento direccional D(theta) = (2/PI) * cos^2(theta)
    vec2 k_norm = normalize(k_vec);
    vec2 w_norm = normalize(u_WindDirection);
    float cos_theta = dot(k_norm, w_norm);
    float cos_half_theta_sq = (1.0 + cos_theta) * 0.5; 

    // Elevamos para ajustar la apertura (a mayor exponente, olas más alineadas)
    float D_theta = (1.0 / PI) * pow(cos_half_theta_sq, 6.0); //Valor recomendado 4.0 - 6.0

    //float D_theta = (2.0 / PI) * pow(max(cos_theta, 0.0), 2.0);
    //float D_theta = (2.0 / PI) * pow(cos_theta, 2.0);
    
    // 5. Transformación al espacio bidimensional de vectores de onda.
    // El jacobiano de la transformación requiere dividir por k y ajustar por la relación d_omega/d_k
    // S(k) = S(omega) * (d_omega / d_k) -> d_omega/d_k = g / (2 * omega)
    float d_omega_dk = u_Gravity / (2.0 * omega);
    
    return S_js * d_omega_dk * D_theta / k;
}

void main(){

    ivec2 texel_coord = ivec2(gl_GlobalInvocationID.xy);
    if (texel_coord.x >= u_N || texel_coord.y >= u_N) return;

    // 1. Mapear coordenadas del texel a frecuencias de Fourier [-N/2, N/2 - 1]
    vec2 f;
    f.x = (texel_coord.x < u_N / 2) ? float(texel_coord.x) : float(texel_coord.x - u_N);
    f.y = (texel_coord.y < u_N / 2) ? float(texel_coord.y) : float(texel_coord.y - u_N);
    
    vec2 k = f * (2.0 * PI / u_L);

    // 2. Coordenadas inversas para -k con wrapping toroidal
    ivec2 texel_coord_neg = (ivec2(u_N) - texel_coord) % u_N;
    
    vec2 f_neg;
    f_neg.x = (texel_coord_neg.x < u_N / 2) ? float(texel_coord_neg.x) : float(texel_coord_neg.x - u_N);
    f_neg.y = (texel_coord_neg.y < u_N / 2) ? float(texel_coord_neg.y) : float(texel_coord_neg.y - u_N);
    
    vec2 k_neg = f_neg * (2.0 * PI / u_L);

    // 3. Muestrear el ruido Gaussiano original (Textura de entrada)
    vec4 noise     = imageLoad(u_GaussianNoiseTex, texel_coord);
    vec4 noise_neg = imageLoad(u_GaussianNoiseTex, texel_coord_neg);

    // 4. Calcular la raíz cuadrada de la densidad espectral de JONSWAP
    float sqrt_P     = sqrt(JonswapSpectrum(k));
    float sqrt_P_neg = sqrt(JonswapSpectrum(k_neg));

    // 5. Generar h0(k) y h0(-k) usando los componentes Gaussianos independientes (r, g)
    // Se multiplica por (1 / sqrt(2)) porque combinamos dos variables aleatorias independientes
    vec2 h0_k     = (noise.xy * (1.0 / sqrt(2.0))) * sqrt_P;
    vec2 h0_neg_k = (noise_neg.xy * (1.0 / sqrt(2.0))) * sqrt_P_neg;

    // 6. Conjugado complejo de h0(-k) -> (Real, -Imaginaria)
    vec2 h0_minus_k_conj = vec2(h0_neg_k.x, -h0_neg_k.y);

    // 7. Escribir el resultado final en los canales RGBA

    //Amplitud Scale Module
    float u_AmplitudScale = A_Scale;   
    //float u_AmplitudScale = 0.04;
    h0_k *= u_AmplitudScale;
    h0_minus_k_conj *= u_AmplitudScale;
    imageStore(u_OutputH0Tex, texel_coord, vec4(h0_k.x, h0_k.y, h0_minus_k_conj.x, h0_minus_k_conj.y));

    //Shift_coord Module
    //ivec2 shift_coord;
    //shift_coord.x = (texel_coord.x < (u_N / 2)) ? (texel_coord.x + (u_N / 2)) : (texel_coord.x - (u_N / 2));
    //shift_coord.y = (texel_coord.y < (u_N / 2)) ? (texel_coord.y + (u_N / 2)) : (texel_coord.y - (u_N / 2));
    //imageStore(u_OutputH0Tex, shift_coord, vec4(h0_k.x, h0_k.y, h0_minus_k_conj.x, h0_minus_k_conj.y));
}