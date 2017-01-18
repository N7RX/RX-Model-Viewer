#version 410 core


in vec4 color;
in vec2 texCoord;

in vec3 Normal;
in vec3 Position;

out vec4 fColor;

uniform sampler2D texture;

uniform int useReflection;
uniform vec3 cameraPos;
uniform sampler2D reflectTex;

void main() 
{ 
	if( useReflection == 1 )
	{
		vec3 I = normalize(Position - cameraPos);
		vec3 R = reflect(I, normalize(Normal));
		fColor = color * texture2D( reflectTex, vec2( R.x, R.y ) ) * texture2D( texture, texCoord );
	}
	else
	{
		fColor = color * texture2D( texture, texCoord );
	}
} 

