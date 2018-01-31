#version 330 core


in vec4 vPosition;
in vec4 vTexCoord;

out vec2 TexCoords;

uniform mat4 Projection;
uniform mat4 ModelView;

void main()
{
	gl_Position = Projection * ModelView * vPosition;
    TexCoords = vec2( vTexCoord.x, vTexCoord.y );
}
