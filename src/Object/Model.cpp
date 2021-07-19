#include "Model.h"
#include <string>
#include <serializer.h>
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
	dimensions[0] = yaw.resolution;
	dimensions[1] = pitch.resolution;
	dimensions[2] = roll.resolution;
	dimensions[3] = width.resolution;
	dimensions[4] = height.resolution;
	dimensions[5] = depth.resolution;
	allocateRegistry();
	for (unsigned int ya = 0; ya < yaw.resolution; ya++)
	for (unsigned int p = 0; p < pitch.resolution; p++)
	for (unsigned int r = 0; r < roll.resolution; r++)
	for (unsigned int x = 0; x < width.resolution; x++)
	for (unsigned int y = 0; y < height.resolution; y++)
	for (unsigned int z = 0; z < depth.resolution; z++)
	{
		SixDOF& sixDOF = templates[ya][p][r][x][y][z].sixDOF;
		sixDOF.position.x = width[x];
		sixDOF.position.y = height[y];
		sixDOF.position.z = depth[z];
		sixDOF.orientation.y = yaw[ya];
		sixDOF.orientation.p = pitch[p];
		sixDOF.orientation.r = roll[r];
	}
}

cv::Point getResolution(ConfigParser& config) {
	vector<string> str = config.getEntries("frame resolution", { "1000", "1000" });
	return cv::Point(stoi(str[0]), stoi(str[1]));
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
	ModelEdgeDetector detector(geo);
	Point renderRes = getResolution(config);
	Point windowRes = renderRes/2;
	
	Renderer renderer(renderRes.x, renderRes.y);
	namedWindow("OpenCV", cv::WINDOW_NORMAL);
	resizeWindow("OpenCV", windowRes.x, windowRes.y);
	moveWindow("OpenCV", 0, windowRes.y);
	namedWindow("Canny", WINDOW_NORMAL);
	resizeWindow("Canny", windowRes.x, windowRes.y);
	moveWindow("Canny", windowRes.x, windowRes.y);
	namedWindow("Wireframe", WINDOW_NORMAL);
	resizeWindow("Wireframe", windowRes.x, windowRes.y);
	moveWindow("Wireframe", windowRes.x * 2, windowRes.y);
	Mat posMap(Size(renderRes.x, renderRes.y), CV_32FC3);
    Mat indexMap(Size(renderRes.x, renderRes.y), CV_32S);
    Mat normalMap(Size(renderRes.x, renderRes.y), CV_8UC3);
    Mat dst(Size(renderRes.x, renderRes.y), CV_8U, Scalar(0));
    Mat detected_edges(Size(renderRes.x, renderRes.y), CV_8U);
	int c = 1;
	for (Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++) {
		SixDOF& sixDOF = i->sixDOF;
		//TODO remove monitoring
		cout << "Rendered " << c++ << "\t frames out of " << templates.num_elements() << "\r";
		//sixDOF.print(cout);
		renderer.setModel(sixDOF);
		glm::mat4 mvp = renderer.renderModel(geo, posMap.data, indexMap.data);
		cv::flip(posMap, posMap, 0);
		cv::flip(indexMap, indexMap, 0);
		normalize(posMap, posMap, 0, 1, NORM_MINMAX);
		imshow("OpenCV", posMap);
		//imshow("Canny", indexMap);
		/*Canny(color, detected_edges, 10, 10 * 3, 3);
		imshow("Canny", detected_edges);
		dst = Scalar(0);
		vector<Edge<>> edges = detector.detectOutlinerEdges(detected_edges, dst, mvp);
		imshow("Wireframe", dst);*/
		waitKey(1000);
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
