#version 150 core

out vec4 outColor;
in vec3 n;
in vec3 color;
in vec3 pos;
in vec3 texcoord;

uniform vec3 lightPos;
uniform vec3 lightParams;
uniform vec3 camPos;
uniform vec3 Leaf1Color;
uniform vec3 Leaf2Color;
uniform vec3 Trunk1Color;
uniform vec3 Trunk2Color;
uniform float modelHeight;

uniform sampler2D tex;

void main()
{
    vec3 col = color;
    vec3 normal = normalize(n);
    vec3 lightDir = normalize(lightPos - pos);
        
    // setup color shade by intensity and tree trunk vs leaves
	float intensity = dot(lightDir,normal);  
    vec3 final_LeafColor, final_TrunkColor;
    if (pos.y > modelHeight-0.03) {
	    if (intensity > 0.5) {
            final_LeafColor = Leaf1Color;
        }
        else{
		    final_LeafColor = Leaf2Color;
        }  
        col = final_LeafColor;  
    }
    else {
	    if (intensity > 0.5) {
            final_TrunkColor = Trunk1Color;
        }
        else{
		    final_TrunkColor = Trunk2Color;
        }    
        col = final_TrunkColor;  
    }

    // calculate depth
    float near = 0.1; 
    float far  = 1.0; 
    float depth = (2.0 * near * far) / (far + near - (gl_FragCoord.z * 2.0 - 1.0) * (far - near));
    col = (col*vec3(depth));


    outColor = vec4(col, 1.0);
    // to show UV coordinates for debugging
    // outColor = vec4(texcoord.xy,0.0, 1.0);

}
