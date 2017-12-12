/* GUIDES:
How to use glPoints: https://stackoverflow.com/questions/27098315/render-large-circular-points-in-modern-opengl
*/

#version 330
layout(location = 0)  out vec4 out0; // color 

uniform float nearPlane;
uniform float farPlane;

in vec4 viewNormal;
in vec3 color;

//#define SIMPLE_POINT 0
#define AFFINE_PROJECTED 0
//#define POTREE 0

void main(){ 
	/* ******************************************************************
		Paint simple Point
	****************************************************************** */
	#ifdef SIMPLE_POINT
		vec2 circCoord0 = gl_PointCoord;
		float alpha = min(1.0,(1.0-length(circCoord0*2.0-1.0))*4.0);
		if(alpha <= 0.0)
		{
			discard;
		}
		
		//vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
		
		//if (dot(circCoord, circCoord) > 1.0) {
		//	discard;
		//}
		
		
		//out0 = abs(vec4(vec3(circCoord.s, circCoord.t, 0.0)*alpha, 1.0));
		//out0 = vec4(1.0)*alpha;
		out0 = vec4(color,1.0);

		
		//out0 = abs(vec4(1.0, 0.0, 0.0, 1.0));
		//out0 = vec4(alpha, 1);
	#endif

	/* ******************************************************************
		Affinely Projected Point Sprites (Page 278)
	****************************************************************** */
	#ifdef AFFINE_PROJECTED
		//vec2 circCoord = 0.5 * gl_PointCoord - vec2(0.5, 0.5); //FALSE
		vec2 circCoord = 2.0 * gl_PointCoord - vec2(1.0, 1.0); //Maps to [-1, 1]
		//vec2 circCoord = gl_PointCoord;

		//float delta_z = - ((viewNormal.x / viewNormal.w) / (viewNormal.z / viewNormal.w)) * circCoord.x - ((viewNormal.y / viewNormal.w) / (viewNormal.z / viewNormal.w)) * circCoord.y;
		//float delta_z = (- ((viewNormal.x ) / (viewNormal.z )) * circCoord.x - ((viewNormal.y ) / (viewNormal.z )) * circCoord.y);
		//vec3 viewN = normalize( vec3(viewNormal) );
		//float delta_z = - ((viewN.x ) / (viewN.z )) * circCoord.x - ((viewN.y ) / (viewN.z )) * circCoord.y;

		//IMPORTANT: Changed one of them to positive -> seems to fix the error that they are displaced (doesnt matter which is positive for teapot/sphere)
		float delta_z = (+ ((viewNormal.x ) / (viewNormal.z )) * circCoord.x - ((viewNormal.y ) / (viewNormal.z )) * circCoord.y);

		float radius = 0.8;
		if(length( vec3(circCoord.x, circCoord.y, delta_z) ) > radius)
		{
			discard;
		}
		else if(length( vec3(circCoord.x, circCoord.y, delta_z) ) >= 0.9 * radius)
		{
			out0 = abs(vec4(0.0, 0.0, 0.0, 1.0));
		}
		else{
			out0 = abs(vec4(1.0, 0.0, 0.0, 1.0));
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
			out0 = abs(vec4(0.0, 0.0, 0.0, 1.0));
		}                                                       
		else{														
			out0 = vec4(1.0, 0.0, 0.0, 1.0);  
		}

		gl_FragDepth = gl_FragCoord.z + 0.01*(1.0-pow(c, 2.0)) * gl_FragCoord.w ;   
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
