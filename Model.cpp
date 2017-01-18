#include "stdafx.h"
#include "Model.h"

using namespace std;


Model::Model() {}

Model::~Model()
{
	// Release memory
	mesh.swap(vector<Mesh>());
}

void Model::glDraw(mat4 projection, bool shadow, bool reflection, const char* drawMode, vec3 camPos, GLuint reflectTex)
{
	// Draw all meshes
	for (int i = 0; i < mesh.size(); i++) {
		mesh[i].glDraw(projection, shadow, reflection, drawMode, camPos, reflectTex);
	}
}

void Model::setupModelBuffers()
{
	for (int i = 0; i < mesh.size(); i++) {
		mesh[i].setupBuffers();
	}
}

void Model::highlightModel(bool rollback, vec3 color)
{
	for (int i = 0; i < mesh.size(); i++) {
		mesh[i].highlight(rollback, color);
	}
}