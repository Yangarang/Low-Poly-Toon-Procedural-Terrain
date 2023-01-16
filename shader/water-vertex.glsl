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

out vec3 n;
out vec3 color;
out vec3 pos;
out vec3 texcoord;

void main()
{
    color = triangleColor;

    // generate up and down motion
    vec3 wavePosition = position;
    wavePosition.y += 0.05 * (cos(0.5*time+3.0) + sin(0.5*time+3.0));

    // generate waves
    float angle = (time + wavePosition.z) * 1.0f;
    wavePosition.x += sin(angle)*0.1f;
    wavePosition.y += sin(angle)*0.1f;

    // update wave normals
    vec3 newNormal = normal;
    newNormal = normalize(vec3(-0.1f * 1.0f * cos(angle),0.0f,1.0f));
    n = mat3(transpose(inverse(modelMatrix))) * newNormal;

    pos = vec3(modelMatrix * vec4(wavePosition, 1.0));
    texcoord = texturecoord;
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(wavePosition, 1.0);
}
