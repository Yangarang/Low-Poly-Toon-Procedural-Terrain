#version 330 core
out vec4 outColor;

in vec3 texcoord;
in vec3 color;
in vec3 pos;

uniform sampler2D grass_tex;

void main()
{    
    vec3 col = vec3(texture(grass_tex,texcoord.xy));
    // get rid of transparency
    if ((col.x < 0.1f && col.y < 0.1f && col.z < 0.1f)  || pos.y > 0.5) {  
        discard;
    }
    col = color;
    // calculate depth
    float near = 0.1; 
    float far  = 1.0; 
    float depth = (2.0 * near * far) / (far + near - (gl_FragCoord.z * 2.0 - 1.0) * (far - near));
    col = (col*vec3(depth));

    outColor = vec4(col, 1.0);   
}