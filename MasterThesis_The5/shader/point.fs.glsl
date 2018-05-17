/* GUIDES:
How to use glPoints: https://stackoverflow.com/questions/27098315/render-large-circular-points-in-modern-opengl
*/
#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;

uniform float nearPlane;
uniform float farPlane;
uniform bool depthToPosTexture;

in vec4 viewNormal;
in vec4 viewPosition;
in vec3 color;
in vec4 positionFBO;

in vec3 lightVecV;

//Render type (only 1 may be active!!)
//#define SIMPLE_POINT 0
#define AFFINE_PROJECTED 0
//#define POTREE 0


//Lighting
//#define PHONG 0

const float NEAR = 0.8;
const float FAR = 1.1;

float linearize(float depth)
{
	return (depth-NEAR)/(FAR-NEAR);
}

void main(){ 
	/* ******************************************************************
	Fill FBO
	****************************************************************** */
	outNormal = vec4(0.5 * viewNormal.xyz + vec3(0.5), 1.0);
	outPos = vec4(positionFBO.xyz/positionFBO.w, 1.0);
	//outPos = vec4(viewPosition.xyz/viewPosition.w, 1.0);
	

	/* ******************************************************************
		Paint simple Point
	****************************************************************** */
	#ifdef SIMPLE_POINT
		vec2 circCoord0 = gl_PointCoord;
		float alpha = length(circCoord0*2.0-1.0);

		float newDepth = gl_FragCoord.z + (pow(alpha, 1.0)) * gl_FragCoord.w ;
		vec3 depth = vec3(linearize(newDepth));

		//outColor = vec4(depth,1.0);
		outColor = vec4(color, 1.0);

		if(alpha >= 1.0)
		{
			discard;
		}
	
		//gl_FragDepth = newDepth; 
		if(depthToPosTexture){
			outPos = vec4(vec3( newDepth), 1.0);
		}
		
	#endif

	/* ******************************************************************
		Affinely Projected Point Sprites (Page 278)
	****************************************************************** */
	#ifdef AFFINE_PROJECTED
		vec2 circCoord = 2.0 * gl_PointCoord - vec2(1.0, 1.0); //Maps to [-1, 1]

		//IMPORTANT: Changed one of them to positive -> seems to fix the error that they are displaced (doesnt matter which is positive for teapot/sphere)
		float delta_z = (+ ((viewNormal.x ) / (viewNormal.z )) * circCoord.x - ((viewNormal.y ) / (viewNormal.z )) * circCoord.y);

		float maxRadius = 1.0;
		float currentRadius = length( vec3(circCoord.x, circCoord.y, delta_z) );
		float newDepth = gl_FragCoord.z + (pow(currentRadius, 1.0)) * gl_FragCoord.w ;
		vec3 depthColor = vec3(newDepth);

		if(currentRadius > maxRadius)
		{
			//outColor = abs(vec4(1.0, 0.0, 0.0, 1.0));
			discard;
		}
		else{
			//outColor = vec4(depthColor, 1.0);
			outColor = vec4(color, 1.0);

			#ifdef PHONG
			outColor = vec4(color, 1.0) * vec4(vec3(pow(max(0.0,dot(viewNormal.xyz, lightVecV)),4.0)),1.0);
			#endif
		}

		

		//Update depth
		//gl_FragDepth = newDepth; 
		if(depthToPosTexture){
			outPos = vec4(vec3( newDepth), 1.0);
			//outPos = vec4(vec3( gl_FragCoord.z + (pow(currentRadius, 2.0)) ), 1.0);
		}
	#endif

	/* ******************************************************************
	Potree example
	****************************************************************** */
	#ifdef POTREE
		float a = pow(2.0*(gl_PointCoord.x - 0.5), 2.0);           
		float b = pow(2.0*(gl_PointCoord.y - 0.5), 2.0);           
		float c = 1.0 - (a + b);                                   
																
		if(c < 0.0){                                               
			discard;                                               
		}
		else if(c < 0.2){
			outColor = abs(vec4(0.0, 0.0, 0.0, 1.0));
		}                                                       
		else{														
			outColor = vec4(1.0, 0.0, 0.0, 1.0);  
		} 
	#endif


	/* ******************************************************************
	Recalc depth
	****************************************************************** */
	//gl_FragDepth = (1.0 / ( gl_FragCoord.z + delta_z )) * (farPlane * nearPlane) / (farPlane - nearPlane) + (farPlane) / (farPlane - nearPlane);

	/* ******************************************************************
		Why does this half the circles?
	****************************************************************** */
	//gl_FragDepth = 1.0 / ( gl_FragCoord.z, delta_z ) * (farPlane * nearPlane) / (farPlane - nearPlane) + (farPlane) / (farPlane - nearPlane);
	
}
