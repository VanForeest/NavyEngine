#version 430 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexUV;

#define NR_MAX_LIGHTS 4

// Propiedades del material
layout(binding = 2) uniform sampler2D FoamMap;

// IBL
layout(binding = 3) uniform samplerCube irradianceMap;
layout(binding = 4) uniform samplerCube prefilterMap;
layout(binding = 5) uniform sampler2D brdfLUT;

uniform vec3 u_WaterColor;
uniform vec3 u_FoamColor;
uniform float BaseRo;
uniform float FoamRo;
uniform float u_metallic;
uniform float u_ao;
uniform vec3 u_FF0;


//Struct de las luces
struct DirecLight{
    vec3 Color;
    vec3 Direction;
};

struct PointLight{
    vec3 Color;
    vec3 Position;
    float Range;
};

uniform DirecLight dirlight;
uniform PointLight polight[NR_MAX_LIGHTS];
uniform vec3 camPos;

const float PI = 3.14159265359;


float DistributionGGX(vec3 N, vec3 H, float roughness){
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness){
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness){
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 ComputeDirecLight(DirecLight dilight, vec3 N, vec3 V, vec3 F0, vec3 worldpos, vec3 albedo, float rough, float metalic){
    vec3 L = normalize(dilight.Direction);
    vec3 H = normalize(V + L);

    vec3 radiance = dilight.Color;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, rough);   
    float G = GeometrySmith(N, V, L, rough);    
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 para prevenir division por 0
    vec3 specular = numerator / denominator;
        
    // kS es igual al Fresnel, creo
    vec3 kS = F;
        
    //Como aqui se respetan las leyes de optica y la ley de la consevacion de energia, la suma de
    //el valor de la especular y el valor de la difusa no puede sobrepasar 1 (a menos que emita luz el objeto)
    vec3 kD = vec3(1.0) - kS;

    //Multiplicamos kd por la inversa del valor de metalic puesto que solo los no metales tienen difusa y parcialmente
    //los semi-metalicos, los metalicos puros no tienen difusa;
    kD *= 1.0 - metalic;	                
           
    // escala la light por NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // sumamos los valores obtenidos para L0
    //A este punto ya multiplicamos el BRDF por el fresnel de Ks, entonces por eso no sale de nuevo
    vec3 finalresult = (kD * albedo / PI + specular) * radiance * NdotL; //esto va para Lo

    return finalresult;
}

vec3 ComputePointLight(PointLight polight, vec3 N, vec3 V, vec3 F0, vec3 worldpos, vec3 albedo, float rough, float metalic){

    vec3 L = normalize(polight.Position - WorldPos);
    vec3 H = normalize(V + L);

    float distance = length(polight.Position - WorldPos);
    float factor = distance / polight.Range;
    float attenuation = clamp(1.0 - factor, 0.0, 1.0);
    attenuation = attenuation * attenuation;
    vec3 radiance = polight.Color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, rough);   
    float G = GeometrySmith(N, V, L, rough);    
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 para prevenir division por 0
    vec3 specular = numerator / denominator;
        
    // kS es igual al Fresnel, creo
    vec3 kS = F;
        
    //Como aqui se respetan las leyes de optica y la ley de la consevacion de energia, la suma de
    //el valor de la especular y el valor de la difusa no puede sobrepasar 1 (a menos que emita luz el objeto)
    vec3 kD = vec3(1.0) - kS;

    //Multiplicamos kd por la inversa del valor de metalic puesto que solo los no metales tienen difusa y parcialmente
    //los semi-metalicos, los metalicos puros no tienen difusa;
    kD *= 1.0 - metalic;	                
           
    // escala la light por NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // sumamos los valores obtenidos para L0
    //A este punto ya multiplicamos el BRDF por el fresnel de Ks, entonces por eso no sale de nuevo
    vec3 finalresult = (kD * albedo / PI + specular) * radiance * NdotL; //esto va para Lo

    return finalresult;
}

void main(){
    
    // input lighting data
    vec3 N = Normal;      
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);
    vec3 Lo = vec3(0.0);

    // Obtenemos los valores del material

    float foamIntensity = texture(FoamMap, TexUV).r;
    float facing = max(dot(N, V), 0.0);

    vec3 deepWaterColor = vec3(0.005, 0.03, 0.08); // Azul profundo
    vec3 shallowWaterColor = u_WaterColor;          // color base actual

    vec3 currentWaterColor = mix(deepWaterColor, shallowWaterColor, facing);
    vec3 albedo = mix(currentWaterColor, u_FoamColor, foamIntensity);
    
    //vec3 albedo = mix(u_WaterColor, u_FoamColor, foamIntensity);
    //vec3 albedo = u_WaterColor;

    float baseRoughness = BaseRo; // Agua perfectamente pulida  0.05
    float foamRoughness = FoamRo;  // Espuma totalmente mate/rugosa 0.9
    float roughness = mix(baseRoughness, foamRoughness, foamIntensity);
    //float roughness = BaseRo;


    float ao = u_ao; //1.0
    float metallic = u_metallic;

    vec3 F0 = u_FF0; // 0.02
    F0 = mix(F0, albedo, metallic);

    //Colaboracion del DirLight
    Lo += ComputeDirecLight(dirlight, N, V, F0, WorldPos, albedo, roughness, metallic);

    //calculo del factor Dispersion
    // En ComputeDirecLight o directamente en el main:
    vec3 L = normalize(dirlight.Direction);

    // Cuanto más mires hacia la luz a través de la ola, más brilla el agua por dentro
    float sssFactor = max(dot(V, -L), 0.0); 
    sssFactor = pow(sssFactor, 4.0); // Concentrar el efecto en el ángulo correcto

    // Un color turquesa/celeste brillante para la luz transmitida
    vec3 sssColor = vec3(0.0, 0.2, 0.1) * dirlight.Color; 
    vec3 sssFinal = sssFactor * sssColor * (1.0 - foamIntensity); // Que no afecte a la espuma

    //Colaboracion de los PointsLight
    /*for(int i = 0; i < 4; ++i){
        // calcula per-light radiance
        Lo += ComputePointLight(polight[i], N, V, F0, WorldPos, albedo, roughness, metallic);
    }*/
      
    
    // ambient lighting (ahora usamos IBL para el ambient factor)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
      
    // combinamos los prefilters maps con el lut para obtener la parte especular de la ecuacion;
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    //vec3 specular = prefilteredColor * F;
    
    vec3 ambient = (kD * diffuse + specular) * ao; 
    
    vec3 color = ambient + Lo + sssFinal;

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color , 1.0);
}