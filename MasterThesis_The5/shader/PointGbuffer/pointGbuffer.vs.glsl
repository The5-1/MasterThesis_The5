#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vNormal; 
layout(location = 2) in  vec3 vColor; 
layout(location = 3) in  float vRadius; 

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 col;
uniform vec3 viewPoint;

uniform float glPointSize;

out vec4 viewNormal;
out vec4 viewPosition;
out vec3 color;
out vec4 positionFBO;

#define BACKFACE_CULLING 0

void main() {
	//Normal
	mat4 normalMatrix = transpose(inverse( viewMatrix * modelMatrix));
	viewNormal =  normalMatrix * vec4(vNormal, 0.0); //homogenous coordinate = 0 for vectors

	//Color
	color = vColor;

	//Position	
	viewPosition = viewMatrix * modelMatrix * vec4(vPosition, 1.0);
	positionFBO = modelMatrix * vec4(vPosition, 1.0);
	gl_Position = projMatrix * viewPosition;
	
	//Size
	gl_PointSize = glPointSize * (1.0 - gl_Position.z / gl_Position.w);
	//gl_PointSize = glPointSize;

	

	/* ******************************************************************
	Backface-Culling (gl_points dont have an implemented backface-cull)
	****************************************************************** */
	#ifdef BACKFACE_CULLING
	vec3 viewDirection = viewPoint - vPosition;
	if(dot(viewDirection, vNormal) < 0 ){
		gl_Position.w = 0.0;
	}
	#endif	
}

