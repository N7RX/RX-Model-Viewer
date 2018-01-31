#pragma once

#include <angel\Angel.h>
#include <vector>
#include <cmath>
#include "Model.h"
#include "CUDA Accelerator.h"


class RXMImporter
{

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

	// Read a RXM model
	int readRXMFile(const char* addrstr, vector<Model> &model, GLuint defaultShader);

	// Read a RXM scene
	int readRXMSFile(const char* addrstr, vector<Model> &model, GLuint defaultShader);
};
