/* GUIDES:
How to use glPoints: https://stackoverflow.com/questions/27098315/render-large-circular-points-in-modern-opengl
*/

#version 330
layout(location = 0)  out vec4 out0; // color 

in vec3 color;

void main(){ 
	
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
}
