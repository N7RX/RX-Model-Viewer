//-------------------------------------------------------------------------------------------------
// Mesh Unit Class
// Adapted by Raymond.X
//-------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"

using namespace std;

Mesh::Mesh() {}

Mesh::Mesh(GLuint skyboxShader)
{
	this->has_diffuse_tetxure = false;
	this->has_specular_texture = false;

	this->is_reflective = false;

	this->is_highlighted = false;

	// Bind shader
	this->shader = skyboxShader;

	// Generate buffer arrays
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	//glGenBuffers(1, &this->ebo);

	//setupBuffers();
	setupAttributePointers();
}

Mesh::Mesh(GLuint shader, vec3 position, vec3 rotation, vec3 scale, vec2 uvScale, vec2 uvShift, float uvRotation)
{
	// Default values
	this->face_number = 0;
	this->vertex_number = 0;
	this->model_position = position;
	this->model_rotation = rotation;
	this->model_scale = scale;
	this->uv_scale = uvScale;
	this->uv_shift = uvShift;
	this->uv_rotation = uvRotation;
	this->material_color = vec3(255, 255, 255);
	this->material_shininess = 100;
	this->translate_velocity = vec3(0, 0, 0);
	this->rotate_velocity = vec3(0, 0, 0);

	this->is_animated = false;

	this->is_highlighted = false;
	this->highlightColor = color3(0, 183, 195);

	this->is_reflective = false;

	strcpy(this->diffuse_tex, "");
	strcpy(this->specular_tex, "");

	this->has_diffuse_tetxure = false;
	this->has_specular_texture = false;

	// Bind shader
	this->shader = shader;

	// Generate buffer arrays
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	//glGenBuffers(1, &this->ebo);

	//setupBuffers();
	setupAttributePointers();
}

Mesh::~Mesh()
{
	// Release memory
	vertices.swap(vector<Vertex>());
	indices.swap(vector<GLuint>());
}

void Mesh::setupBuffers()
{
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
		&this->vertices[0], GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
	//	&this->indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setupAttributePointers()
{
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

	// Vertex postion pointer
	GLuint vPosition = glGetAttribLocation(shader, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		(GLvoid*)0);

	// Vertex normal pointer
	GLuint vNormal = glGetAttribLocation(shader, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, normal));

	// Vertex texture coordinates pointer
	GLuint vTexCoord = glGetAttribLocation(shader, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, tex_coords));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setupMaterial(int materialRed, int materialGreen, int materialBlue, float materialShininess)
{
	// Update properties
	material_color = color3(materialRed, materialGreen, materialBlue);
	material_shininess = materialShininess;
}

void Mesh::setupMaterial(vec3 color, float materialShininess)
{
	material_color = color;
	material_shininess = materialShininess;
}

void Mesh::setShaderMaterial(bool use_reflection)
{
	GLfloat r, g, b;
	if (!is_highlighted) { // Highlighted
		// Translate color value into decimal form
		r = material_color[0] / 255.0f; g = material_color[1] / 255.0f; b = material_color[2] / 255.0f;

		// Material properties
		// These constants are determined by material's characteristics, customizable in later updates
		vec4 material_ambient(r / 5.0f, g / 5.0f, b / 5.0f, 1.0f);
		vec4 material_diffuse(r, g, b, 1.0f);
		vec4 material_specular(1.0f, 1.0f, 1.0f, 1.0f);

		// Set uniform values
		glUniform4fv(glGetUniformLocation(shader, "MaterialAmbient"), 1, material_ambient);
		glUniform4fv(glGetUniformLocation(shader, "MaterialDiffuse"), 1, material_diffuse);
		glUniform4fv(glGetUniformLocation(shader, "MaterialSpecular"), 1, material_specular);
		glUniform1f(glGetUniformLocation(shader, "MaterialShininess"), material_shininess);
	}
	else { // Normal display
		r = highlightColor[0] / 255.0f; g = highlightColor[1] / 255.0f; b = highlightColor[2] / 255.0f;

		vec4 material_ambient(r / 5.0f, g / 5.0f, b / 5.0f, 1.0f);
		vec4 material_diffuse(r, g, b, 1.0f);
		vec4 material_specular(1.0f, 1.0f, 1.0f, 1.0f);

		glUniform4fv(glGetUniformLocation(shader, "MaterialAmbient"), 1, material_ambient);
		glUniform4fv(glGetUniformLocation(shader, "MaterialDiffuse"), 1, material_diffuse);
		glUniform4fv(glGetUniformLocation(shader, "MaterialSpecular"), 1, material_specular);
		glUniform1f(glGetUniformLocation(shader, "MaterialShininess"), material_shininess);
	}

	// Enable/Disable reflection computation
	if (use_reflection) {
		if (is_reflective) glUniform1i(glGetUniformLocation(shader, "useReflection"), 1);
		else glUniform1i(glGetUniformLocation(shader, "useReflection"), 0);
	}
	else glUniform1i(glGetUniformLocation(shader, "useReflection"), 0);

}

void Mesh::glDraw(mat4 projection, bool shadow, bool reflection, const char* drawMode, vec3 camPos, GLuint reflectTex)
{
	// Bind shader
	glUseProgram(shader);

	// Projection matrix
	glUniformMatrix4fv(glGetUniformLocation(shader, "Projection"), 1, GL_TRUE, projection);

	// Set material info
	setShaderMaterial(reflection);

	if (drawMode == DRAW_LINE) shadow = false;
	// Use shadow
	if (shadow) glUniform1i(glGetUniformLocation(shader, "useShadow"), 1);
	else glUniform1i(glGetUniformLocation(shader, "useShadow"), 0);

	// Bind array
	glBindVertexArray(vao);

	// Bind diffuse map
	if (has_diffuse_tetxure) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse_texture.id);
		glUniform1i(glGetUniformLocation(shader, "texture"), 0);
	}

	// Bind specular map
	if (has_specular_texture) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specular_texture.id);
		glUniform1i(glGetUniformLocation(shader, "specular"), 1);
	}

	// Setup reflection data
	if (is_reflective && reflection) {
		glUniform3f(glGetUniformLocation(shader, "cameraPos"), camPos.x, camPos.y, camPos.z);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, reflectTex);
		glUniform1i(glGetUniformLocation(shader, "reflectTex"), 2);
	}
	
	// Draw into buffers
	if (drawMode == DRAW_LINE) {
		//glDrawElements(GL_LINE_LOOP, this->indices.size(), GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}
	if (drawMode == DRAW_FACE) {
		//glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	}

	glBindVertexArray(0);
}

void Mesh::setDiffuseTexture(GLuint texture)
{
	diffuse_texture.id = texture;
	diffuse_texture.type = DIFFUSE;
	has_diffuse_tetxure = true;
}

void Mesh::setSpecularTexture(GLuint texture)
{
	specular_texture.id = texture;
	specular_texture.type = SPECULAR;
	has_specular_texture = true;
}

void Mesh::replaceShader(GLuint shader)
{
	this->shader = shader;
	setupAttributePointers();
}

void Mesh::setTextureFilePath(const char* diffuse, const char* specular)
{
	strcpy(diffuse_tex, diffuse);
	strcpy(specular_tex, specular);
}

void Mesh::highlight(bool rollback, vec3 color)
{
	if (!rollback) {
		is_highlighted = true;
		highlightColor = color;
	}
	else {
		is_highlighted = false;
	}
}