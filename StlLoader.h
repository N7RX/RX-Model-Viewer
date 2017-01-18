#pragma once

#include <angel\Angel.h>
#include <vector>
#include "Model.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>      
#include <assimp/postprocess.h>

using namespace std;

class StlLoader
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

	// Read STL file
	int readFile(const char* addrstr, vector<Model> &model);
};