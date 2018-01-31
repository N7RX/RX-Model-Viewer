#pragma once

#include <angel\Angel.h>
#include <vector>
#include <fstream>
#include "Model.h"


class RXMExporter
{
public:
	
	// Export single model
	bool exportToRXM(Model model, const char* filePath);

	// Export current scene
	bool exportToRXMS(vector<Model> model, const char* filePath);
};
