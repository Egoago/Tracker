#include "Object.h"
#include <string>
#include "opencv2/opencv.hpp"
//#include "opencv2/highgui/highgui.hpp"
#include <serializer.h>
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

void generate6DOFs(ConfigParser& config, vector<SixDOF>& sixDOFs) {
	//TODO tune granularity
	//TODO perspective distribution
	Range width(config.getEntries("width", { "-40.0", "40.0", "5" }));
	Range height(config.getEntries("height", { "-40.0", "40.0", "5" }));
	Range depth(config.getEntries("depth", { "-2000.0", "200.0", "5" }));
	//TODO uniform sphere distr <=> homogenous tensor layout????
	Range roll(config.getEntries("roll", { "0", "360", "3" }));
	Range yaw(config.getEntries("yaw", { "0", "360", "6" }));
	Range pitch(config.getEntries("pitch", { "0", "180", "3" }));
	float x, y, z, ya, p, r;
	x=y=z=ya=p=r=0.0f;
	for (r = roll.begin; r < roll.end; r+= roll.step)
	for (ya = yaw.begin; ya < yaw.end; ya+= yaw.step)
	for (p = pitch.begin; p < pitch.end; p+= pitch.step)
	for (y = height.begin; y < height.end; y+= height.step)
	for (x = width.begin; x < width.end; x+= width.step)
	for (z = depth.begin; z < depth.end; z+= depth.step)
	{
		SixDOF sixDOF;
		sixDOF.position.x = x;
		sixDOF.position.y = y;
		sixDOF.position.z = z;
		sixDOF.orientation.y = ya;
		sixDOF.orientation.p = p;
		sixDOF.orientation.r = r;
		sixDOFs.push_back(sixDOF);
	}
}

void Object::generarteObject(const string& fileName) {
	using namespace cv;

	generate6DOFs(config, sixDOFs);
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
	cout << "6dof count: " << sixDOFs.size() << endl;
	int c = 0;
	for (const SixDOF& sixDOF : sixDOFs) {
		cout << "Rendered " << c++ << "\t frames out of " << sixDOFs.size() << "\r";
		renderer.setModel(sixDOF);
		glm::mat4 mvp = renderer.renderModel(geo, depth.data, color.data);
		cv::flip(color, color, 0);
		cv::flip(depth, depth, 0);
		//imshow("OpenCV", color);
		Canny(color, detected_edges, 10, 10 * 3, 3);
		//imshow("Canny", detected_edges);
		dst = Scalar(0);
		std::vector<Edge> edges = detector.detectOutlinerEdges(detected_edges, dst, mvp);
		imshow("Wireframe", dst);
		Rasterizer rasterizer(edges, 4.0f, 0.5f);
		//break;
		waitKey(100);
	}
	ofstream os(LOADED_OBJECTS_FOLDER + objectName + ".data");
	os << "jaj";
	os.close();
}

Object::Object(string fileName)
{
	getObjectName(fileName, objectName);
	
	vector<string> loadedObjects = config.getEntries("loaded objects");
	if (loadedObjects.size() > 0 && find(loadedObjects.begin(), loadedObjects.end(), objectName) != loadedObjects.end())
		loadObject(objectName);
	else
		generarteObject(fileName);
}

