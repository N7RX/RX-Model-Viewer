#pragma once

#include <angel\Angel.h>
#include <vector>
#include <cmath>
#include "Model.h"
#include "CUDA Accelerator.h"


class ObjLoader
{

protected:

public:
	// Thread number
	int threadNum = 1;

	// CUDA indicator
	bool useCUDA;

	// Model offset
	vec4 position;

	// Mode scale
	vec4 scale;

	// Model rotation
	vec3 rotation;

	// Texture coordinates scale
	vec4 uvScale;

	// Texture coordinates offset
	vec4 uvShift;

	// Texture coordinates rotation
	float uvRotation;

	// Should adjust model position
	bool stickToGround;

	// Read OBJ file
	int readFile(const char* addrstr, vector<Model> &model, GLuint defaultShader, bool multiMesh); // Common use
	int readFile(const char* addrstr, Mesh &mesh, GLuint defaultShader, bool multiMesh); // Scene elements loading
};

