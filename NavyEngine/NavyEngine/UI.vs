#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform vec2 offset;
uniform vec2 scale;

void main()
{
    vec2 position = aPos.xy * scale + offset;
    gl_Position = vec4(position, 0.0, 1.0);
    TexCoords = aTexCoords;
}
