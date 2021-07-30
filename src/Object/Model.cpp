#include "Model.h"
#include <mutex>
#include <random>
#include <opencv2/imgproc.hpp>
//TODO clear includes
#include "../Misc/Links.h"
#include "../Rendering/Renderer.h"
//TODO remove logging
#include "../Misc/Log.h"
#include "AssimpGeometry.h"

//TODO break apart
using namespace tr;

ConfigParser Model::config(OBJ_CONFIG_FILE);

void getObjectName(std::string& fName, std::string& oName) {
	size_t dot = fName.find('.');
	if (dot == std::string::npos) {
		oName = fName;
		fName += ".STL";
	}
	else oName = fName.substr(0, dot);
}

void Model::generate6DOFs() {
	//TODO remove logging
	Logger::logProcess("generate6DOFs");
	//TODO tune granularity
	//TODO perspective distribution
	Range width(config.getEntries("width", { "-40.0", "40.0", "4" }));
	Range height(config.getEntries("height", { "-40.0", "40.0", "4" }));
	Range depth(config.getEntries("depth", { "-2000.0", "-300.0", "2" }));
	//TODO uniform sphere distr <=> homogenous tensor layout????
	Range roll(config.getEntries("roll", { "0", "360", "4" }));
	Range yaw(config.getEntries("yaw", { "0", "360", "4" }));
	Range pitch(config.getEntries("pitch", { "0", "180", "2" }));
	templates.allocate({
		width.resolution,
		height.resolution,
		depth.resolution,
		yaw.resolution,
		pitch.resolution,
		roll.resolution });
	for (unsigned int x = 0; x < width.resolution; x++)
	for (unsigned int y = 0; y < height.resolution; y++)
	for (unsigned int z = 0; z < depth.resolution; z++)
	for (unsigned int ya = 0; ya < yaw.resolution; ya++)
	for (unsigned int p = 0; p < pitch.resolution; p++)
	for (unsigned int r = 0; r < roll.resolution; r++)
	{
		SixDOF& sixDOF = templates.at({x,y,z,ya,p,r}).sixDOF;
		sixDOF.position.x = width[x];
		sixDOF.position.y = height[y];
		sixDOF.position.z = depth[z];
		sixDOF.orientation.y = yaw[ya];
		sixDOF.orientation.p = pitch[p];
		sixDOF.orientation.r = roll[r];
	}
	//TODO remove logging
	Logger::logProcess("generate6DOFs");
}

//TODO remove monitoring
void drawOnFrame(const glm::vec3& p, cv::Mat& posMap, const glm::mat4& mvp, cv::Scalar color) {
	glm::vec4 pPos = mvp * glm::vec4(p, 1.0f);
	pPos /= pPos.w;
	pPos = (pPos + 1.0f) / 2.0f;
	cv::Point pixel((int)(pPos.x * (float)posMap.cols +0.5f),
		(int)(pPos.y * (float)posMap.rows + 0.5f));
	circle(posMap, pixel, 1, color, -1);
}

void floatToBinary(const cv::Mat& floatMap, cv::Mat& binary) {
	cv::absdiff(floatMap, cv::Scalar::all(0), binary);
	switch (binary.channels()) {
		case 3: cv::transform(binary, binary, cv::Matx13f(1.0, 1.0, 1.0)); break;
		case 2: cv::transform(binary, binary, cv::Matx12f(1.0, 1.0)); break;
		case 4: cv::transform(binary, binary, cv::Matx14f(1.0, 1.0, 1.0, 1.0)); break;
	}
	cv::threshold(binary, binary, 1e-10, 255, cv::THRESH_BINARY);
	binary.convertTo(binary, CV_8U);
}

