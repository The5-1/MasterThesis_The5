#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;
layout(location = 3) out vec4 outDepth;

in vec2 vTexCoord;

//declare uniforms
uniform sampler2D texColor;
//uniform sampler2D texNormal;
//uniform sampler2D texPosition;
uniform sampler2D texDepth;

uniform float resolutionWIDTH;
uniform float resolutionHEIGHT;
uniform float radius;
uniform vec2 dir;

void main() {
    
	//this will be our RGBA sum
	vec4 sum = vec4(0.0);
	
	//our original texcoord for this fragment
	vec2 tc = vTexCoord;
	
	//the amount to blur, i.e. how far off center to sample from 
	//1.0 -> blur by one pixel
	//2.0 -> blur by two pixels, etc.
	float blurWidth = radius / resolutionWIDTH; 
    float blurHeight = radius / resolutionHEIGHT; 

	//the direction of our blur
	//(1.0, 0.0) -> x-axis blur
	//(0.0, 1.0) -> y-axis blur
	float hstep = dir.x;
	float vstep = dir.y;
    
	//apply blurring, using a 9-tap filter with predefined gaussian weights

	sum += texture2D(texColor, vec2(tc.x - 4.0*blurWidth*hstep, tc.y - 4.0*blurHeight*vstep)) * 0.0162162162;
	sum += texture2D(texColor, vec2(tc.x - 3.0*blurWidth*hstep, tc.y - 3.0*blurHeight*vstep)) * 0.0540540541;
	sum += texture2D(texColor, vec2(tc.x - 2.0*blurWidth*hstep, tc.y - 2.0*blurHeight*vstep)) * 0.1216216216;
	sum += texture2D(texColor, vec2(tc.x - 1.0*blurWidth*hstep, tc.y - 1.0*blurHeight*vstep)) * 0.1945945946;
	
	sum += texture2D(texColor, vec2(tc.x, tc.y)) * 0.2270270270;
	
	sum += texture2D(texColor, vec2(tc.x + 1.0*blurWidth*hstep, tc.y + 1.0*blurHeight*vstep)) * 0.1945945946;
	sum += texture2D(texColor, vec2(tc.x + 2.0*blurWidth*hstep, tc.y + 2.0*blurHeight*vstep)) * 0.1216216216;
	sum += texture2D(texColor, vec2(tc.x + 3.0*blurWidth*hstep, tc.y + 3.0*blurHeight*vstep)) * 0.0540540541;
	sum += texture2D(texColor, vec2(tc.x + 4.0*blurWidth*hstep, tc.y + 4.0*blurHeight*vstep)) * 0.0162162162;

	//discard alpha for our simple demo, multiply by vertex color and return
	outColor = vec4(sum.rgb, 1.0);
    //outNormal = vec4(texture2D(texNormal, tc).rgb, 1.0);
    //outPos = vec4(texture2D(texPosition, tc).rgb, 1.0);
    outDepth = vec4(vec3(texture2D(texDepth, tc).r), 1.0);
}