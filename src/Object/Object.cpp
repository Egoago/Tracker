#include "Object.h"
#include <string>
#include "opencv2/opencv.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include "../Misc/Links.h"
#include "AssimpGeometry.h"
#include "ModelEdgeDetector.h"
#include "../Rendering/Renderer.h"
#include "Rasterizer.h"

using namespace std;

ConfigParser Object::config(OBJ_CONFIG_FILE);

void getObjectName(string& fName, string& oName) {
	size_t dot = fName.find('.');
	if (dot == string::npos) {
		oName = fName;
		fName += ".STL";
	}
	else oName = fName.substr(0, dot);
}

void Object::loadObject(const string& objectName) {
	ifstream is(LOADED_OBJECTS_FOLDER + objectName + ".data");
	is >> bits(*this);
	is.close();
}

void Object::generate6DOFs() {
	//TODO tune granularity
	//TODO perspective distribution
	Range width(config.getEntries("width", { "-40.0", "40.0", "8" }));
	Range height(config.getEntries("height", { "-40.0", "40.0", "8" }));
	Range depth(config.getEntries("depth", { "-2000.0", "200.0", "8" }));
	//TODO uniform sphere distr <=> homogenous tensor layout????
	Range roll(config.getEntries("roll", { "0", "360", "4" }));
	Range yaw(config.getEntries("yaw", { "0", "360", "8" }));
	Range pitch(config.getEntries("pitch", { "0", "180", "4" }));
	snapshots.resize(boost::extents[yaw.resolution]
									[pitch.resolution]
									[roll.resolution]
									[width.resolution]
									[height.resolution]
									[depth.resolution]);
	for (size_t ya = 0; ya < yaw.resolution; ya++)
	for (size_t p = 0; p < pitch.resolution; p++)
	for (size_t r = 0; r < roll.resolution; r++)
	for (size_t x = 0; x < width.resolution; x++)
	for (size_t y = 0; y < height.resolution; y++)
	for (size_t z = 0; z < depth.resolution; z++)
	{
		SixDOF& sixDOF = snapshots[ya][p][r][x][y][z].sixDOF;
		sixDOF.position.x = width[x];
		sixDOF.position.y = height[y];
		sixDOF.position.z = depth[z];
		sixDOF.orientation.y = yaw[ya];
		sixDOF.orientation.p = pitch[p];
		sixDOF.orientation.r = roll[r];
	}
}

void Object::generarteObject(const string& fileName) {
	using namespace cv;
	generate6DOFs();
	Geometry geo = AssimpGeometry(fileName);
	ModelEdgeDetector detector(geo);
	Renderer renderer(500, 500);
	namedWindow("OpenCV", cv::WINDOW_NORMAL);
	resizeWindow("OpenCV", 500, 500);
	moveWindow("OpenCV", 0, 500);
	namedWindow("Canny", WINDOW_NORMAL);
	resizeWindow("Canny", 500, 500);
	moveWindow("Canny", 500, 500);
	namedWindow("Wireframe", WINDOW_NORMAL);
	resizeWindow("Wireframe", 500, 500);
	moveWindow("Wireframe", 1000, 500);
	Mat depth(Size(500, 500), CV_32F);
    Mat color(Size(500, 500), CV_8UC3);
    Mat dst(Size(500, 500), CV_8U, Scalar(0));
    Mat detected_edges(Size(500, 500), CV_8U);
	int c = 0;
	for (Snapshot* i = snapshots.data(); i < (snapshots.data() + snapshots.num_elements()); i++) {
		SixDOF& sixDOF = i->sixDOF;
		cout << "Rendered " << c++ << "\t frames out of " << snapshots.num_elements() << "\r";
		//sixDOF.print(cout);
		renderer.setModel(sixDOF);
		glm::mat4 mvp = renderer.renderModel(geo, depth.data, color.data);
		cv::flip(color, color, 0);
		cv::flip(depth, depth, 0);
		imshow("OpenCV", color);
		Canny(color, detected_edges, 10, 10 * 3, 3);
		imshow("Canny", detected_edges);
		dst = Scalar(0);
		std::vector<Edge> edges = detector.detectOutlinerEdges(detected_edges, dst, mvp);
		imshow("Wireframe", dst);
		Rasterizer rasterizer(edges, 4.0f, 0.5f);
		i->M = rasterizer.getM();
		i->M_ = rasterizer.getM_();
		//break;
		waitKey(1);
	}
	ofstream os(LOADED_OBJECTS_FOLDER + objectName + ".data");
	os << "jaj";
	os.close();
}


Object::Object(string fileName) : snapshots(Registry(boost::extents[0][0][0][0][0][0]))
{
	getObjectName(fileName, objectName);
	
	vector<string> loadedObjects = config.getEntries("loaded objects");
	if (loadedObjects.size() > 0 && find(loadedObjects.begin(), loadedObjects.end(), objectName) != loadedObjects.end())
		loadObject(objectName);
	else
		generarteObject(fileName);
}

