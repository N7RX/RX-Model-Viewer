#version 330 core


in  vec4 color;

// Reflection parameters
in vec3 Normal;
in vec3 Position;

out vec4 fColor;

uniform int useReflection;
uniform vec3 cameraPos;
uniform sampler2D reflectTex;

void main()
{ 
	// With reflection
	if( useReflection == 1 )
	{
		vec3 I = normalize(Position - cameraPos);
		vec3 R = reflect(I, normalize(Normal));
		fColor = color * texture2D( reflectTex, vec2( R.x, R.y ) );
	}
	else
	{
		fColor = color;
	}
} 