void getContour(const cv::Mat& lowDirMap, cv::Mat& contourMap) {
	cv::Mat tmp;
	floatToBinary(lowDirMap, tmp);
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(tmp, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	cv::drawContours(contourMap, contours, 0, cv::Scalar(255, 255, 255));
}

void Model::extractCandidates() {
	candidates.clear();
	cv::Mat maskMap;
	floatToBinary(*textureMaps[HDIR], maskMap);
	getContour(*textureMaps[LDIR], maskMap);
	std::mutex mtx;
	maskMap.forEach<uchar>(
		[&textureMaps = textureMaps, &candidates = candidates, &mtx]
		(uchar& pixel, const int* p) -> void {
			if (pixel == 0) return;

			//test high thres
			cv::Point3f pos = textureMaps[HPOS]->at<cv::Point3f>(p[0], p[1]);
			cv::Point3f dir = textureMaps[HDIR]->at<cv::Point3f>(p[0], p[1]);
			if (pos.dot(pos) > 1e-5 && dir.dot(dir) > 1e-5) {
				mtx.lock();
				candidates.push_back(Candidate(pos, dir));
				mtx.unlock();
				return;
			}
			//test low thres
			pos = textureMaps[LPOS]->at<cv::Point3f>(p[0], p[1]);
			dir = textureMaps[LDIR]->at<cv::Point3f>(p[0], p[1]);
			if (pos.dot(pos) > 1e-5 && dir.dot(dir) > 1e-5) {
				mtx.lock();
				candidates.push_back(Candidate(pos, dir));
				mtx.unlock();
			}
		});
}

void Model::rasterizeCandidates(Template* temp) {
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

void Model::generarteObject(const std::string& fileName) {
	//TODO proper resource destruction
	//TODO remove logging
	Logger::logProcess("generarteObject");
	//TODO add loading bar
	generate6DOFs();
	Geometry geo = AssimpGeometry(fileName);
	Renderer renderer(geo);
	
	int c = 1;																//____________
	Logger::warning("remove divide by 10");									//|HERE|	  |
#pragma message("[WARNING] remove divide by 10")							//			  V
	for (Template* i = templates.begin(); i < (templates.begin() + templates.getSize()/10); i++) {
		//TODO use CUDA with OpenGL directly on GPU
		// instead moving textures to CPU and using OpenCV
		// OpenMP??

		//TODO remove monitoring
		Logger::log("Rendered " + std::to_string(c++) +
			"\t frames out of "+ std::to_string(templates.getSize()) + "\r", true);
		renderer.setModel(i->sixDOF);
		renderer.render(textureMaps);
		extractCandidates();
		rasterizeCandidates(i);
	}
	Logger::log("\n");
	//TODO remove logging
	Logger::logProcess("generarteObject");
}

Model::Model(std::string fileName)
{
	getObjectName(fileName, objectName);
	
	std::vector<std::string> loadedObjects = config.getEntries("loaded objects");
	bool loaded = false;
	if (loadedObjects.size() > 0 && find(loadedObjects.begin(), loadedObjects.end(), objectName) != loadedObjects.end())
		loaded = load();
	if (!loaded) {
		generarteObject(fileName);
		config.getEntries("loaded objects").push_back(objectName);
		config.save();
		save();
	}
	std::cout << *this;
}

std::string getSavePath(const std::string& objectName) {
	return LOADED_OBJECTS_FOLDER + objectName + ".txt";
}

void Model::save(std::string fileName) {
	//TODO remove logging
	Logger::logProcess("save");
	if (fileName.empty())
		fileName = getSavePath(objectName);
	std::ofstream out(fileName, std::ios::out | std::ios::binary);
	out << bits(objectName);
	out << bits(templates);
	out.close();
	//TODO remove logging
	Logger::logProcess("save");
}

bool Model::load() {
	//TODO remove logging
	Logger::logProcess("load");
	std::ifstream in(getSavePath(objectName), std::ios::in | std::ios::binary);
	if (!in.is_open()) {
		Logger::warning(objectName + " save file not found");
		return false;
	}
	in >> bits(objectName);
	in >> bits(templates);
	in.close();
	//TODO remove logging
	Logger::logProcess("load");
	return true;
}
