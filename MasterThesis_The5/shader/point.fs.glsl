/* GUIDES:
How to use glPoints: https://stackoverflow.com/questions/27098315/render-large-circular-points-in-modern-opengl
*/

#version 330
layout(location = 0)  out vec4 out0; // color 

uniform float nearPlane;
uniform float farPlane;

in vec4 viewNormal;
in vec3 color;

void main(){ 
	/* ******************************************************************
		Paint simple Point
	****************************************************************** */
	/*
	vec2 circCoord = gl_PointCoord;
	float alpha = min(1.0,(1.0-length(circCoord*2.0-1.0))*4.0);
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
	*/

	/* ******************************************************************
		Affinely Projected Point Sprites (Page 278)
	****************************************************************** */
	//vec2 circCoord = 0.5 * gl_PointCoord - vec2(0.5, 0.5); //FALSE
	vec2 circCoord = 2.0 * gl_PointCoord - vec2(1.0);

	//float delta_z = - ((viewNormal.x / viewNormal.w) / (viewNormal.z / viewNormal.w)) * circCoord.x - ((viewNormal.y / viewNormal.w) / (viewNormal.z / viewNormal.w)) * circCoord.y;
	float delta_z = - ((viewNormal.x ) / (viewNormal.z )) * circCoord.x - ((viewNormal.y ) / (viewNormal.z )) * circCoord.y;
	//vec3 viewN = normalize( vec3(viewNormal) );
	//float delta_z = - ((viewN.x ) / (viewN.z )) * circCoord.x - ((viewN.y ) / (viewN.z )) * circCoord.y;

	float radius = 0.5;
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


	/* ******************************************************************
	Recalc depth
	****************************************************************** */
	//gl_FragDepth = (1.0 / ( gl_FragCoord.z + delta_z )) * (farPlane * nearPlane) / (farPlane - nearPlane) + (farPlane) / (farPlane - nearPlane);

	/* ******************************************************************
		Why does this half the circles?
	****************************************************************** */
	//gl_FragDepth = 1.0 / ( gl_FragCoord.z, delta_z ) * (farPlane * nearPlane) / (farPlane - nearPlane) + (farPlane) / (farPlane - nearPlane);
	
}
