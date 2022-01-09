#pragma once
#include <string>

namespace tr {
	//	CONFIG
	static const std::string CONFIG_FILE = "config.ini";
	static const std::string CONFIG_SECTION_LABEL = "SECTION";
	static const std::string CONFIG_TABULATOR = "  ";
	static const std::string CONFIG_DELIMITER = "=";
	static const std::string CONFIG_SUB_DELIMITER = ";";
	static const std::string CONFIG_SECTION_RENDER = "Render";
	static const std::string CONFIG_SECTION_GEOMETRY = "Geometry";
	static const std::string CONFIG_SECTION_DCD3T = "DCD3T";
	static const std::string CONFIG_SECTION_MODEL = "Model";
	static const std::string CONFIG_SECTION_REGISTRATION = "Registration";
	static const std::string CONFIG_SECTION_ESTIMATION = "Estimation";
	static const std::string CONFIG_SECTION_CAMERA = "Camera";

	//  MODEL
	static const std::string MODELS_FOLDER = "Resources/Models/";
	static const std::string MODELS_EXTENSION = ".stl";
	static const std::string LOADED_OBJECTS_FOLDER = "Resources/Models/Loaded/";
	static const std::string LOADED_OBJECT_FILENAME_EXTENSION = ".dat";

	//  SHADER
	static const std::string SHADERS_FOLDER = "src/Shaders/";
	static const std::string VERTEX_SHADER_EXTENSION = ".vert";
	static const std::string FRAGMENT_SHADER_EXTENSION = ".frag";
	
	//  TEST
	static const std::string TEST_FRAME_CUBE_WEBCAM = "Resources/TestFrames/testFrame.jpg";
	static const std::string TEST_FRAME_CUBE = "Resources/TestFrames/cube_noise.jpg";
	static const std::string TEST_FRAME_TRIANGLE = "Resources/TestFrames/triangle.png";
	static const std::string TEST_FRAME_CYLINDER = "Resources/TestFrames/cylinder.png";
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