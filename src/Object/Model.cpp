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

void Model::extractValidPoints(const cv::Mat& maskMap,
					const cv::Mat& posMap,
					const cv::Mat& offsetPosMap,
					Template* templ,
					const glm::mat4 MVP) {
	std::mutex mtx;
	const static float rasterProb = stof(config.getEntry("rasterization probability", "0.1"));
	maskMap.forEach<uchar>(
		[maskMap, MVP, &mtx, templ, &offsetPosMap, &posMap](uchar& pixel, const int* p) -> void {
			//test mask
			if (pixel < 1) return;

			//test pos
			cv::Point3f pos = posMap.at<cv::Point3f>(p[0], p[1]);
			if (pos.dot(pos) < 1e-5) return;

			//test offsetPos
			cv::Point3f offsetPos = offsetPosMap.at<cv::Point3f>(p[0], p[1]);
			//if (offsetPos.dot(offsetPos) < 1e-3) return;

			//TODO remove		|
			// this is heavy	V
			
			//test mask+dir
			glm::vec4 p2 = MVP * glm::vec4( offsetPos.x,
											offsetPos.y,
											offsetPos.z, 1.0f);
			p2 /= p2.w;
			p2 = (p2 + 1.0f) / 2.0f;

			const int offsetX = (int)(p2.x * posMap.cols + 0.5f);
			const int offsetY = (int)(p2.y * posMap.rows + 0.5f);

			uchar offsetPixel = maskMap.at<uchar>(offsetX, offsetY);
			if (offsetPixel < 1) return;

			//register
			mtx.lock();
			templ->pos.emplace_back(pos.x, pos.y, pos.z);
			templ->offsetPos.emplace_back(offsetPos.x, offsetPos.y, offsetPos.z);
			mtx.unlock();
		});
}


void createMask(const cv::Mat& posMap, cv::Mat& maskMap) {
	//TODO simplify xyz->binary transformation
	// - use OpenGL instead of OpenCV
	using namespace cv;
	static Mat sum(Size(maskMap.cols, maskMap.rows), CV_32F);
	static Mat binary(Size(maskMap.cols, maskMap.rows), CV_8U);
	absdiff(posMap, Scalar::all(0), sum);
	transform(sum, sum, cv::Matx13f(1.0, 1.0, 1.0));
	threshold(sum, binary, 1e-10, 255, THRESH_BINARY);
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

//TODO remove monitoring
void drawOnFrame(const glm::vec3& p, cv::Mat& posMap, const glm::mat4& mvp, cv::Scalar color) {
	glm::vec4 pPos = mvp * glm::vec4(p, 1.0f);
	pPos /= pPos.w;
	pPos = (pPos + 1.0f) / 2.0f;
	cv::Point pixel((int)(pPos.x * (float)posMap.cols +0.5f),
		(int)(pPos.y * (float)posMap.rows + 0.5f));
	cv::circle(posMap, pixel, 1, color, -1);
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
	namedWindow("Offset Pos", WINDOW_NORMAL);
	resizeWindow("Offset Pos", windowRes.x, windowRes.y);
	moveWindow("Offset Pos", windowRes.x * 2, 50);
	int c = 1;
	for (Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++) {
		//TODO use CUDA with OpenGL directly on GPU
		// instead moving textures to CPU and using OpenCV
		// OpenMP??

		//TODO remove monitoring
		//cout << "Rendered " << c++ << "\t frames out of " << templates.num_elements() << "\r";
		//i->sixDOF.print(cout);
		renderer.setModel(i->sixDOF);
		std::vector<cv::Mat*> textureMaps = renderer.render();
		Mat& maskMap = *textureMaps.at(0);
		Mat& posMap = *textureMaps.at(1);
		Mat& offsetPosMap = *textureMaps.at(2);

		createMask(posMap, maskMap);

		imshow("Mask", maskMap);
		waitKey(10000);
		auto mvp = renderer.getMVP();
		extractValidPoints(maskMap, posMap, offsetPosMap, i, mvp);
		//TODO remove logging
		std::cout << "samples: " << i->pos.size() << std::endl;

		for (unsigned int x = 0; x < i->pos.size(); x++) {
			drawOnFrame(i->pos[x], posMap, mvp, Scalar(255,0,0));
			drawOnFrame(i->offsetPos[x], posMap, mvp, Scalar(0,255,0));
			drawOnFrame(i->pos[x], offsetPosMap, mvp, Scalar(255,0,0));
			glm::vec4 p2 = mvp * glm::vec4( i->offsetPos[x].x,
											i->offsetPos[x].y,
											i->offsetPos[x].z, 1.0f);
			p2 /= p2.w;
			p2 = (p2 + 1.0f) / 2.0f;

			const int offsetX = (int)(p2.x * posMap.cols + 0.5f);
			const int offsetY = (int)(p2.y * posMap.rows + 0.5f);

			//uchar offsetPixel = maskMap.at<uchar>(offsetX, offsetY);
			cv::circle(offsetPosMap, cv::Point(offsetX, offsetY), 1, Scalar(0,255,0), -1);
		}
		imshow("Offset Pos", offsetPosMap);
		imshow("Pos", posMap);
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
