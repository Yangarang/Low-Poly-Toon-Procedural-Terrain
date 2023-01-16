#version 330 core

in vec3 position;
out vec3 TexCoords;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 triangleColor;
out vec3 color;

void main()
{
    TexCoords = position;
    color = triangleColor;
    gl_Position = projMatrix * viewMatrix * vec4(position, 1.0);
}  