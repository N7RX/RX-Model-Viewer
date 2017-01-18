#version 410 core


in vec4 color;
in vec2 texCoord;
in float Ks;

in vec3 Normal;
in vec3 Position;

out vec4 fColor;

uniform sampler2D texture;
uniform sampler2D specular;

uniform vec4 LightSpecular;

uniform int useShadow;

uniform int useReflection;
uniform vec3 cameraPos;
uniform sampler2D reflectTex;

void main() 
{ 
	vec4 colorLit;
	if(useShadow == 1)
	{
		vec4 specularProduct = Ks * LightSpecular * texture2D( specular, texCoord );
		colorLit = color + specularProduct;
	}
	else
		colorLit = color;
	
	if( useReflection == 1 )
	{
		vec3 I = normalize(Position - cameraPos);
		vec3 R = reflect(I, normalize(Normal));
		fColor = colorLit * texture2D( reflectTex, vec2( R.x, R.y ) ) * texture2D( texture, texCoord );
	}
	else
		fColor = colorLit * texture2D( texture, texCoord );
} 

