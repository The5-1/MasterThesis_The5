#version 330
  
layout(location = 0) in  vec3 vPosition; 
layout(location = 1) in  vec3 vNormal; 
layout(location = 2) in  vec3 vColor; 
layout(location = 3) in  float vRadius; 

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec4 clearColor;

uniform vec3 viewPoint;

uniform int renderPass;

uniform vec3 cameraPos;
uniform float glPointSize;

out vec4 viewNormal;
out vec4 viewPosition;
out vec3 coordinateGridPos;
out vec3 color;
out vec4 positionFBO;

out float distToCamera;

#define BACKFACE_CULLING 0


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

void main() {
	//Normal
	mat4 normalMatrix = transpose(inverse( viewMatrix * modelMatrix));
	viewNormal =  normalMatrix * vec4(vNormal, 0.0); //homogenous coordinate = 0 for vectors

	vec4 cs_position = viewMatrix * modelMatrix * vec4(vPosition, 1.0);
    distToCamera = -cs_position.z / 10.0;

	//Color
	//vec3 lightdir = normalize(vec3(1.0,1.5,0.5));
	//vec3 lightdir = normalize(vec3(1.0,0.0,0.0));
	//vec3 lightdir = normalize(vec3(0.0,1.0,0.0));

	//color = vColor*abs(vNormal);

	//float NoL = abs(dot(vNormal,lightdir));
	//vec3 farbe = vec3(0.33,0.33,1.0);
	//color = farbe*farbe*NoL + vec3(1.0)*pow(NoL,5.0)*0.25;

	vec3 lightdir = normalize(vec3(1.0,2.0,-0.7));
	float NoL = max(0.0,dot(vNormal,lightdir));
	vec3 farbe = vColor.rgb;
	color = farbe*farbe*(0.5+0.5*NoL);

	color = srgbToLinear(color.rgb);




	//color = srgbToLinear(vColor) * 0.35;

	//Position	
	viewPosition = viewMatrix * modelMatrix * vec4(vPosition, 1.0);
	positionFBO = modelMatrix * vec4(vPosition, 1.0);
	gl_Position = projMatrix * viewPosition;

	//Size (Constant size, selfmade)
	gl_PointSize = glPointSize * (1.0 - gl_Position.z / gl_Position.w);

	//High-Quality Point-Based Rendering on Modern GPUs (Seite 3)
	//float distanceCam = length( - viewPosition.xyz ); //Does not need to be divided by w! w = 1 (Viewtrafo, is only rotation/Translation)
	//gl_PointSize = glPointSize * (1.0 / distanceCam) * (768.0 / 2.0);

	#ifdef BACKFACE_CULLING
	vec3 viewDirection = viewPoint - vPosition;
	if(dot(viewDirection, vNormal) < 0 ){
		gl_Position.w = 0.0;
	}
	#endif	
}

