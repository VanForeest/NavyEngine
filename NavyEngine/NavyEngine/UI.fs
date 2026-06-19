#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float alpha;
uniform vec2 uvMin;
uniform vec2 uvMax;

void main()
{
    vec2 uv = mix(uvMin, uvMax, TexCoords);
    vec4 color = texture(screenTexture, uv);
    FragColor = vec4(color.rgb, color.a * alpha);
}
