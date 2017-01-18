//------------------------------------------------------------------------------------------------
// Model Parser for OBJ file
// Adapted by Raymond.X
//------------------------------------------------------------------------------------------------

#include "stdafx.h" // Must call first
#include "ObjLoader.h"


int ObjLoader::readFile(const char* addrstr, vector<Model> &model, GLuint defaultShader, bool multiMesh)
{
	model.push_back(*new Model()); // Create a new model
	int modelIndex = model.size() - 1; // Current operating model

	int mesh_num = 0; // Total mesh number

	int parsingMode = 0; // 0: Group by material (GBM), 1: Group by objects (GBO)

	vec3 indicesOffsets(0, 0, 0); // For GBO parsing

	// Buffers
	vector<vec4> temp_positions;
	vector<vec4> temp_normals;
	vector<vec4> temp_texcoords;
	vector<GLuint> vertexIndices;
	vector<GLuint> uvIndices;
	vector<GLuint> normalIndices;

	vector<vec3> temp_group; // Record offsets between each mesh in non-multi-mesh mode

	// Vertex and normal rotate matrices
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

	int f_count = 0;	  // Face count
	int g_count = -1;	  // Mesh ncount
	bool have_uv = false; // Indicate whether this mesh has UV indices

	temp_group.push_back(vec3(0, 0, 0)); // First mesh has no offset

	FILE * file;
	fopen_s(&file, addrstr, "rb");
	if (file == NULL) {
		CString notifStr("Cannot open this file.");
		AfxMessageBox(notifStr);
		return -1;
	}

	// Create new mesh
	model[modelIndex].mesh.push_back(*new Mesh(defaultShader, vec3(position.x, position.y, position.z),
		vec3(rotation.x, rotation.y, rotation.z), vec3(scale.x, scale.y, scale.z),
		vec2(uvScale.x, uvScale.y), vec2(uvShift.x, uvShift.y), uvRotation));
	mesh_num++;

	while (1) {

		// Line header buffer
		char lineHeader[128];

		// Read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // End of file, quit reading

		// Group by material or group by object
		if (strcmp(lineHeader, "usemtl") == 0 || strcmp(lineHeader, "object") == 0) {

			if (parsingMode == 0) {
				g_count++;
				have_uv = false; // Reset UV indicator
			}

			if (parsingMode == 1) {
				if (strcmp(lineHeader, "usemtl") == 0)
					continue;
				else {
					g_count++;
					have_uv = false;
				}
			}

			if (strcmp(lineHeader, "object") == 0)
				parsingMode = 1; // GBO detected

			if (g_count != 0) { // More than one mesh
				// Record mesh offset	
				if (!multiMesh) temp_group.push_back(vec3(temp_positions.size(), temp_texcoords.size(), temp_normals.size()));

				// Create new mesh
				else if (multiMesh) {
					temp_group.push_back(vec3(0, 0, 0)); // No offsets in multi-mesh mode

					if (parsingMode == 1) { // GBO format's indices are as a whole
						temp_group.pop_back();
						temp_group.push_back(vec3(temp_positions.size(), temp_texcoords.size(), temp_normals.size()));
					}

					// Output previous mesh's data before starting a new one
#pragma region Output mesh data
					unsigned int start, end;
					if (parsingMode == 0) {
						start = 0; end = vertexIndices.size();
					}
					else {
						start = indicesOffsets[0]; end = vertexIndices.size();
					}
					// For each vertex of each triangle
					if (vertexIndices.size() == uvIndices.size()) { // Has correct texture coordinates
						for (unsigned int i = start; i < end; i++) {

							// Get the indices of the attributes
							unsigned int vertexIndex = vertexIndices[i];
							unsigned int normalIndex = normalIndices[i];
							unsigned int uvIndex = uvIndices[i];

							// Get the attributes from the index
							vec4 vertex = temp_positions[vertexIndex - 1];
							vec4 normal = temp_normals[normalIndex - 1];
							vec4 uv = temp_texcoords[uvIndex - 1];

							Vertex temp_vertex;
							// Put attributes into the buffers
							temp_vertex.position = rotate*vertex*scale + position;
							temp_vertex.normal = rotate*normal;
							temp_vertex.tex_coords = rotate_uv*uv*uvScale + uvShift;

							model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertices.push_back(temp_vertex);
						}
					}
					else { // Does not have texture coordinates or are incorrect
						for (unsigned int i = start; i < end; i++) {

							unsigned int vertexIndex = vertexIndices[i];
							unsigned int normalIndex = normalIndices[i];

							vec4 vertex = temp_positions[vertexIndex - 1];
							vec4 normal = temp_normals[normalIndex - 1];
							vec4 uv(0, 0, 0, 0);

							Vertex temp_vertex;
							temp_vertex.position = rotate*vertex*scale + position;
							temp_vertex.normal = rotate*normal;
							temp_vertex.tex_coords = uv;

							model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertices.push_back(temp_vertex);
						}
					}

					// Output mesh info
					model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].face_number = f_count;				 // Face number
					model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertex_number = temp_positions.size(); // Vextex number

					// Clear buffer
					if (parsingMode == 0) {
						temp_positions.swap(vector<vec4>());
						temp_normals.swap(vector<vec4>());
						temp_texcoords.swap(vector<vec4>());
						vertexIndices.swap(vector<GLuint>());
						uvIndices.swap(vector<GLuint>());
						normalIndices.swap(vector<GLuint>());
					}
#pragma endregion
					f_count = 0; // Reset face number

					// Start a new mesh
					model[modelIndex].mesh.push_back(*new Mesh(defaultShader, vec3(position.x, position.y, position.z),
						vec3(rotation.x, rotation.y, rotation.z), vec3(scale.x, scale.y, scale.z),
						vec2(uvScale.x, uvScale.y), vec2(uvShift.x, uvShift.y), uvRotation));
					mesh_num++;
				}
			}

			indicesOffsets = vec3(vertexIndices.size(), uvIndices.size(), normalIndices.size());

		}

		// Vertex
		else if (strcmp(lineHeader, "v") == 0) {
			vec4 vertex(0, 0, 0, 1.0f);
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_positions.push_back(vertex);
		}

		// Texture coordinates
		else if (strcmp(lineHeader, "vt") == 0) {
			have_uv = true; // This mesh has UV coordinates
			vec4 uv(0, 0, 0, 0);
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			//uv.y = -uv.y; // Invert V coordinate if DDS texture is used
			temp_texcoords.push_back(uv);
		}

		// Normal
		else if (strcmp(lineHeader, "vn") == 0) {
			vec4 normal(0, 0, 0, 0);
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}

		// Face
		else if (strcmp(lineHeader, "f") == 0) {

			string vertex1, vertex2, vertex3;
			int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches;

			if (have_uv) // With UV
				matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			else // Without UV
				matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
			
			if (matches == 6) {
				if (g_count > 0) { // With mesh offset
					vertexIndices.push_back(abs(vertexIndex[0]) + temp_group[g_count][0]);
					vertexIndices.push_back(abs(vertexIndex[1]) + temp_group[g_count][0]);
					vertexIndices.push_back(abs(vertexIndex[2]) + temp_group[g_count][0]);

					normalIndices.push_back(abs(normalIndex[0]) + temp_group[g_count][2]);
					normalIndices.push_back(abs(normalIndex[1]) + temp_group[g_count][2]);
					normalIndices.push_back(abs(normalIndex[2]) + temp_group[g_count][2]);
				}
				else { // Without mesh offset
					vertexIndices.push_back(abs(vertexIndex[0]));
					vertexIndices.push_back(abs(vertexIndex[1]));
					vertexIndices.push_back(abs(vertexIndex[2]));

					normalIndices.push_back(abs(normalIndex[0]));
					normalIndices.push_back(abs(normalIndex[1]));
					normalIndices.push_back(abs(normalIndex[2]));
				}
			}

			else if(matches == 9) {
				if (g_count > 0) {
					vertexIndices.push_back(abs(vertexIndex[0]) + temp_group[g_count][0]);
					vertexIndices.push_back(abs(vertexIndex[1]) + temp_group[g_count][0]);
					vertexIndices.push_back(abs(vertexIndex[2]) + temp_group[g_count][0]);

					uvIndices.push_back(abs(uvIndex[0]) + temp_group[g_count][1]);
					uvIndices.push_back(abs(uvIndex[1]) + temp_group[g_count][1]);
					uvIndices.push_back(abs(uvIndex[2]) + temp_group[g_count][1]);

					normalIndices.push_back(abs(normalIndex[0]) + temp_group[g_count][2]);
					normalIndices.push_back(abs(normalIndex[1]) + temp_group[g_count][2]);
					normalIndices.push_back(abs(normalIndex[2]) + temp_group[g_count][2]);
				}
				else {
					vertexIndices.push_back(abs(vertexIndex[0]));
					vertexIndices.push_back(abs(vertexIndex[1]));
					vertexIndices.push_back(abs(vertexIndex[2]));

					uvIndices.push_back(abs(uvIndex[0]));
					uvIndices.push_back(abs(uvIndex[1]));
					uvIndices.push_back(abs(uvIndex[2]));

					normalIndices.push_back(abs(normalIndex[0]));
					normalIndices.push_back(abs(normalIndex[1]));
					normalIndices.push_back(abs(normalIndex[2]));
				}
			}

			else {
				CString notifStr("File format is not supported.");
				AfxMessageBox(notifStr);
				return false;
			}
			f_count++;
		}

		else {
			if (parsingMode == 0) { // GBO parsing needs detecting 'object' keyword, can't ignore the rest
				// Comments or other things that take up the rest of the line
				char extraBuffer[1024];
				fgets(extraBuffer, 1024, file);
			}
		}

	}

	// Output last mesh's data
