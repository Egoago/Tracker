#pragma once
#include <string>

namespace tr {
	static const char* CONFIG_FOLDER = "Config/";
	static const char* MODELS_FOLDER = "Resources/Models/";
	static const char* MODELS_EXTENSION = ".stl";
	static const char* CONFIG_EXTENSION = ".conf";
	static const char* LOADED_OBJECTS_FOLDER = "Resources/Models/Loaded/";
	static const char* LOADED_OBJECT_FILENAME_EXTENSION = ".dat";
	static const char* OBJ_CONFIG_FILE = "objf";
	static const char* REND_CONFIG_FILE = "rend";
	static const char* GEO_CONFIG_FILE = "geo";
	static const char* DCDT3_CONFIG_FILE = "dcdt3";
	static const char* POSE_CONFIG_FILE = "pose";
	static const char* TEST_FRAME_CUBE = "Resources/TestFrames/cube_noise.png";
	static const char* TEST_FRAME_TRIANGLE = "Resources/TestFrames/triangle.png";
	static const char* TEST_FRAME_CYLINDER = "Resources/TestFrames/cylinder.png";
	static const char* SHADERS_FOLDER = "src/Shaders/";
	inline static std::string MFC_TEST_FRAME(unsigned int index, const char* folder) {
		const char* orientation[9] = {"normal",
									  "top",
									  "top right",
									  "right",
									  "bottom right",
									  "bottom",
									  "bottom left",
									  "left",
									  "top left"};
		std::string path("Resources/TestFrames/");
		path += folder;
		path += "/";
		path += orientation[index];
		path += ".png";
		return path;
	}
}