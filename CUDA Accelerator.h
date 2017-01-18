#pragma once

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <angel\Angel.h>
#include "Mesh.h"

#define TILE_WIDTH 16 // Block size

using namespace std;

// Complete model transaltion
extern "C" void translate_CUDA(vector<Vertex> &vertices, vec4 translation);

// Complete model rotation
extern "C" void rotate_CUDA(vector<Vertex> &vertices, mat4 rotation);

// Complete model rescaling
extern "C" void rescale_CUDA(vector<Vertex> &vertices, vec4 scale);

// Complete texture coordinates shifting
extern "C" void shiftUV_CUDA(vector<Vertex> &vertices, vec4 shift);

// Complete texture coordinates rescaling
extern "C" void rescaleUV_CUDA(vector<Vertex> &vertices, vec4 scale);

// Complete texture coordinates rotation
extern "C" void rotateUV_CUDA(vector<Vertex> &vertices, float angle);

// Complete model position adjustment (stick to ground)
extern "C" void stg_translate_CUDA(vector<vec4> &positions, float compensation);