#pragma region Output mesh data
	// For each vertex of each triangle
	if (vertexIndices.size() == uvIndices.size()) { // Has correct texture coordinates
		for (unsigned int i = 0; i < vertexIndices.size(); i++) {

			// Get the indices of the attributes
			unsigned int vertexIndex = vertexIndices[i];
			unsigned int normalIndex = normalIndices[i];
			unsigned int uvIndex = uvIndices[i];

			// Get the attributes from the index
			vec4 vertex = temp_positions[vertexIndex - 1];
			vec4 normal = temp_normals[normalIndex - 1];
			vec4 uv = temp_texcoords[uvIndex - 1];

			Vertex temp_vertex;
			// Put attributes into the buffers
			temp_vertex.position = rotate*vertex*scale + position;
			temp_vertex.normal = rotate*normal;
			temp_vertex.tex_coords = rotate_uv*uv*uvScale + uvShift;

			model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertices.push_back(temp_vertex);
		}
	}
	else { // Does not have texture coordinates or are incorrect
		for (unsigned int i = 0; i < vertexIndices.size(); i++) {

			unsigned int vertexIndex = vertexIndices[i];
			unsigned int normalIndex = normalIndices[i];

			vec4 vertex = temp_positions[vertexIndex - 1];
			vec4 normal = temp_normals[normalIndex - 1];
			vec4 uv(0, 0, 0, 0);

			Vertex temp_vertex;
			temp_vertex.position = rotate*vertex*scale + position;
			temp_vertex.normal = rotate*normal;
			temp_vertex.tex_coords = uv;

			model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertices.push_back(temp_vertex);
		}
	}

	// Output mesh info
	model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].face_number = f_count;				 // Face number
	model[modelIndex].mesh[model[modelIndex].mesh.size() - 1].vertex_number = temp_positions.size(); // Vextex number

	// Release memory
	temp_positions.swap(vector<vec4>());
	temp_normals.swap(vector<vec4>());
	temp_texcoords.swap(vector<vec4>());
	temp_group.swap(vector<vec3>());
	vertexIndices.swap(vector<GLuint>());
	uvIndices.swap(vector<GLuint>());
	normalIndices.swap(vector<GLuint>());

#pragma endregion

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

// For single mesh model, like skybox
int ObjLoader::readFile(const char* addrstr, Mesh &mesh, GLuint defaultShader, bool multiMesh)
{
	int mesh_num = 0;
	vector<Model> tempMod;

	mesh_num = readFile(addrstr, tempMod, defaultShader, multiMesh);
	mesh = tempMod[0].mesh[0];

	tempMod.swap(vector<Model>());
	return mesh_num;
}