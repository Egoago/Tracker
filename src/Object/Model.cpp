#include "Model.h"
#include <string>
#include <serializer.h>
#include <mutex>
//TODO clear includes
#include "opencv2/opencv.hpp"
#include "../Misc/Links.h"
#include "AssimpGeometry.h"
#include "ModelEdgeDetector.h"
#include "../Rendering/Renderer.h"

using namespace std;

ConfigParser Model::config(OBJ_CONFIG_FILE);

void getObjectName(string& fName, string& oName) {
	size_t dot = fName.find('.');
	if (dot == string::npos) {
		oName = fName;
		fName += ".STL";
	}
	else oName = fName.substr(0, dot);
}

void Model::allocateRegistry() {
	templates.resize(boost::extents[dimensions[0]]
									[dimensions[1]]
									[dimensions[2]]
									[dimensions[3]]
									[dimensions[4]]
									[dimensions[5]]);
}

void Model::generate6DOFs() {
	//TODO tune granularity
	//TODO perspective distribution
	Range width(config.getEntries("width", { "-40.0", "40.0", "4" }));
	Range height(config.getEntries("height", { "-40.0", "40.0", "4" }));
	Range depth(config.getEntries("depth", { "-2000.0", "-300.0", "2" }));
	//TODO uniform sphere distr <=> homogenous tensor layout????
	Range roll(config.getEntries("roll", { "0", "360", "4" }));
	Range yaw(config.getEntries("yaw", { "0", "360", "4" }));
	Range pitch(config.getEntries("pitch", { "0", "180", "2" }));
	dimensions[0] = width.resolution;
	dimensions[1] = height.resolution;
	dimensions[2] = depth.resolution;
	dimensions[3] = yaw.resolution;
	dimensions[4] = pitch.resolution;
	dimensions[5] = roll.resolution;
	allocateRegistry();
	for (unsigned int x = 0; x < width.resolution; x++)
	for (unsigned int y = 0; y < height.resolution; y++)
	for (unsigned int z = 0; z < depth.resolution; z++)
	for (unsigned int ya = 0; ya < yaw.resolution; ya++)
	for (unsigned int p = 0; p < pitch.resolution; p++)
	for (unsigned int r = 0; r < roll.resolution; r++)
	{
		SixDOF& sixDOF = templates[x][y][z][ya][p][r].sixDOF;
		sixDOF.position.x = width[x];
		sixDOF.position.y = height[y];
		sixDOF.position.z = depth[z];
		sixDOF.orientation.y = yaw[ya];
		sixDOF.orientation.p = pitch[p];
		sixDOF.orientation.r = roll[r];
	}
}

void Model::rasterize(const vector<Edge<>>& edges, Template* snapshot) {
	const static float step = stof(config.getEntry("rasterization step", "2.0"));
	const static float d = stof(config.getEntry("rasterization offset", "0.5"));
	for (const Edge<>& edge : edges)
	{
		glm::vec3 dir = glm::normalize(edge.b - edge.a);
		float dist = glm::distance(edge.a, edge.b);
		glm::vec3 p = edge.a;
		while (dist > 0) {
			snapshot->M.push_back(p);
			snapshot->M_.push_back(p + d * dir);
			p += step * dir;
			dist -= step;
		}
	}
}

void Model::generarteObject(const string& fileName) {
	//TODO add loading bar
	using namespace cv;
	generate6DOFs();
	Geometry geo = AssimpGeometry(fileName);
	Renderer renderer(geo);
	glm::uvec2 renderRes = renderer.getResolution();
	glm::uvec2 windowRes = renderRes / glm::uvec2(3,3);
	
	//TODO remove monitoring
	namedWindow("Mask", WINDOW_NORMAL);
	resizeWindow("Mask", windowRes.x, windowRes.y);
	moveWindow("Mask", 0, 50);
	namedWindow("Pos", WINDOW_NORMAL);
	resizeWindow("Pos", windowRes.x, windowRes.y);
	moveWindow("Pos", windowRes.x, 50);
	namedWindow("Directions", WINDOW_NORMAL);
	resizeWindow("Directions", windowRes.x, windowRes.y);
	moveWindow("Directions", windowRes.x * 2, 50);
	Mat posSum(Size(renderRes.x, renderRes.y), CV_32F);
	Mat out(Size(renderRes.x, renderRes.y), CV_8U);
	int c = 1;
	for (Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++) {
		//TODO use CUDA with OpenGL directly on GPU
		// instead moving textures to CPU and using OpenCV
		//TODO remove monitoring
		cout << "Rendered " << c++ << "\t frames out of " << templates.num_elements() << "\r";
		//i->sixDOF.print(cout);
		renderer.setModel(i->sixDOF);
		std::vector<cv::Mat*> textureMaps = renderer.render();
		Mat& maskMap = *textureMaps.at(0);
		Mat& posMap = *textureMaps.at(1);
		Mat& dirMap = *textureMaps.at(2);
		posMap.convertTo(posMap, CV_32FC3, 1.0 / 32.5, 0.0);

		//TODO simplify xyz->binary transformation
		// - use OpenGL instead of OpenCV
		transform(posMap, posSum, cv::Matx13f(0.5, 0.5, 0.5));
		threshold(posSum, out, 1e-3, 255, THRESH_BINARY);
		out.convertTo(out, CV_8U);
		vector<vector<Point> > contours;
		findContours(out, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		imshow("Mask", maskMap);
		out = Scalar::all(0);
		drawContours(out, contours, 0, Scalar(255, 255, 255), 1, LINE_8, noArray(), 1, Point(1,1));

		//maskMap += out;
		std::vector<cv::Mat> channels;
		Mat alpha, rgba;
		//maskMap.convertTo(maskMap, CV_32F, 1/2.0);
		posMap.convertTo(posMap, CV_32FC3, 1/2.0);
		split(posMap, channels);
		channels.at(2) += maskMap;
		merge(channels, posMap);
		
		imshow("Pos", posMap);
		imshow("Directions", dirMap);

		maskMap.convertTo(maskMap, CV_8U);
		posMap.convertTo(posMap, CV_32FC3);

		/////////////////////////
		waitKey(30000);
		//rasterize(edges, i);
	}
	cout << endl;
}

Model::Model(string fileName)
{
	getObjectName(fileName, objectName);
	
	vector<string> loadedObjects = config.getEntries("loaded objects");
	if (loadedObjects.size() > 0 && find(loadedObjects.begin(), loadedObjects.end(), objectName) != loadedObjects.end())
		load();
	else {
		generarteObject(fileName);
		config.getEntries("loaded objects").push_back(objectName);
		config.save();
		save();
	}
}

string getSavePath(const string& objectName) {
	return LOADED_OBJECTS_FOLDER + objectName + ".txt";
}

void Model::save(std::string fileName) {
	if (fileName.empty())
		fileName = getSavePath(objectName);
	ofstream out(fileName);
	for (int i = 0; i < templates.dimensionality; i++)
		out << bits(dimensions[i]);
	for (Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++)
		out << bits(*i);
	out.close();
}

void Model::load() {
	ifstream in(getSavePath(objectName));
	for (int i = 0; i < templates.dimensionality; i++)
		in >> bits(dimensions[i]);
	allocateRegistry();
	for (Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++)
		in >> bits(*i);
	in.close();
}
