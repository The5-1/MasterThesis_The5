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

out vec4 viewNormal;
out vec3 color;
out vec4 positionFBO;

#define BACKFACE_CULLING 0

//Pointsize
//#define PS_BASIC 0
//#define PS_CONSTANT 0
//#define PS_LION 0

void main() {
	mat4 normalMatrix = transpose(inverse( viewMatrix * modelMatrix));
	viewNormal =  normalMatrix * vec4(vNormal, 1);

	//mat3 normalMatrix = inverse(transpose(mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz)));
	//viewNormal =  vec4(normalize(normalMatrix * vNormal), 1);

	color = vColor;
		
	vec4 posV = viewMatrix * modelMatrix * vec4(vPosition, 1.0);
	
	//gl_PointSize = 15.0f;//*(1.0-posV.z);
	positionFBO = modelMatrix * vec4(vPosition, 1.0);
	gl_Position = projMatrix * posV;
	
	//Constant point size
	//gl_PointSize = 500.0f * vRadius * (1.0-gl_Position.z/gl_Position.w);
	gl_PointSize = 80.0f*(1.0 - gl_Position.z/gl_Position.w);
	//View-Dependant point size
	//gl_PointSize = 50.0f;

	/*
	float uPointSizeMultiplicator = 10.0f;
	float trans = 0.5 + max(length(posV)-10.0, 0.0) / 30.0;
	vec4 p1 = projMatrix * posV;
	vec4 p2 = projMatrix * (posV + vec4(trans,trans,0.0,0.0));
	p1.xyz = p1.xyz / p1.w;
	p2.xyz = p2.xyz / p1.w;
	vec2 dist = p1.xy - p2.xy;
	float ps = length(dist) * 30.0;
	ps = max(3.0, ps);
	ps = ps * uPointSizeMultiplicator;
	gl_PointSize = ps;
	*/

	/* ******************************************************************
	Backface-Culling (gl_points dont have an implemented backface-cull)
	****************************************************************** */
	#ifdef BACKFACE_CULLING
	vec3 viewDirection = viewPoint - vPosition;
	if(dot(viewDirection, vNormal) < 0 ){
		gl_Position.w = 0.0;
	}
	#endif

	/* ******************************************************************
		Calculate tangent Vecotrs for the point
		-Use the Viewspace Normal (now the z-Coordinate represent ALWAYS the depth, its in the (-1,-1,-1) to (1,1,1) unit cube)
		-Now we can decide which is the best Tangent direction
	****************************************************************** */
	/*
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
	*/
}

