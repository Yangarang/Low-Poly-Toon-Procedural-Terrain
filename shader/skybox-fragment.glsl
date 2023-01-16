#version 330 core
out vec4 outColor;

in vec3 TexCoords;
in vec3 color;

uniform samplerCube skybox;

void main()
{    
    outColor = texture(skybox, TexCoords);
    //outColor = vec4(color, 1.0);
}