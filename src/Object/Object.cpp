#include <string>
#include "Object.h"
#include <serializer.h>
#include "../Misc/Links.cpp"
#include "Coordinates.h"

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

}

void Object::generarteObject(const string& fileName) {
	vector<SixDOF> sixDOFs;
	generate6DOFs(config, sixDOFs);
	config.getEntries("loaded objects").push_back(objectName);
	config.save();
	ofstream os(LOADED_OBJECTS_FOLDER + fileName);
	os << "jaj";
	os.close();
	//ModelEdgeDetector detector;
	//        detector.detectOutlinerEdges(*geo);sss*/
	//        Renderer renderer(argc, argv, 500, 500);
	//        float rot = 90.0f;
	//        namedWindow("OpenCV", WINDOW_NORMAL);
	//        resizeWindow("OpenCV", 500, 500);
	//        moveWindow("OpenCV", 0, 500);
	//        namedWindow("Canny", WINDOW_NORMAL);
	//        resizeWindow("Canny", 500, 500);
	//        moveWindow("Canny", 500, 500);
	//        namedWindow("Wireframe", WINDOW_NORMAL);
	//        resizeWindow("Wireframe", 500, 500);
	//        moveWindow("Wireframe", 1000, 500);
	//        Mat depth(Size(500, 500), CV_32F);
	//        Mat color(Size(500, 500), CV_8UC3);
	//        Mat dst(Size(500, 500), CV_8U, Scalar(0));
	//        Mat detected_edges(Size(500, 500), CV_8U);
	//        Geometry geometry = AssimpGeometry("cube.STL");
	//        ModelEdgeDetector detector(geometry);
	//        while (1) {
	//            renderer.setModel(-32.5f / 2.0f, 0.0f, -150.0f, 0.0f, rot);
	//            glm::mat4 mvp = renderer.renderModel(geometry, depth.data, color.data);
	//            cv::flip(color, color, 0);
	//            cv::flip(depth, depth, 0);
	//            Canny(color, detected_edges, 10, 10 * 3, 3);
	//            imshow("Canny", detected_edges);
	//            dst = Scalar(0);
	//            std::vector<Edge> edges = detector.detectOutlinerEdges(detected_edges, dst, mvp);
	//            imshow("Wireframe", dst);
	//            Rasterizer rasterizer(edges, 4.0f, 0.5f);
	//            rot += 0.01f;
	//            //break;
	//            waitKey(1);
	//        }
	//        /*Camera* cam = new OpenCVCamera();
	//        namedWindow("Original", WINDOW_NORMAL);
	//        resizeWindow("Original", cam->getWidth() / 2, cam->getHeight() / 2);
	//        moveWindow("Original", 50, 50);
	//        namedWindow("Canny", WINDOW_NORMAL);
	//        resizeWindow("Canny", cam->getWidth()/2, cam->getHeight()/2);
	//        moveWindow("Canny", cam->getWidth() / 2+50, 50);
	//        while (1)
	//        {
	//            char* frame = cam->getNextFrame();
	//            Mat wrapped(Size(cam->getWidth(), cam->getHeight()), cam->getFormat(), frame);
	//            cout <<"\r"<<cam->getFPS();
	//            imshow("Original", wrapped);
	//            Mat dst, detected_edges;
	//            blur(wrapped, detected_edges, Size(5, 5));
	//            Canny(wrapped, detected_edges, 200, 30 * 3, 3);
	//            dst = Scalar::all(0);
	//            wrapped.copyTo(dst, detected_edges);
	//            imshow("Canny", dst);
	//            waitKey(1);
	//        }
	//        delete cam;*/
	//        return 0;
}

Object::Object(string fileName)
{
	getObjectName(fileName, objectName);
	
	vector<string> loadedObjects = config.getEntries("loaded objects");
	if (loadedObjects.size() > 0 && find(loadedObjects.begin(), loadedObjects.end(), objectName) != loadedObjects.end())
		loadObject(objectName);
	else
		generarteObject(fileName);
//#include "opencv2/opencv.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "Camera/OpenCVCamera.h"
//#include "Rendering/Renderer.h"
//#include "Object/AssimpGeometry.h"
//#include "Object/ModelEdgeDetector.h"
//#include <glm/mat4x4.hpp>
//#include "Object/Rasterizer.h"
//
//    using namespace cv;
//    using namespace std;
//
//    glm::vec2 screenToNDC2(glm::vec4 screen) {
//        screen = screen / screen.w;
//        //screen = screen / screen.z;
//        screen = (screen + 1.0f) / 2.0f;
//        return glm::vec2(screen.x, 1.0f - screen.y);
//    }
//
//
//    int main(int argc, char** argv) {
//        /*Geometry* geo = new AssimpGeometry("cube.STL");
//        
//    }
}

