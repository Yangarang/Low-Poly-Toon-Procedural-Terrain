#version 330 core

in vec3 position;
in vec3 texturecoord;
out vec3 texcoord;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 modelMatrix;
uniform vec3 grassColor;

out vec3 color;
out vec3 pos;

void main()
{
    texcoord = texturecoord;
    color = grassColor;
    pos = position;
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}  