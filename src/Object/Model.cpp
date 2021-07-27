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
#include <random>

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

void Model::rasterize(const cv::Mat& maskMap,
					const cv::Mat& posMap,
					const cv::Mat& dirMap,
					Template* templ) {
	//TODO remove logging
	std::cout << "rasterizing\n";
	std::mutex mtx;
	const static float rasterProb = stof(config.getEntry("rasterization probability", "0.1"));
	maskMap.forEach<uchar>(
		[&mtx, templ, &dirMap, &posMap](uchar& pixel, const int* p) -> void {
			//test mask
			if (pixel < 128) return;

			//test pos
			cv::Point3f pos = posMap.at<cv::Point3f>(p[0], p[1]);
			if (pos.dot(pos) < 1e-3) return;

			//test dir
			cv::Point3f dir = dirMap.at<cv::Point3f>(p[0], p[1]);
			if (dir.dot(dir) < 1e-3) return;
			
			//rasterization
			static thread_local std::random_device rd;
			static thread_local std::mt19937 gen(rd());
			std::bernoulli_distribution bern(rasterProb);
			if(!bern(gen)) return;

			//register
			mtx.lock();
			templ->pos.emplace_back(pos.x, pos.y, pos.z);
			templ->dir.emplace_back(dir.x, dir.y, dir.z);
			mtx.unlock();
		});
}


void createMask(const cv::Mat& dirMap, cv::Mat& maskMap) {
	//TODO remove logging
	std::cout << "creating mask\n";
	//TODO simplify xyz->binary transformation
	// - use OpenGL instead of OpenCV
	using namespace cv;
	static Mat sum(Size(maskMap.cols, maskMap.rows), CV_32F);
	static Mat binary(Size(maskMap.cols, maskMap.rows), CV_8U);
	absdiff(dirMap, Scalar::all(0), sum);
	transform(sum, sum, cv::Matx13f(1.0, 1.0, 1.0));
	threshold(sum, binary, 1e-5, 255, THRESH_BINARY);
	binary.convertTo(binary, CV_8U);

	vector<vector<Point> > contours;
	findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	drawContours(maskMap, contours, 0, Scalar(255, 255, 255));
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
	int c = 1;
	for (Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++) {
		//TODO use CUDA with OpenGL directly on GPU
		// instead moving textures to CPU and using OpenCV
		// OpenMP??

		//TODO remove monitoring
		cout << "Rendered " << c++ << "\t frames out of " << templates.num_elements() << "\r";
		i->sixDOF.print(cout);
		renderer.setModel(i->sixDOF);
		//TODO remove logging
		std::cout << "rendering\n";
		std::vector<cv::Mat*> textureMaps = renderer.render();
		Mat& maskMap = *textureMaps.at(0);
		Mat& posMap = *textureMaps.at(1);
		Mat& dirMap = *textureMaps.at(2);

		createMask(dirMap, maskMap);

		imshow("Mask", maskMap);
		imshow("Pos", posMap);
		imshow("Directions", dirMap);
		//TODO remove logging
		std::cout << "waiting for key...\n";
		waitKey(10000);
		rasterize(maskMap, posMap, dirMap, i);
		//TODO remove logging
		std::cout << "samples: " << i->pos.size() << std::endl;;
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
