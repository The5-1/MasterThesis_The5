#version 330
layout(location = 0)  out vec4 out0; // color 

in FragmentData 
      {
        vec2 texcoord;
        vec3 color;
      } FragmentIn;



void main()
{
	
	// calculate whether this fragment is inside or outside the splat circle
	// tex coords from -1.0 to 1.0
	
	//Possible Outputs
	//**************** 1 **************** Circle:
	if ((pow(FragmentIn.texcoord.x, 2.0) + pow( FragmentIn.texcoord.y, 2.0) > 1.0)) discard;
	out0 = vec4(FragmentIn.color, 1.0);
	
	//**************** 2 **************** Circle with black border box:
	//out0 = vec4(FragmentIn.color, 1.0)*step(length(FragmentIn.texcoord),1.0); //just multiply color with 1 when legth < 1.0 and 0 otherwise
	
	//**************** 3 **************** Circle with black border box (Anti-Aliased):
	//out0 = vec4(FragmentIn.color, 1.0)*clamp((1.0-length(FragmentIn.texcoord))*10.0,0.0,1.0); //same as above but instead of a hard step() we trick arround with clamp to get smooth anti-aliased corners
}
