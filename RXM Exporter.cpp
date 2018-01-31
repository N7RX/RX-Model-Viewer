//------------------------------------------------------------------------------------------------
// Model Parser for RXM file
// Written by Raymond.X
//------------------------------------------------------------------------------------------------

#include "stdafx.h"
#include "RXM Exporter.h"

using namespace std;


bool RXMExporter::exportToRXM(Model model, const char* filePath)
{
	ofstream fileWriter;
	fileWriter.open(filePath);

	// Header
	fileWriter << "#RX_Model_Viewer model_file\n";
	fileWriter << "#version 1.1\n";
	fileWriter << "\n";

	// Mesh
	for (int i = 0; i < model.mesh.size(); i++) {

		fileWriter << "Mesh " << i << "\n";
		fileWriter << "\n";

		// Transform info
		fileWriter << "position " << model.mesh[i].model_position.x;
		fileWriter << "\/" << model.mesh[i].model_position.y << "\/" << model.mesh[i].model_position.z << "\n";
		fileWriter << "rotation " << model.mesh[i].model_rotation.x;
		fileWriter << "\/" << model.mesh[i].model_rotation.y << "\/" << model.mesh[i].model_rotation.z << "\n";
		fileWriter << "scale " << model.mesh[i].model_scale.x;
		fileWriter << "\/" << model.mesh[i].model_scale.y << "\/" << model.mesh[i].model_scale.z << "\n";
		fileWriter << "\n";

		// UV info
		fileWriter << "uv_scale " << model.mesh[i].uv_scale.x << "\/" << model.mesh[i].uv_scale.y << "\n";
		fileWriter << "uv_shift " << model.mesh[i].uv_shift.x << "\/" << model.mesh[i].uv_shift.y << "\n";
		fileWriter << "uv_rotation " << model.mesh[i].uv_rotation << "\n";
		fileWriter << "\n";

		// Material info
		fileWriter << "material_color " << model.mesh[i].material_color[0];
		fileWriter << "\/" << model.mesh[i].material_color[1] << "\/" << model.mesh[i].material_color[2] << "\n";
		fileWriter << "material_shininess " << model.mesh[i].material_shininess << "\n";
		fileWriter << "reflective " << model.mesh[i].is_reflective << "\n";
		fileWriter << "\n";

		// Texture file path info
		if (strcmp(model.mesh[i].diffuse_tex, "") != 0)
			fileWriter << "diffuse " << model.mesh[i].diffuse_tex << "\n";
		else
			fileWriter << "diffuse null\n";
		if (strcmp(model.mesh[i].specular_tex, "") != 0)
			fileWriter << "specular " << model.mesh[i].specular_tex << "\n";
		else
			fileWriter << "specular null\n";
		fileWriter << "\n";

		// Animation info
		fileWriter << "animated " << model.mesh[i].is_animated << "\n";
		fileWriter << "translate_velocity " << model.mesh[i].translate_velocity.x;
		fileWriter << "\/" << model.mesh[i].translate_velocity.y << "\/" << model.mesh[i].translate_velocity.z << "\n";
		fileWriter << "rotate_velocity " << model.mesh[i].rotate_velocity.x;
		fileWriter << "\/" << model.mesh[i].rotate_velocity.y << "\/" << model.mesh[i].rotate_velocity.z << "\n";
		fileWriter << "\n";

		// Data marker
		fileWriter << "#vertices\n";
		fileWriter << "\n";

		// Vertices data
		for (int j = 0; j < model.mesh[i].vertices.size(); j++) {
			// Vertex
			fileWriter << "v " << model.mesh[i].vertices[j].position.x;
			fileWriter << "\/" << model.mesh[i].vertices[j].position.y << "\/" << model.mesh[i].vertices[j].position.z << "\n";
			// Normal
			fileWriter << "vn " << model.mesh[i].vertices[j].normal.x;
			fileWriter << "\/" << model.mesh[i].vertices[j].normal.y << "\/" << model.mesh[i].vertices[j].normal.z << "\n";
			// TexCoords
			fileWriter << "vt " << model.mesh[i].vertices[j].tex_coords.x << "\/" << model.mesh[i].vertices[j].tex_coords.y << "\n";
		}

	}

	fileWriter.close();

	return true;
}

