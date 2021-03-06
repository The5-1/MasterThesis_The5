/*
#version 330
  
 // we accept points and render quads
layout (points) in;
layout (triangle_strip, max_vertices=4) out;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

//uniform float splatSize = 1.0;

in VertexData 
{
	vec3  color;
	vec3  normal;
	float radius;
} VertexIn[];

out FragmentData 
{
	vec2 texcoord;
	vec3 color;
} VertexOut;

void main()
{
	float splatSize = 8.0;

	// discard outliers
	if (VertexIn[0].radius > 0.0)
	{

	  // Matrix setup
	  mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	  mat3 normalMatrix = inverse(transpose(mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz)));

	  vec4 pointCenter = gl_in[0].gl_Position;
	  vec3 pointNormal = normalize(normalMatrix * VertexIn[0].normal);

	  // create tangent space. Helper vectors dependent on major orientation of normal
	  vec3 u, v;
	  if (abs(pointNormal.y) > abs(pointNormal.x))
	  {
		v = cross(pointNormal, vec3(1.0, 0.0, 0.0));
		u = cross(pointNormal, v);
	  }
	  else
	  {
		v = cross(vec3(0.0, 1.0, 0.0), pointNormal);
		u = cross(pointNormal, v);
	  }

	  // Scale the splat
	  u *= VertexIn[0].radius;
	  v *= VertexIn[0].radius;
	  
	  // Manually alter size
	  u *= (splatSize / 2.0);
	  v *= (splatSize / 2.0);

	  // Calculate the four corner vertices of the quad
	  vec4 a = pointCenter + vec4(-u-v, 0.0);
	  vec4 b = pointCenter + vec4(-u+v, 0.0);
	  vec4 c = pointCenter + vec4(+u+v, 0.0);
	  vec4 d = pointCenter + vec4(+u-v, 0.0);

	  // transform the four points. Note the order of output
	  gl_Position = mvp * b;
	  VertexOut.texcoord = vec2(-1.0, 1.0);
	  VertexOut.color = VertexIn[0].color;
	  EmitVertex();

	  gl_Position = mvp * a;
	  VertexOut.texcoord = vec2(-1.0, -1.0);
	  EmitVertex();

	  gl_Position = mvp * c;
	  VertexOut.texcoord = vec2(1.0, 1.0);
	  EmitVertex();

	  gl_Position = mvp * d;
	  VertexOut.texcoord = vec2(1.0, -1.0);
	  EmitVertex();

	}

}
*/


#version 330
  
 // we accept points and render quads
layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in VertexData 
{
	vec3  color;
	vec3  normal;
	float radius;
} VertexIn[];

out FragmentData 
{
	vec2 texcoord;
	vec3 color;
} VertexOut;

void main()
{
	float splatSize = 2;

	// discard outliers
	if (VertexIn[0].radius > 0.0)
	{

		// Matrix setup
		mat4 mvp = projMatrix * viewMatrix * modelMatrix;
		mat3 normalMatrix = inverse(transpose(mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz)));

		vec4 pointCenter = gl_in[0].gl_Position;
		vec3 pointNormal = normalize(normalMatrix * VertexIn[0].normal);

		// create tangent space. Helper vectors dependent on major orientation of normal
		vec3 u, v;
		if (abs(pointNormal.y) > abs(pointNormal.x))
		{
		v = cross(pointNormal, vec3(1.0, 0.0, 0.0));
		u = cross(pointNormal, v);
		}
		else
		{
		v = cross(vec3(0.0, 1.0, 0.0), pointNormal);
		u = cross(pointNormal, v);
		}

		u = normalize(u);
		v = normalize(v);

		// Scale the splat
		u *= VertexIn[0].radius;
		v *= VertexIn[0].radius;

		// Manually alter size
		//u *= (splatSize / 2.0);
		//v *= (splatSize / 2.0);
		
		u *= (splatSize);
		v *= (splatSize);
		
		// Calculate the four corner vertices of the quad
		//Höhe = sqrt(Splatsize^2 + Splatsize^2) + Splatsize
		float height = 2.41421356237 * splatSize;
		//KatheteLength = a = 2 * height / sqrt(2)
		float katheteLength = 1.41421356237 * height;
		
		vec4 a = pointCenter + vec4(+ u - v - katheteLength * u, 0.0);
		vec4 b = pointCenter + vec4(+ u - v, 0.0);
		vec4 c = pointCenter + vec4(+ u - v + katheteLength * v, 0.0);

		// transform the four points. Note the order of output
		gl_Position = mvp * b;
		VertexOut.texcoord = vec2(-1.0, 1.0);
		VertexOut.color = VertexIn[0].color;
		EmitVertex();

		gl_Position = mvp * a;
		VertexOut.texcoord = vec2(-1.0, -1.0);
		EmitVertex();

		gl_Position = mvp * c;
		VertexOut.texcoord = vec2(1.0, 1.0);
		EmitVertex();
	}

}

/*
#version 330
  
 // we accept points and render quads
layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in VertexData 
{
	vec3  color;
	vec3  normal;
	float radius;
} VertexIn[];

out FragmentData 
{
	vec2 texcoord;
	vec3 color;
} VertexOut;

void main()
{
	float splatSize = 1.5;

	// discard outliers
	if (VertexIn[0].radius > 0.0)
	{

		// Matrix setup
		mat4 mvp = projMatrix * viewMatrix * modelMatrix;
		mat3 normalMatrix = inverse(transpose(mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz)));

		vec4 pointCenter = gl_in[0].gl_Position;
		vec3 pointNormal = normalize(normalMatrix * VertexIn[0].normal);

		// create tangent space. Helper vectors dependent on major orientation of normal
		vec3 u, v;
		if (abs(pointNormal.y) > abs(pointNormal.x))
		{
		v = cross(pointNormal, vec3(1.0, 0.0, 0.0));
		u = cross(pointNormal, v);
		}
		else
		{
		v = cross(vec3(0.0, 1.0, 0.0), pointNormal);
		u = cross(pointNormal, v);
		}

		u = normalize(u);
		v = normalize(v);

		// Scale the splat
		u *= VertexIn[0].radius;
		v *= VertexIn[0].radius;

		// Manually alter size
		//u *= (splatSize / 2.0);
		//v *= (splatSize / 2.0);
		
		u *= (splatSize);
		v *= (splatSize);
		
		// Calculate the four corner vertices of the quad
		float height = 3.0 * splatSize;
		float sideLength = 3.4641016 * splatSize;
		
		vec4 a = pointCenter + vec4(-0.5 *sideLength *u - 0.5 * height *v , 0.0);
		vec4 b = pointCenter + vec4(0.5 *sideLength *u, 0.0);
		vec4 c = pointCenter + vec4(0.5 *height *v, 0.0);

		// transform the four points. Note the order of output
		gl_Position = mvp * b;
		VertexOut.texcoord = vec2(-1.0, 1.0);
		VertexOut.color = VertexIn[0].color;
		EmitVertex();

		gl_Position = mvp * a;
		VertexOut.texcoord = vec2(-1.0, -1.0);
		EmitVertex();

		gl_Position = mvp * c;
		VertexOut.texcoord = vec2(1.0, 1.0);
		EmitVertex();
	}

}
*/