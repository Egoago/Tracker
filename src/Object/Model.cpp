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
using namespace cv;
using namespace tr;

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
	tr::Range width(config.getEntries("width", { "-40.0", "40.0", "4" }));
	tr::Range height(config.getEntries("height", { "-40.0", "40.0", "4" }));
	tr::Range depth(config.getEntries("depth", { "-2000.0", "-300.0", "2" }));
	//TODO uniform sphere distr <=> homogenous tensor layout????
	tr::Range roll(config.getEntries("roll", { "0", "360", "4" }));
	tr::Range yaw(config.getEntries("yaw", { "0", "360", "4" }));
	tr::Range pitch(config.getEntries("pitch", { "0", "180", "2" }));
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
void drawOnFrame(const glm::vec3& p, Mat& posMap, const glm::mat4& mvp, Scalar color) {
	glm::vec4 pPos = mvp * glm::vec4(p, 1.0f);
	pPos /= pPos.w;
	pPos = (pPos + 1.0f) / 2.0f;
	Point pixel((int)(pPos.x * (float)posMap.cols +0.5f),
		(int)(pPos.y * (float)posMap.rows + 0.5f));
	circle(posMap, pixel, 1, color, -1);
}

void floatToBinary(const Mat& floatMap, Mat& binary) {
	absdiff(floatMap, Scalar::all(0), binary);
	switch (binary.channels()) {
		case 3: transform(binary, binary, Matx13f(1.0, 1.0, 1.0)); break;
		case 2: transform(binary, binary, Matx12f(1.0, 1.0)); break;
		case 4: transform(binary, binary, Matx14f(1.0, 1.0, 1.0, 1.0)); break;
	}
	threshold(binary, binary, 1e-10, 255, THRESH_BINARY);
	binary.convertTo(binary, CV_8U);
}

void getContour(const Mat& lowDirMap, Mat& contourMap) {
	floatToBinary(lowDirMap, contourMap);
	vector<vector<Point> > contours;
	findContours(contourMap, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	contourMap = Scalar::all(0);
	drawContours(contourMap, contours, 0, Scalar(255, 255, 255));
}

void Model::extractCandidates(const glm::mat4& MVP) {

	Mat contourMap;
	getContour(*textureMaps[LDIR], contourMap);

	Mat curvatureMap;
	floatToBinary(highDirMap, curvatureMap);


	mutex mtx;
	textureMaps[HPOS]->forEach<Point3f>(
		[&textureMaps = textureMaps, MVP, &mtx, &candidates = candidates]
		(Point3f& pixel, const int* p) -> void {
			//test mask
			if (pixel < 1) return;

			//test pos
			Point3f pos = posMap.at<Point3f>(p[0], p[1]);
			if (pos.dot(pos) < 1e-5) return;

			//test offsetPos
			Point3f offsetPos = offsetPosMap.at<Point3f>(p[0], p[1]);
			//if (offsetPos.dot(offsetPos) < 1e-3) return;

			//TODO remove		|
			// this is heavy	V
			
			//test mask+dir
			/*glm::vec4 p2 = MVP * glm::vec4( offsetPos.x,
											offsetPos.y,
											offsetPos.z, 1.0f);
			p2 /= p2.w;
			p2 = (p2 + 1.0f) / 2.0f;

			const int offsetX = (int)(p2.x * posMap.cols + 0.5f);
			const int offsetY = (int)(p2.y * posMap.rows + 0.5f);

			uchar offsetPixel = maskMap.at<uchar>(offsetX, offsetY);
			if (offsetPixel < 1) return;*/

			//register
			mtx.lock();
			templ->pos.emplace_back(pos.x, pos.y, pos.z);
			templ->offsetPos.emplace_back(offsetPos.x, offsetPos.y, offsetPos.z);
			mtx.unlock();
		});
}

void Model::rasterizeCandidates(Template* temp)
{
	const static float rasterProb = stof(config.getEntry("rasterization probability", "0.1"));
	const static float rasterOffset = stof(config.getEntry("rasterization offset", "1.0"));
	
	const static std::mt19937 gen(std::random_device{}());
	const static std::bernoulli_distribution dist(rasterProb);
	std::vector<bool> mask(candidates.size());
	std::generate(mask.begin(), mask.end(), [&] { return dist(gen); });
	for(unsigned int i = 0; i < mask.size(); i++)
		if (mask[i]) {
			temp->pos.push_back(candidates[i].pos);
			//TODO check dir vector length
			temp->offsetPos.push_back(candidates[i].pos + candidates[i].dir * rasterOffset);
		}
}

void Model::generarteObject(const string& fileName) {
	//TODO add loading bar
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
		renderer.render(textureMaps);

		extractCandidates(renderer.getMVP());
		rasterizeCandidates(i);

		waitKey(10000);
		//TODO remove logging
		cout << "samples: " << i->pos.size() << endl;

		//for (unsigned int x = 0; x < i->pos.size(); x++) {
		//	drawOnFrame(i->pos[x], posMap, mvp, Scalar(255,0,0));
		//	drawOnFrame(i->offsetPos[x], posMap, mvp, Scalar(0,255,0));
		//	drawOnFrame(i->pos[x], offsetPosMap, mvp, Scalar(255,0,0));
		//	glm::vec4 p2 = mvp * glm::vec4( i->offsetPos[x].x,
		//									i->offsetPos[x].y,
		//									i->offsetPos[x].z, 1.0f);
		//	p2 /= p2.w;
		//	p2 = (p2 + 1.0f) / 2.0f;

		//	const int offsetX = (int)(p2.x * posMap.cols + 0.5f);
		//	const int offsetY = (int)(p2.y * posMap.rows + 0.5f);

		//	//uchar offsetPixel = maskMap.at<uchar>(offsetX, offsetY);
		//	circle(offsetPosMap, Point(offsetX, offsetY), 1, Scalar(0,255,0), -1);
		//}
		//imshow("Offset Pos", offsetPosMap);
		//imshow("Pos", posMap);
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

void Model::save(string fileName) {
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
