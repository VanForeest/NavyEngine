#version 410 core

in vec3 WorldPos;

out vec4 FragColor;

//Generalmente las texturas para skybox vienen con dimensiones de rectangulo
//La convertimos a "cubemap" localmente por mera conveniencia para futuros pasos
//y porque es mas facil encontrarlas en formato HDRi, que weba (¬_¬)
uniform sampler2D RectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v){
	
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;

	return uv;
}

void main(){

	vec2 uv = SampleSphericalMap(normalize(WorldPos));
	vec3 color = texture(RectangularMap, uv).rgb;

	FragColor = vec4(color, 1.0);
}