#version 410 core

out vec4 FragColor;

void main()
{             
    // Forzamos la salida de un color para que los drivers estrictos no descarten el fragmento
    FragColor = vec4(1.0);
}
