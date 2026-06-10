#version 410 core


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

//Uniforms para el oceano
uniform float time;
uniform float Amplitude;
uniform float wavelenght;
uniform float speed;

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

const float PI = 3.14159265359;
const float G = 9.80665;

void main(){
	
	//Codigo del movimiento del agua (Oceano)
	float Final_X = aPos.x;
	float Final_Y = 0;
	float Final_Z = aPos.z;

	//Parametros para una Onda Gerstner

	vec2 Direction[] = {vec2(1.0, 0.0), vec2(0.8, 0.3), vec2(0.6, 0.8), vec2(-0.3, 1.0), vec2(-0.8, 0.2)}; // D = (Direction.x, Direction.z)
	float Amplitud[] = {2.0, 1.5, 1.0, 0.7, 0.4}; // A
	float WaveLength[] = {50, 35, 25, 15, 10}; // l
	float K; //Numero de onda: 2.0 * PI / WaveLength
	float W; //Velocidad angular: sqrt(G * K)
	float theta; // 0 = K * (Direction.x * X + Direction.z * Z) - W * time
	float Q[] = {0.5, 0.4, 0.4, 0.3, 0.2}; //Stepness

	//Parametros para las normales
	vec3 TanX = vec3(1, 0, 0);
	vec3 TanZ = vec3(0, 0 ,1);
	float Dx, Dy, Dz;

	//Las ondas de Gerstner calculan el movimiento para todos los ejes
	//Dependen de una variable Q (Stepness)
	//'X = X + Q * Amplitud * Direction.x * cos(theta)
	//'Y = Amplitud * sin(theta)
	//'Z = Z + Q * Amplitud * Direction.z * cos(theta)

	//Se calcula la altura final teniendo en cuenta todas las ondas


	for(int i = 0; i < 4; i++){
		
		//Calcular posiciones
		K = 2.0 * PI / WaveLength[i];
		W = sqrt(G * K);

		theta = K * dot(Direction[i], aPos.xz) - W * time;
		float c = cos(theta);
		float s = sin(theta);

		Final_X += Q[i] * Amplitud[i] * Direction[i].x * c;
		Final_Y += Q[i] * Amplitud[i] * s;
		Final_Z += Q[i] * Amplitud[i] * Direction[i].y * c;

		float commonDx = -Q[i] * Amplitud[i] * K * Direction[i].x * s;
		float commonDy = Amplitud[i] * K * c;
		float commonDz = -Q[i] * Amplitud[i] * K * Direction[i].y * s;

		//Para normal X
		Dx = commonDx * Direction[i].x;
		Dy = commonDy * Direction[i].x;
		Dz = commonDx * Direction[i].y;

		TanX += vec3(Dx, Dy, Dz);

		//Para normal Z
		Dx = commonDz * Direction[i].x;
		Dy = commonDy * Direction[i].y;
		Dz = commonDz * Direction[i].y;

		TanZ += vec3(Dx, Dy, Dz);

	}

	
	vec3 FinalNormal = normalize(cross(TanZ, TanX));
	vec3 OceanWorldPos = vec3(Final_X, Final_Y, Final_Z);

	//Transformaciones finales al plano

	WorldPos = vec3(model * vec4(OceanWorldPos, 1.0));
	Normal = NormalMatrix * FinalNormal;
	TexUV = aTexUV;

	gl_Position = projection * view * vec4(WorldPos, 1.0);
}