bool RXMExporter::exportToRXMS(vector<Model> model, const char* filePath)
{
	ofstream fileWriter;
	fileWriter.open(filePath);

	// Header
	fileWriter << "#RX_Model_Viewer scene_file\n";
	fileWriter << "#version 1.1\n";
	fileWriter << "\n";

	// Write each model's data
	for (int i = 0; i < model.size(); i++) {

		// Mark a model
		fileWriter << "Model " << i << "\n";
		fileWriter << "\n";

		// Each mesh of each model
		for (int j = 0; j < model[i].mesh.size(); j++) {

			fileWriter << "Mesh " << j << "\n";
			fileWriter << "\n";

			// Transform info
			fileWriter << "position " << model[i].mesh[j].model_position.x;
			fileWriter << "\/" << model[i].mesh[j].model_position.y << "\/" << model[i].mesh[j].model_position.z << "\n";
			fileWriter << "rotation " << model[i].mesh[j].model_rotation.x;
			fileWriter << "\/" << model[i].mesh[j].model_rotation.y << "\/" << model[i].mesh[j].model_rotation.z << "\n";
			fileWriter << "scale " << model[i].mesh[j].model_scale.x;
			fileWriter << "\/" << model[i].mesh[j].model_scale.y << "\/" << model[i].mesh[j].model_scale.z << "\n";
			fileWriter << "\n";

			// UV info
			fileWriter << "uv_scale " << model[i].mesh[j].uv_scale.x << "\/" << model[i].mesh[j].uv_scale.y << "\n";
			fileWriter << "uv_shift " << model[i].mesh[j].uv_shift.x << "\/" << model[i].mesh[j].uv_shift.y << "\n";
			fileWriter << "uv_rotation " << model[i].mesh[j].uv_rotation << "\n";
			fileWriter << "\n";

			// Material info
			fileWriter << "material_color " << model[i].mesh[j].material_color[0];
			fileWriter << "\/" << model[i].mesh[j].material_color[1] << "\/" << model[i].mesh[j].material_color[2] << "\n";
			fileWriter << "material_shininess " << model[i].mesh[j].material_shininess << "\n";
			fileWriter << "reflective " << model[i].mesh[j].is_reflective << "\n";
			fileWriter << "\n";

			// Texture file path info
			if (strcmp(model[i].mesh[j].diffuse_tex, "") != 0)
				fileWriter << "diffuse " << model[i].mesh[j].diffuse_tex << "\n";
			else
				fileWriter << "diffuse null\n";
			if (strcmp(model[i].mesh[j].specular_tex, "") != 0)
				fileWriter << "specular " << model[i].mesh[j].specular_tex << "\n";
			else
				fileWriter << "specular null\n";
			fileWriter << "\n";

			// Animation info
			fileWriter << "animated " << model[i].mesh[j].is_animated << "\n";
			fileWriter << "translate_velocity " << model[i].mesh[j].translate_velocity.x;
			fileWriter << "\/" << model[i].mesh[j].translate_velocity.y << "\/" << model[i].mesh[j].translate_velocity.z << "\n";
			fileWriter << "rotate_velocity " << model[i].mesh[j].rotate_velocity.x;
			fileWriter << "\/" << model[i].mesh[j].rotate_velocity.y << "\/" << model[i].mesh[j].rotate_velocity.z << "\n";
			fileWriter << "\n";

			// Data marker
			fileWriter << "#vertices\n";
			fileWriter << "\n";

			// Vertices data
			for (int k = 0; k < model[i].mesh[j].vertices.size(); k++) {
				// Vertex
				fileWriter << "v " << model[i].mesh[j].vertices[k].position.x;
				fileWriter << "\/" << model[i].mesh[j].vertices[k].position.y << "\/" << model[i].mesh[j].vertices[k].position.z << "\n";
				// Normal
				fileWriter << "vn " << model[i].mesh[j].vertices[k].normal.x;
				fileWriter << "\/" << model[i].mesh[j].vertices[k].normal.y << "\/" << model[i].mesh[j].vertices[k].normal.z << "\n";
				// TexCoords
				fileWriter << "vt " << model[i].mesh[j].vertices[k].tex_coords.x << "\/" << model[i].mesh[j].vertices[k].tex_coords.y << "\n";
			}

			fileWriter << "\n";

		}

	}

	fileWriter.close();

	return true;
}