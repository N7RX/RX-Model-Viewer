//------------------------------------------------------------------------------------------------
// Model Parser for RXM file
// Written by Raymond.X
//------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "RXM Importer.h"

using namespace std;


int RXMImporter::readRXMFile(const char* addrstr, vector<Model> &model, GLuint defaultShader)
{
	model.push_back(*new Model()); // Create a new model
	int modelIndex = model.size() - 1; // Current operating model

	int mesh_num = 0; // Total mesh number

	// Texture file path buffer
	char tempStr[1024];

	// Mesh rotate matrices
	float cx = cos(rotation.x), sx = sin(rotation.x);
	float cy = cos(rotation.y), sy = sin(rotation.y);
	float cz = cos(rotation.z), sz = sin(rotation.z);
	mat4 rotateX{
		1, 0, 0, 0,
		0, cx,-sx, 0,
		0, sx, cx, 0,
		0, 0, 0, 1
	};
	mat4 rotateY{
		cy, 0, sy, 0,
		0, 1, 0, 0,
		-sy, 0, cy, 0,
		0, 0, 0, 1
	};
	mat4 rotateZ{
		cz,-sz, 0, 0,
		sz, cz, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	mat4 rotate = rotateX * rotateY * rotateZ;

	// UV rotate matrix
	float cuv = cos(uvRotation), suv = sin(uvRotation);
	mat4 rotate_uv{
		cuv, suv, 0, 0,
		-suv, cuv, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};

	FILE * file;
	fopen_s(&file, addrstr, "rb");
	if (file == NULL) {
		CString notifStr("Cannot open this file.");
		AfxMessageBox(notifStr);
		return -1;
	}
	
	bool endOfFile = false; // Mark end of file

	// Read model data
	while (1) {

		// Read mesh info
		while (1) {

			// Line header buffer
			char lineHeader[128];

			// Read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (strcmp(lineHeader, "#vertices") == 0 || res == EOF)
				break; // Start of vertices data, quit the loop

			// New mesh
			else if (strcmp(lineHeader, "Mesh") == 0 || strcmp(lineHeader, "mesh") == 0) {

				model[modelIndex].mesh.push_back(*new Mesh(defaultShader, vec3(position.x, position.y, position.z),
					vec3(rotation.x, rotation.y, rotation.z), vec3(scale.x, scale.y, scale.z),
					vec2(uvScale.x, uvScale.y), vec2(uvShift.x, uvShift.y), uvRotation));
			}

			// Mesh position info
			else if (strcmp(lineHeader, "position") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_position.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_position.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_position.z);
			}

			// Mesh rotation info
			else if (strcmp(lineHeader, "rotation") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_rotation.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_rotation.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_rotation.z);
			}

			// Mesh scale info
			else if (strcmp(lineHeader, "scale") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_scale.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_scale.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_scale.z);
			}

			// UV scale info
			else if (strcmp(lineHeader, "uv_scale") == 0) {
				fscanf(file, "%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_scale[0], 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_scale[1]);
			}

			// UV shift info
			else if (strcmp(lineHeader, "uv_shift") == 0) {
				fscanf(file, "%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_shift[0], 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_shift[1]);
			}

			// UV rotation info
			else if (strcmp(lineHeader, "uv_rotation") == 0) {
				fscanf(file, "%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_rotation);
			}

			// Mesh material color info
			else if (strcmp(lineHeader, "material_color") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].material_color[0], 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].material_color[1], 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].material_color[2]);
			}

			// Mesh material shininess info
			else if (strcmp(lineHeader, "material_shininess") == 0) {
				fscanf(file, "%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].material_shininess);
			}

			// Mesh reflection info
			else if (strcmp(lineHeader, "reflective") == 0) {
				fscanf(file, "%d\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].is_reflective);
			}

			// Mesh diffuse texture info
			else if (strcmp(lineHeader, "diffuse") == 0) {
				fscanf(file, "\n%[^\n^\r]", &tempStr);
				if (strcmp(tempStr, "null") != 0) {
					strcpy(model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].diffuse_tex, tempStr);
				}
			}

			// Mesh specular texture info
			else if (strcmp(lineHeader, "specular") == 0) {
				fscanf(file, "\n%[^\n^\r]", &tempStr);
				if (strcmp(tempStr, "null") != 0) {
					strcpy(model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].specular_tex, tempStr);
				}
			}

			// Mesh animation info
			else if (strcmp(lineHeader, "animated") == 0) {
				fscanf(file, "%d\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].is_animated);
			}

			// Mesh tranlsation velocity info
			else if (strcmp(lineHeader, "translate_velocity") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].translate_velocity.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].translate_velocity.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].translate_velocity.z);
			}

			// Mesh rotation velocity info
			else if (strcmp(lineHeader, "rotate_velocity") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].rotate_velocity.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].rotate_velocity.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].rotate_velocity.z);
			}

		}

		Vertex tempVertex;
		int f_count = 0; // Face number
		int v_count = 0; // Vertex info count

		// Read mesh vertices data
		while (1) {

			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF) {
				endOfFile = true;
				break; // End of file, quit reading
			}

			// New mesh
			else if (strcmp(lineHeader, "Mesh") == 0 || strcmp(lineHeader, "mesh") == 0) {

				// Record mesh statistics
				model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].face_number = f_count / 3;
				model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertex_number = f_count / 5; // Temporary approximation
				f_count = 0; // Reset

				model[modelIndex].mesh.push_back(*new Mesh(defaultShader, vec3(position.x, position.y, position.z),
					vec3(rotation.x, rotation.y, rotation.z), vec3(scale.x, scale.y, scale.z),
					vec2(uvScale.x, uvScale.y), vec2(uvShift.x, uvShift.y), uvRotation));

				break; // Start previous loop to read mesh info 
			}

			// Vertex
			if (strcmp(lineHeader, "v") == 0) {
				vec4 vertex(0, 0, 0, 1.0f);
				fscanf(file, "%f/%f/%f\n", &vertex.x, &vertex.y, &vertex.z);
				tempVertex.position = rotate*vertex*scale + position;
				v_count++;
				f_count++;
			}

			// Normal
			else if (strcmp(lineHeader, "vn") == 0) {
				vec4 normal(0, 0, 0, 0);
				fscanf(file, "%f/%f/%f\n", &normal.x, &normal.y, &normal.z);
				tempVertex.normal = rotate*normal;
				v_count++;
			}

			// TexCoords
			else if (strcmp(lineHeader, "vt") == 0) {
				vec4 uv(0, 0, 0, 0);
				fscanf(file, "%f/%f\n", &uv.x, &uv.y);
				tempVertex.tex_coords = rotate_uv*uv*uvScale + uvShift;
				v_count++;

				if (v_count == 3) { // One face's data is complete
					v_count = 0;
					model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertices.push_back(tempVertex);
				}
			}

		}

		if (endOfFile) break;

	}

	// Adjust model position to stick it to the ground (make Y=0)
	if (stickToGround) {

		int start = model[modelIndex].mesh.size() - 1;
		int end = model[modelIndex].mesh.size() - 1 - mesh_num;

		// Find lowest point (Y axis)
		GLfloat lowest_point = model[modelIndex].mesh[start].vertices[0].position.y;
		for (int i = start; i > end; i--) {
			for (int j = 0; j < model[modelIndex].mesh[i].vertices.size(); j++) {
				if (model[modelIndex].mesh[i].vertices[j].position.y < lowest_point) lowest_point = model[modelIndex].mesh[i].vertices[j].position.y;
			}
		}

		// Compensating offset
		if (lowest_point != 0) {
			GLfloat compensation = -lowest_point;

			for (int i = start; i > end; i--) {
				if (useCUDA) { // Use CUDA acceleration

					vector<vec4> tempPosition;

					for (int j = 0; j < model[modelIndex].mesh[i].vertices.size(); j++) {
						tempPosition.push_back(model[modelIndex].mesh[i].vertices[j].position);
					}

					stg_translate_CUDA(tempPosition, compensation);

#pragma omp parallel for num_threads(threadNum) 
					for (int j = 0; j < model[modelIndex].mesh[i].vertices.size(); j++) {
						model[modelIndex].mesh[i].vertices[j].position.y = tempPosition[j].y;
					}

					tempPosition.swap(vector<vec4>());
				}

				else { // Use OpenMP
#pragma omp parallel for num_threads(threadNum) 
					for (int j = 0; j < model[modelIndex].mesh[i].vertices.size(); j++) {
						model[modelIndex].mesh[i].vertices[j].position.y = model[modelIndex].mesh[i].vertices[j].position.y + compensation;
					}
				}
			}

		}
	}

	return mesh_num; // Return total mesh number
}

