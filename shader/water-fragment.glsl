#version 150 core

out vec4 outColor;
in vec3 n;
in vec3 color;
in vec3 pos;
in vec3 texcoord;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform float time;

uniform sampler2D water_tex;
uniform samplerCube skybox;

void main()
{
    vec3 col = color;
    vec3 normal = normalize(n);
    vec3 lightDir = normalize(lightPos - pos);
    
    // phong lighting with water texture
    col = 0.3*clamp( vec3(texture(water_tex,texcoord.xy)) * lightParams.x + 
        vec3(texture(water_tex,texcoord.xy)) * max(0.0, dot(normal, lightDir)) + 
        vec3(1.0) * pow(max(0.0, dot( normalize(camPos - pos), normalize( reflect(-lightDir, normal)))), lightParams.y),
        0.0, 1.0);  
        
    // animate water texture in circles and with distortion based on time
    col += 1.0*vec3(texture(water_tex,texcoord.xy + 
        vec2((sin(time * 0.5f) + 0.1f) / 10.0f, (cos(time * 0.5f) + 0.1f) / 10.0f) 
        + 0.1*vec2(cos(time*0.8+3.0*texcoord.x), sin(time*0.8+3.0*texcoord.y))));       

    outColor = vec4(col,0.5);
    // to show UV coordinates for debugging
    // outColor = vec4(texcoord.xy,0.0, 1.0);
    // to show just the texture for debugging    
    // outColor = vec4(vec3(texture(tex,texcoord.xy)), 0.5);

}
