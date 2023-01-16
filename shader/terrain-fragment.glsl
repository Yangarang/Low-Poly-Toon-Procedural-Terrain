#version 150 core

out vec4 outColor;
in vec3 n;
in vec3 color;
in vec3 pos;
in vec3 texcoord;
in vec3 biocoord;

uniform vec3 triangleColor;
uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform float time;

uniform vec3 WaterColor;
uniform vec3 Sand1Color;
uniform vec3 Sand2Color;
uniform vec3 Grass1Color;
uniform vec3 Grass2Color;
uniform vec3 Rock1Color;
uniform vec3 Rock2Color;
uniform vec3 SnowColor;

uniform sampler2D tex;
uniform samplerCube skybox;

float linear_interpolate(float a0, float a1, float w) {
    return (a1 - a0) * w + a0;
}

void main()
{
    vec3 col = color;
    vec3 normal = normalize(n);
    vec3 lightDir = normalize(lightPos - pos);
    
    // setup color shade by intensity
	float intensity = dot(lightDir,normal);  
    vec3 final_GrassColor, final_RockColor, final_SandColor;
	if (intensity > 0.5) {
        final_GrassColor = Grass1Color;
    }
    else{
		final_GrassColor = Grass2Color;
    }
	if (intensity > 0.5) {
		final_RockColor = Rock1Color;
    }
    else{
        final_RockColor = Rock2Color;
    }
	if (intensity > 0.5) {
		final_SandColor = Sand1Color;
    }
    else{
        final_SandColor = Sand2Color;
    }
    
    // color by height and divide by Perlin and waves
    float slope = 1.0f - normal.y;
    float wave =   0.1 * (cos(0.5*(pos.x+pos.y)+10.0) + sin(0.5*(pos.x+pos.y)+10.0));
    if (pos.y < -0.3 + wave) {
        col = WaterColor;
    }
    else if (pos.y < 0.3 + wave) {
        col = final_SandColor;      
    }
    else if (pos.y < 4.0 + biocoord.x) {
        col = final_GrassColor;
    }
    else if (pos.y < 10.0 + biocoord.x) {
        col = final_RockColor;
    }
    else {
        // color snow tops by slope
        if (slope > 0.4) {
            col = final_RockColor;        
        }
        else {
            col = SnowColor;
        }
    }      
    
    
    // calculate depth
    float near = 0.1; 
    float far  = 6.0; 
    float depth = (2.0 * near * far) / (far + near - (gl_FragCoord.z * 2.0 - 1.0) * (far - near));
    col = (col*vec3(depth))/4.0;
    
    
    outColor = vec4(col,1.0);

    // to show UV coordinates for debugging
    // outColor = vec4(texcoord.xy,0.0, 1.0);
    // to show just the texture for debugging    
    // outColor = vec4(vec3(texture(tex,texcoord.xy)), 0.5);

}
