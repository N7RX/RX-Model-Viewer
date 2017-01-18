#version 410 core


in vec4 vPosition;
out vec4 color;

uniform mat4 Projection;
uniform mat4 ModelView;

void main()
{
	gl_Position = Projection * ModelView * vPosition;
    color = vec4( 0.5, 0.5, 0.5, 1 );
}
