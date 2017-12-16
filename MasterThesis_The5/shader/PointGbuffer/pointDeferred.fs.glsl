#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;

in vec2 tc;

uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texPosition;
uniform sampler2D texDepth;

uniform vec3 lightVecV;

void main(){
    vec3 dif = texture2D(texColor, tc).rgb;
	vec3 nor = texture2D(texNormal, tc).rgb;
	vec3 pos = texture2D(texPosition, tc).rgb;

    //float lightint = 1.0 / (length(pos - lightVecV)*length(pos - lightVecV));
	float lightint = 1.0;


    vec3 lightDir = normalize(lightVecV - pos);
    
    
    
    vec3 lightout = max(dot(nor, lightDir), 0.0) * dif * lightint;
	
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	outColor = vec4(lightColor * lightout, 1.0);
	
}
