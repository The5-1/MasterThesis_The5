#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vNormal; 
layout(location = 2) in  vec3 vColor; 

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 col;

out vec3 color;

void main() {
	color = vColor;
		
	vec4 posV = viewMatrix * modelMatrix * vec4(vPosition, 1.0);
	
	//gl_PointSize = 15.0f;//*(1.0-posV.z);
	
	gl_Position = projMatrix * posV;
	
	gl_PointSize = 50.0f*(1.0-gl_Position.z/gl_Position.w);
}

