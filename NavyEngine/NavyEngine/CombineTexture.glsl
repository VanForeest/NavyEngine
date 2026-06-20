#version 430 core

layout (local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rg32f) uniform readonly image2D MoveX; // Desplazamiento Horizontal (X)
layout(binding = 1, rg32f) uniform readonly image2D MoveY; // Desplazamiento Vertical (Y)
layout(binding = 2, rg32f) uniform readonly image2D MoveZ; // Desplazamiento Horizontal (Z)

layout(binding = 3, rgba32f) uniform writeonly image2D MovementTexture;

uniform uint Resolution;

void main(){
	
	ivec2 texel_coord = ivec2(gl_GlobalInvocationID.xy);
    
    if (texel_coord.x >= Resolution || texel_coord.y >= Resolution) {
        return;
    }

    float X_r = imageLoad(MoveX, texel_coord).r;
    float Y_r = imageLoad(MoveY, texel_coord).r;
    float Z_r = imageLoad(MoveZ, texel_coord).r;

    imageStore(MovementTexture, texel_coord, vec4(X_r, Y_r, Z_r, 1.0));

}