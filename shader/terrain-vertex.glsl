#version 150 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform float time;

in vec3 position;
in vec3 normal;
in vec3 texturecoord;
in vec3 biomecoord;

out vec3 n;
out vec3 color;
out vec3 pos;
out vec3 texcoord;
out vec3 biocoord;

void main()
{
    color = triangleColor;
    pos = vec3(modelMatrix * vec4(position, 1.0));
    texcoord = texturecoord;
    biocoord = biomecoord;
    n = normal;
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
