#version 430 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D Texture;

void main()
{
    vec4 NewTexture = texture(Texture, TexCoords);
    
    //Posiblemente la textura que estamos probando sea una RGBA32F, asi que ajustamos sus valores al intervalo [0 - 1]
    vec4 visualColor = NewTexture.rgba * 0.5 + 0.5;
    
    //FragColor = visualColor;
    //FragColor = vec4(NewTexture.rg, 0.0, 0.0);

    //FragColor = NewTexture; //Probar texturas rgba32f

    //FragColor = vec4(vec3(NewTexture.r), 1.0); //Probar DnTextures

    vec3 NormalMap = NewTexture.rgb * 0.5 + 0.5;
    FragColor = vec4(NormalMap, 1.0); //Probar Normal map
}