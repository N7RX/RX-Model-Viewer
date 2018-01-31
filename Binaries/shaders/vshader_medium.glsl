#version 330 core


in vec4 vPosition;
in vec4 vNormal;
in vec4 vTexCoord;

out vec4 color;
out vec2 texCoord;

out vec3 Normal;
out vec3 Position;

uniform mat4 ModelView;
uniform mat4 Projection;

uniform vec4 MaterialAmbient, MaterialDiffuse, MaterialSpecular;
uniform float MaterialShininess;

uniform vec4 LightAmbient, LightDiffuse, LightSpecular;
uniform vec4 LightPosition;

uniform int useShadow;

void main()
{
	vec4 AmbientProduct = MaterialAmbient * LightAmbient;
	vec4 DiffuseProduct = MaterialDiffuse * LightDiffuse;
	vec4 SpecularProduct = MaterialSpecular * LightSpecular;
	
	if (useShadow == 1)
	{
		// Transform vertex  position into eye coordinates
		vec3 pos = (ModelView * vPosition).xyz;
		
		vec3 E = normalize( -pos );
		vec3 N = normalize( ModelView*vNormal ).xyz;
		
		vec3 L = normalize( (ModelView*LightPosition).xyz - pos );
		vec3 H = normalize( L + E );

		// Compute terms in the illumination equation
		vec4 ambient = AmbientProduct;

		float Kd = max( dot(L, N), 0.0 );
		vec4 diffuse = Kd * DiffuseProduct;

		float Ks = pow( max(dot(N, H), 0.0), MaterialShininess );
		vec4 specular = Ks * SpecularProduct;
		
		if( dot(L, N) < 0.0 ) {
		specular = vec4(0.0, 0.0, 0.0, 1.0);
		} 

		// Add up the light component
		color = ambient + diffuse + specular;
	}
	else
		color = DiffuseProduct + AmbientProduct;
    color.a = 1.0;
	
	gl_Position = Projection * ModelView * vPosition;
	
	Normal = mat3(transpose(inverse(ModelView))) * vec3( vNormal.x, vNormal.y, vNormal.z );
    Position = vec3(ModelView * vPosition);
	
	texCoord = vec2( vTexCoord.x, vTexCoord.y );
}
