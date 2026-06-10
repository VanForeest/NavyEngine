#version 410 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexUV;


#define NR_MAX_LIGHTS 4

// Propiedades del material
uniform sampler2D texture_BaseColor1;
uniform sampler2D texture_Normal1;
uniform sampler2D texture_ORM1;

uniform bool hasBaseColorMap;
uniform bool hasNormalMap;
uniform bool hasORM;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

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

vec3 getNormalFromMap(){

    vec3 tangentNormal = texture(texture_Normal1, TexUV).xyz * 2.0 - 1.0;
    
    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexUV);
    vec2 st2 = dFdy(TexUV);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


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

    // Obtenemos los valores del material
    vec3 albedo = hasBaseColorMap ? pow(texture(texture_BaseColor1, TexUV).rgb, vec3(2.2)) : vec3(0.5, 0.5, 0.5);
    float ao = hasORM ? texture(texture_ORM1, TexUV).r : 1.0;
    float roughness = hasORM ? texture(texture_ORM1, TexUV).g : 0.2;
    float metallic = hasORM ? texture(texture_ORM1, TexUV).b : 0.0;
     
    // input lighting data
    vec3 N = hasNormalMap ? getNormalFromMap() : Normal;      
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);
        
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    Lo += ComputeDirecLight(dirlight, N, V, F0, WorldPos, albedo, roughness, metallic);

    for(int i = 0; i < 4; ++i){
        // calcula per-light radiance
        Lo += ComputePointLight(polight[i], N, V, F0, WorldPos, albedo, roughness, metallic);
    }   
    
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

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color , 1.0);
}