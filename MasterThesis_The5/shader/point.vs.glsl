#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vNormal; 
layout(location = 2) in  vec3 vColor; 

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 col;

uniform vec3 viewPoint;

out vec4 viewNormal;
out vec3 color;

void main() {
	mat4 normalMatrix = transpose(inverse( viewMatrix * modelMatrix));
	viewNormal =  normalMatrix * vec4(vNormal, 1);

	//mat3 normalMatrix = inverse(transpose(mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz)));
	//viewNormal =  vec4(normalize(normalMatrix * vNormal), 1);

	color = vColor;
		
	vec4 posV = viewMatrix * modelMatrix * vec4(vPosition, 1.0);
	
	//gl_PointSize = 15.0f;//*(1.0-posV.z);
	
	gl_Position = projMatrix * posV;
	
	//Constant point size
	gl_PointSize = 80.0f*(1.0-gl_Position.z/gl_Position.w);
	//View-Dependant point size
	//gl_PointSize = 50.0f;

	/* ******************************************************************
	Backface-Culling (gl_points dont have an implemented backface-cull)
	****************************************************************** */
	//vec3 viewDirection = viewPoint - vPosition;
	//if(dot(viewDirection, vNormal) < 0 ){
	//	gl_Position.w = 0.0;
	//}

	/* ******************************************************************
		Calculate tangent Vecotrs for the point
		-Use the Viewspace Normal (now the z-Coordinate represent ALWAYS the depth, its in the (-1,-1,-1) to (1,1,1) unit cube)
		-Now we can decide which is the best Tangent direction
	****************************************************************** */
	vec3 u, v;
	  if (abs(viewNormal.y) > abs(viewNormal.x))
	  {
		v = cross(viewNormal.xyz, vec3(1.0, 0.0, 0.0));
		u = cross(viewNormal.xyz, v);
	  }
	  else
	  {
		v = cross(vec3(0.0, 1.0, 0.0), viewNormal.xyz);
		u = cross(viewNormal.xyz, v);
	  }
}

