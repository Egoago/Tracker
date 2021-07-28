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
	Mat tmp;
	floatToBinary(lowDirMap, tmp);
	vector<vector<Point> > contours;
	findContours(tmp, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	drawContours(contourMap, contours, 0, Scalar(255, 255, 255));
}

void Model::extractCandidates() {
	candidates.clear();
	Mat maskMap;
	floatToBinary(*textureMaps[HDIR], maskMap);
	getContour(*textureMaps[LDIR], maskMap);
	//TODO remove
	imshow("Mask", maskMap);
	mutex mtx;
	maskMap.forEach<uchar>(
		[&textureMaps = textureMaps, &candidates = candidates, &mtx]
		(uchar& pixel, const int* p) -> void {
			if (pixel == 0) return;

			//test high thres
			Point3f pos = textureMaps[HPOS]->at<Point3f>(p[0], p[1]);
			Point3f dir = textureMaps[HDIR]->at<Point3f>(p[0], p[1]);
			if (pos.dot(pos) > 1e-5 && dir.dot(dir) > 1e-5) {
				mtx.lock();
				candidates.push_back(Candidate(pos, dir));
				mtx.unlock();
				return;
			}
			//test low thres
			pos = textureMaps[LPOS]->at<Point3f>(p[0], p[1]);
			dir = textureMaps[LDIR]->at<Point3f>(p[0], p[1]);
			if (pos.dot(pos) > 1e-5 && dir.dot(dir) > 1e-5) {
				mtx.lock();
				candidates.push_back(Candidate(pos, dir));
				mtx.unlock();
			}
		});
}

void Model::rasterizeCandidates(Template* temp)
{
	//TODO scale rastProb based on candidate count
	const static float rasterProb = stof(config.getEntry("rasterization probability", "0.1"));
	const static float rasterOffset = stof(config.getEntry("rasterization offset", "1.0"));
	
	static std::mt19937 gen(std::random_device{}());
	static std::bernoulli_distribution dist(rasterProb);
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
	namedWindow("Candidates", WINDOW_NORMAL);
	resizeWindow("Candidates", windowRes.x, windowRes.y);
	moveWindow("Candidates", windowRes.x * 2, 50);
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

		extractCandidates();
		cout << "candidates: " << candidates.size() << endl;
		rasterizeCandidates(i);
		cout << "rasterized: " << i->pos.size() << endl;

		imshow("Pos", *textureMaps[LPOS]);
		//TODO remove logging
		for (unsigned int x = 0; x < i->pos.size(); x++)
			drawOnFrame(i->pos[x], *textureMaps[LDIR], renderer.getMVP(), Scalar(255,0,0));
		for (unsigned int x = 0; x < i->pos.size(); x++)
			drawOnFrame(i->offsetPos[x], *textureMaps[LDIR], renderer.getMVP(), Scalar(0,255,0));
		imshow("Candidates", *textureMaps[LDIR]);

		waitKey(10000);
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
