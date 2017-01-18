#pragma once

#include <angel\Angel.h>
#include <vector>
#include <string>

#define DRAW_FACE "GL_TRIANGLES"
#define DRAW_LINE "GL_LINE_LOOP"

#define DIFFUSE "diffuse"
#define SPECULAR "specular"

using namespace std;

typedef vec3 color3;


// Vertex unit
struct Vertex
{
	vec4 position;
	vec4 normal;
	vec4 tex_coords;
};

// Texture unit
struct Texture
{
	GLuint id;
	string type;
};


class Mesh
{
public:
	// Model data
	vector<Vertex> vertices;
	vector<GLuint> indices;

	// Properties
	unsigned int face_number;
	unsigned int vertex_number;

	// Transform info
	vec3 model_position;
	vec3 model_rotation;
	vec3 model_scale;

	// Texture info
	vec2 uv_scale;
	vec2 uv_shift;
	GLfloat uv_rotation;

	// Material info
	color3 material_color;
	GLfloat material_shininess;
	bool is_reflective;

	// Texture file path
	char diffuse_tex[1024];
	char specular_tex[1024];

	// Animation info
	bool is_animated;
	// Translation
	vec3 translate_velocity;
	// Rotation
	vec3 rotate_velocity;
	
	// Highlight indicator
	bool is_highlighted;
	color3 highlightColor;

	// Constructor and destructor
	Mesh(); // Don't use this explicitly
	Mesh(GLuint skyboxShader); // For skybox construction only
	Mesh(GLuint shader, vec3 position, vec3 rotation, vec3 scale, vec2 uvScale, vec2 uvShift, float uvRotation);
	~Mesh();

	void setupBuffers(); // Setup/Refresh buffer arrays
	void setupMaterial(int materialRed, int materialGreen, int materialBlue, float materialShininess); // Modify material
	void setupMaterial(vec3 color, float materialShininess);

	void glDraw(mat4 projection, bool shadow, bool reflection, const char* drawMode, vec3 camPos, GLuint reflectTex); // Draw by OpenGL
	void setDiffuseTexture(GLuint texture);    // Set diffuse map
	void setSpecularTexture(GLuint texture);   // Set specular map

	void replaceShader(GLuint shader); // Replace bound shader
	void setTextureFilePath(const char* diffuse, const char* specular); // Bind texture file path

	void highlight(bool rollback, vec3 color); // Highlight model

private:
	GLuint shader; // Used shader

	GLuint vao; // Vertex array
	GLuint vbo; // Buffer array
	//GLuint ebo; // Element buffer array

	// Texture info
	Texture diffuse_texture;
	Texture specular_texture;

	// Texture indicator
	bool has_diffuse_tetxure;
	bool has_specular_texture;

	void setupAttributePointers();  // Setup vertex attribute pointers
	void setShaderMaterial(bool use_reflection);		// Set material parameters in shader
};