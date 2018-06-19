#version 330
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPos;
layout(location = 3) out vec4 outDepth;

in vec2 tc;

uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texPosition;
uniform sampler2D texDepth;

uniform vec3 lightVecV;

//Lighting
//#define PHONG 0
#define BLENDING 0
#define DEPTH_DIFFERENCE 0

#define GAMMA_CORRECTION 0

vec3 LinearToSRGB(vec3 linear)
{
#if GAMMA_CORRECTION > 0
	return pow(linear,vec3(1.0/2.2));
#else
	return linear;
#endif
}

vec3 srgbToLinear(vec3 linear)
{
#if GAMMA_CORRECTION > 0
	return pow(linear,vec3(2.2));
#else
	return linear;
#endif
}


void main(){

    vec4 dif = texture2D(texColor, tc); //Fuzzy blended color goes in!
	
	//pointless, contain only depth again and again...
	vec4 nor = texture2D(texNormal, tc);
	vec4 pos = texture2D(texPosition, tc);
	vec4 depth = texture2D(texDepth, tc);
	
	//Lighting
	#ifdef PHONG
		//float lightint = 1.0 / (length(pos - lightVecV)*length(pos - lightVecV));
		float lightint = 1.0;
		vec3 lightDir = normalize(lightVecV - pos.rgb);
		vec3 lightout = max(dot(nor.rgb, lightDir), 0.0) * dif.rgb * lightint;
		vec3 lightColor = vec3(1.0, 1.0, 1.0);
		outColor = vec4(lightColor * lightout, 1.0);
	#endif

	#ifdef BLENDING
		outColor = dif / (dif.w); //get rid of the initial alpha of the clear color, if clearcolor is 0.0 we should be good
		outColor.rgb = LinearToSRGB(outColor.rgb);

		//outColor.rgb = LinearToSRGB(outColor.rgb * 10.0); //nanosuit is too dark, add *40.0 inside brackets! Make sure to turn on Gamma correction in this and the previous color shader!
		//outColor.rgb = outColor.rgb * 3.5;
		//outColor.rgb = dif.rgb * 3.5;
//
//
		//outNormal = nor / nor.w;
		//outPos = pos / pos.w;
		//gl_FragDepth = texture2D(texDepth, tc).r;

		/*
		outColor = vec4(texture2D(texColor, tc).rgb, 1.0);
		outNormal = vec4(texture2D(texNormal, tc).rgb, 1.0);
		outPos = vec4(texture2D(texPosition, tc).rgb, 1.0);
		outDepth = vec4(vec3(texture2D(texDepth, tc).r), 1.0);
		gl_FragDepth = outDepth.r;
		*/
	#endif
}