int RXMImporter::readRXMSFile(const char* addrstr, vector<Model> &model, GLuint defaultShader)
{
	int modelIndex = model.size() - 1; // Current operating model

	int model_num = 0; // Total model number

	// Texture file path buffer
	char tempStr[1024];

	// Mesh rotate matrices
	float cx = cos(rotation.x), sx = sin(rotation.x);
	float cy = cos(rotation.y), sy = sin(rotation.y);
	float cz = cos(rotation.z), sz = sin(rotation.z);
	mat4 rotateX{
		1, 0, 0, 0,
		0, cx,-sx, 0,
		0, sx, cx, 0,
		0, 0, 0, 1
	};
	mat4 rotateY{
		cy, 0, sy, 0,
		0, 1, 0, 0,
		-sy, 0, cy, 0,
		0, 0, 0, 1
	};
	mat4 rotateZ{
		cz,-sz, 0, 0,
		sz, cz, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	mat4 rotate = rotateX * rotateY * rotateZ;

	// UV rotate matrix
	float cuv = cos(uvRotation), suv = sin(uvRotation);
	mat4 rotate_uv{
		cuv, suv, 0, 0,
		-suv, cuv, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};

	FILE * file;
	fopen_s(&file, addrstr, "rb");
	if (file == NULL) {
		CString notifStr("Cannot open this file.");
		AfxMessageBox(notifStr);
		return 0;
	}


	bool newMesh = false;
	bool newModel = false;
	bool firstModel = true;
	bool endOfFile = false;

	// Read scene data
	while (1) {

		// Read first model start mark
		while (firstModel) {
			// Line header buffer
			char lineHeader[128];
			// Read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF) break; // Error, shouldn't come here
			// Create first new model
			else if (strcmp(lineHeader, "Model") == 0 || strcmp(lineHeader, "model") == 0) {
				model.push_back(*new Model());
				modelIndex = model.size() - 1;

				model_num++;
			}
			// Create first new mesh
			else if (strcmp(lineHeader, "Mesh") == 0 || strcmp(lineHeader, "mesh") == 0) {
				model[modelIndex].mesh.push_back(*new Mesh(defaultShader, vec3(position.x, position.y, position.z),
					vec3(rotation.x, rotation.y, rotation.z), vec3(scale.x, scale.y, scale.z),
					vec2(uvScale.x, uvScale.y), vec2(uvShift.x, uvShift.y), uvRotation));

				firstModel = false;
				break;
			}
		}

		// Read mesh info
		while (1) {

			char lineHeader[128];

			int res = fscanf(file, "%s", lineHeader);
			if (strcmp(lineHeader, "#vertices") == 0 || res == EOF)
				break; // Start of vertices data or error, quit the loop

			// Mesh position info
			else if (strcmp(lineHeader, "position") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_position.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_position.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_position.z);
			}

			// Mesh rotation info
			else if (strcmp(lineHeader, "rotation") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_rotation.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_rotation.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_rotation.z);
			}

			// Mesh scale info
			else if (strcmp(lineHeader, "scale") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_scale.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_scale.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].model_scale.z);
			}

			// UV scale info
			else if (strcmp(lineHeader, "uv_scale") == 0) {
				fscanf(file, "%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_scale[0], 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_scale[1]);
			}

			// UV shift info
			else if (strcmp(lineHeader, "uv_shift") == 0) {
				fscanf(file, "%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_shift[0], 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_shift[1]);
			}

			// UV rotation info
			else if (strcmp(lineHeader, "uv_rotation") == 0) {
				fscanf(file, "%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].uv_rotation);
			}

			// Mesh material color info
			else if (strcmp(lineHeader, "material_color") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].material_color[0], 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].material_color[1], 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].material_color[2]);
			}

			// Mesh material shininess info
			else if (strcmp(lineHeader, "material_shininess") == 0) {
				fscanf(file, "%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].material_shininess);
			}

			// Mesh reflection info
			else if (strcmp(lineHeader, "reflective") == 0) {
				fscanf(file, "%d\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].is_reflective);
			}

			// Mesh diffuse texture info
			else if (strcmp(lineHeader, "diffuse") == 0) {
				fscanf(file, "\n%[^\n^\r]", &tempStr);
				if (strcmp(tempStr, "null") != 0) {
					strcpy(model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].diffuse_tex, tempStr);
				}
			}

			// Mesh specular texture info
			else if (strcmp(lineHeader, "specular") == 0) {
				fscanf(file, "\n%[^\n^\r]", &tempStr);
				if (strcmp(tempStr, "null") != 0) {
					strcpy(model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].specular_tex, tempStr);
				}
			}

			// Mesh animation info
			else if (strcmp(lineHeader, "animated") == 0) {
				fscanf(file, "%d\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].is_animated);
			}

			// Mesh tranlsation velocity info
			else if (strcmp(lineHeader, "translate_velocity") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].translate_velocity.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].translate_velocity.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].translate_velocity.z);
			}

			// Mesh rotation velocity info
			else if (strcmp(lineHeader, "rotate_velocity") == 0) {
				fscanf(file, "%f/%f/%f\n", &model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].rotate_velocity.x, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].rotate_velocity.y, 
					&model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].rotate_velocity.z);
			}

		}

		Vertex tempVertex;
		int f_count = 0; // Face number
		int v_count = 0; // Vertex info count

		// Read vertices data
		while (1) {

			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF) {
				endOfFile = true;
				break; // End of file
			}

			if (strcmp(lineHeader, "Mesh") == 0 || strcmp(lineHeader, "mesh") == 0) {
				newMesh = true;
				break; // Start of a new mesh
			}

			if (strcmp(lineHeader, "Model") == 0 || strcmp(lineHeader, "model") == 0) {
				newModel = true;
				break; // Start of a new model
			}

			// Vertex
			if (strcmp(lineHeader, "v") == 0) {
				vec4 vertex(0, 0, 0, 1.0f);
				fscanf(file, "%f/%f/%f\n", &vertex.x, &vertex.y, &vertex.z);
				tempVertex.position = rotate*vertex*scale + position;
				v_count++;
				f_count++;
			}

			// Normal
			else if (strcmp(lineHeader, "vn") == 0) {
				vec4 normal(0, 0, 0, 0);
				fscanf(file, "%f/%f/%f\n", &normal.x, &normal.y, &normal.z);
				tempVertex.normal = rotate*normal;
				v_count++;
			}

			// TexCoords
			else if (strcmp(lineHeader, "vt") == 0) {
				vec4 uv(0, 0, 0, 0);
				fscanf(file, "%f/%f\n", &uv.x, &uv.y);
				tempVertex.tex_coords = rotate_uv*uv*uvScale + uvShift;
				v_count++;
				if (v_count == 3) {
					v_count = 0;
					model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertices.push_back(tempVertex);
				}
			}

		}

		model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].face_number = f_count / 3;
		model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertex_number = f_count / 5; // Temporary approximation

		// End of file
		if (endOfFile) break;

		// Create new mesh
		if (newMesh) {
			newMesh = false;

			model[modelIndex].mesh.push_back(*new Mesh(defaultShader, vec3(position.x, position.y, position.z),
				vec3(rotation.x, rotation.y, rotation.z), vec3(scale.x, scale.y, scale.z),
				vec2(uvScale.x, uvScale.y), vec2(uvShift.x, uvShift.y), uvRotation));
		}

		// Create a new model
		if (newModel) {
			newModel = false;

			model.push_back(*new Model());
			modelIndex = model.size() - 1;
			model[modelIndex].mesh.push_back(*new Mesh(defaultShader, vec3(position.x, position.y, position.z),
				vec3(rotation.x, rotation.y, rotation.z), vec3(scale.x, scale.y, scale.z),
				vec2(uvScale.x, uvScale.y), vec2(uvShift.x, uvShift.y), uvRotation));

			model_num++;
		}

	}


	// Adjust model position to stick it to the ground (make Y=0)
	if (stickToGround) {

		// Find lowest point (Y axis)
		GLfloat lowest_point = model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertices[0].position.y;

		for (int j = modelIndex; j > modelIndex - model_num; j--) {
			int start = model[j].mesh.size() - 1;
			int end = -1;
			for (int i = start; i > end; i--) {
				for (int k = 0; k < model[j].mesh[i].vertices.size(); k++) {
					if (model[j].mesh[i].vertices[k].position.y < lowest_point) lowest_point = model[j].mesh[i].vertices[k].position.y;
				}
			}
		}

		// Compensating offset
		if (lowest_point != 0) {
			GLfloat compensation = -lowest_point;

			// Adjust each model
			for (int k = modelIndex; k > modelIndex - model_num; k--) {

				int start = model[k].mesh.size() - 1;
				int end = -1;

				for (int i = start; i > end; i--) {
					if (useCUDA) { // Use CUDA acceleration

						vector<vec4> tempPosition;

						for (int j = 0; j < model[k].mesh[i].vertices.size(); j++) {
							tempPosition.push_back(model[k].mesh[i].vertices[j].position);
						}

						stg_translate_CUDA(tempPosition, compensation);

#pragma omp parallel for num_threads(threadNum) 
						for (int j = 0; j < model[k].mesh[i].vertices.size(); j++) {
							model[k].mesh[i].vertices[j].position.y = tempPosition[j].y;
						}

						tempPosition.swap(vector<vec4>());
					}

					else { // Use OpenMP
#pragma omp parallel for num_threads(threadNum) 
						for (int j = 0; j < model[k].mesh[i].vertices.size(); j++) {
							model[k].mesh[i].vertices[j].position.y = model[k].mesh[i].vertices[j].position.y + compensation;
						}
					}
				}

			}

		}
	}

	return model_num;
}