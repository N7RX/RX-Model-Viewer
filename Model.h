#pragma once

#include <angel\Angel.h>
#include <vector>
#include "Mesh.h"

typedef vec3 color3;


class Model
{
public:
	// Meshess
	vector<Mesh> mesh;

	Model();
	~Model();

	// Draw model
	void glDraw(mat4 projection, bool shadow, bool reflection, const char* drawMode, vec3 camPos, GLuint reflectTex);

	// Reconstruct all meshes' buffers
	void setupModelBuffers();

	// Highlight model (all mesh)
	void highlightModel(bool rollback, vec3 color);

private:

};