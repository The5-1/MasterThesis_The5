//http://www.mbroecker.com/research_pbr.html

#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vNormal; 
layout(location = 2) in  vec3 vColor; 

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 col;

 out VertexData 
{
	vec3  color;
	vec3  normal;
	float radius;
} vertexOut;

void main() {
	vec3 lightdir = normalize(vec3(1.0,1.5,0.5));
	float NoL = abs(dot(vNormal,lightdir));
	vec3 c = vec3(0.33,0.33,1.0)*NoL + vec3(1.0)*pow(NoL,5.0)*0.25;

	gl_Position = vec4(vPosition, 1.0);

	vertexOut.color = c;
	vertexOut.color = vColor;
	
	vertexOut.normal = normalize(vNormal);
	vertexOut.radius = 0.06;
}

