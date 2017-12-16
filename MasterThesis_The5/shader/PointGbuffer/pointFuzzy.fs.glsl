/* GUIDES:
How to use glPoints: https://stackoverflow.com/questions/27098315/render-large-circular-points-in-modern-opengl
*/
#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;

uniform float depthEpsilonOffset;

in vec4 viewNormal;
in vec4 viewPosition;
in vec3 color;
in vec4 positionFBO;

//Render type (only 1 may be active!!)
//#define SIMPLE_POINT 0
#define AFFINE_PROJECTED 0
//#define POTREE 0

//Lighting
#define PHONG 0

void main(){ 
	/* ******************************************************************
	Fill FBO
	****************************************************************** */
	outNormal = vec4(0.5 * viewNormal.xyz + vec3(0.5), 1.0);
	outPos = vec4(positionFBO.xyz/positionFBO.w, 1.0);

	/* ******************************************************************
		Affinely Projected Point Sprites (Page 278)
	****************************************************************** */
	vec2 circCoord = 2.0 * gl_PointCoord - vec2(1.0, 1.0); //Maps to [-1, 1]

	//IMPORTANT: Changed one of them to positive -> seems to fix the error that they are displaced (doesnt matter which is positive for teapot/sphere)
	float delta_z = (+ ((viewNormal.x ) / (viewNormal.z )) * circCoord.x - ((viewNormal.y ) / (viewNormal.z )) * circCoord.y);

	float maxRadius = 1.0;
	float currentRadius = length( vec3(circCoord.x, circCoord.y, delta_z) );
	if(currentRadius > maxRadius)
	{
		//outColor = abs(vec4(1.0, 0.0, 0.0, 1.0));
		discard;
	}
	else{
		outColor = vec4(color, 1.0);
	}

	//Update depth
	gl_FragDepth = gl_FragCoord.z + (pow(currentRadius, 2.0)) * gl_FragCoord.w - depthEpsilonOffset; 	
}
