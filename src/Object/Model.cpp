#include "Model.h"
#include <mutex>
#include <random>
#include <opencv2/imgproc.hpp>
#include "AssimpGeometry.h"
#include "../Rendering/Renderer.h"
#include "../Misc/Links.h"
//TODO remove logging
#include "../Misc/Log.h"
#include <opencv2/highgui.hpp>

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
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	//TODO tune granularity
	const static Range width(config.getEntries("width", { "-120.0", "120.0", "1" }));
	const static Range height(config.getEntries("height", { "-120.0", "120.0", "1" }));
	const static Range depth(config.getEntries("depth", { "-325.0", "-8000.0", "2" }));
	//TODO uniform sphere distr <=> homogenous tensor layout????
	const static Range roll(config.getEntries("roll", { "0", "360", "2" }));
	const static Range yaw(config.getEntries("yaw", { "0", "360", "2" }));
	const static Range pitch(config.getEntries("pitch", { "0", "180", "2" }));
	templates.allocate({
		width.resolution,
		height.resolution,
		depth.resolution,
		yaw.resolution,
		pitch.resolution,
		roll.resolution });
	for (unsigned int ya = 0; ya < yaw.resolution; ya++)
	for (unsigned int p = 0; p < pitch.resolution; p++)
	for (unsigned int r = 0; r < roll.resolution; r++)
	for (unsigned int x = 0; x < width.resolution; x++)
	for (unsigned int y = 0; y < height.resolution; y++)
	for (unsigned int z = 0; z < depth.resolution; z++)
	{
		SixDOF& sixDOF = templates.at({x,y,z,ya,p,r}).sixDOF;
		sixDOF.position.x = width[x] / depth.begin * depth[z];
		sixDOF.position.y = height[y] / depth.begin * depth[z];
		sixDOF.position.z = depth[z];
		sixDOF.orientation.y = yaw[ya];
		sixDOF.orientation.p = pitch[p];
		sixDOF.orientation.r = roll[r];
	}
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

glm::vec2 renderPoint(const glm::vec3& p, const glm::mat4& mvp) {
	glm::vec4 pPos = mvp * glm::vec4(p, 1.0f);
	pPos /= pPos.w;
	pPos = (pPos + 1.0f) / 2.0f;
	return glm::vec2(pPos.x, pPos.y);
}

//TODO remove monitoring
void drawOnFrame(const glm::vec3& p, cv::Mat& canvas, const glm::mat4& mvp, cv::Scalar color) {
	glm::vec2 uv = renderPoint(p, mvp);
	cv::Point pixel((int)std::roundf(uv.x * (canvas.cols-1)),
					(int)std::roundf(uv.y * (canvas.rows-1)));
	circle(canvas, pixel, 1, color, -1);
}

void floatToBinary(const cv::Mat& floatMap, cv::Mat& binary) {
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	cv::absdiff(floatMap, cv::Scalar::all(0), binary);
	switch (binary.channels()) {
		case 3: cv::transform(binary, binary, cv::Matx13f(1.0, 1.0, 1.0)); break;
		case 2: cv::transform(binary, binary, cv::Matx12f(1.0, 1.0)); break;
		case 4: cv::transform(binary, binary, cv::Matx14f(1.0, 1.0, 1.0, 1.0)); break;
	}
	cv::threshold(binary, binary, 1e-10, 255, cv::THRESH_BINARY);
	binary.convertTo(binary, CV_8U);
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

void getContour(const cv::Mat& lowDirMap, cv::Mat& contourMap) {
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	cv::Mat tmp;
	floatToBinary(lowDirMap, tmp);
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(tmp, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	cv::drawContours(contourMap, contours, 0, cv::Scalar(255, 255, 255));
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

void Model::extractCandidates() {
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	candidates.clear();
	cv::Mat maskMap;
	floatToBinary(*textureMaps[Renderer::HDIR], maskMap);
	cv::imshow("hdir mask", maskMap);
	getContour(*textureMaps[Renderer::LDIR], maskMap);
	cv::imshow("hdir + contour", maskMap);
	std::mutex mtx;
	maskMap.forEach<uchar>(
		[&textureMaps = textureMaps, &candidates = candidates, &mtx]
		(uchar& pixel, const int* p) -> void {
			if (pixel == 0) return;

			//test high thres
			cv::Point3f pos = textureMaps[Renderer::HPOS]->at<cv::Point3f>(p[0], p[1]);
			cv::Point3f dir = textureMaps[Renderer::HDIR]->at<cv::Point3f>(p[0], p[1]);
			if (pos.dot(pos) > 1e-5 && dir.dot(dir) > 1e-5) {
				mtx.lock();
				candidates.push_back(Candidate(pos, dir));
				mtx.unlock();
				return;
			}
			//test low thres
			pos = textureMaps[Renderer::LPOS]->at<cv::Point3f>(p[0], p[1]);
			dir = textureMaps[Renderer::LDIR]->at<cv::Point3f>(p[0], p[1]);
			if (pos.dot(pos) > 1e-5 && dir.dot(dir) > 1e-5) {
				mtx.lock();
				candidates.push_back(Candidate(pos, dir));
				mtx.unlock();
			}
		});
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

void Model::rasterizeCandidates(Template* temp) {
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	const unsigned int bufferSize = (unsigned int)candidates.size();
	const static int rasterCount = std::stoi(config.getEntry("rasterization count", "100"));
	const static float rasterOffset = stof(config.getEntry("rasterization offset", "4.0f"));
	float rasterProb = (float)rasterCount / (float)bufferSize;
	if (rasterProb > 1.0f) rasterProb = 1.0f;
	static std::mt19937 gen(std::random_device{}());
	static std::bernoulli_distribution dist(rasterProb);
	std::vector<bool> mask(candidates.size());
	unsigned int count = 0;
	std::generate(mask.begin(), mask.end(), [&] {
		count++;
		return dist(gen); 
	});
	temp->pos.resize(count);
	temp->offsetPos.resize(count);
	//TODO parallelization-compact
	for(unsigned int i = 0; i < bufferSize; i++)
		if (mask[i]) {
			const glm::vec3 p = candidates[i].pos;
			if(std::fabsf(glm::length(candidates[i].dir)-1.0f) > 1e-10f)
			Logger::warning("normal: " + std::to_string(glm::length(candidates[i].dir)));
			temp->pos[i] = p;
			temp->offsetPos[i] = p + (candidates[i].dir * rasterOffset);
		}
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

void renderTemplate(Template* temp, const glm::mat4& mvp) {
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	const unsigned int pixelCount = (unsigned int)temp->pos.size();
	temp->uv.resize(pixelCount);
	temp->angle.resize(pixelCount);
	for (unsigned int i = 0; i < pixelCount; i++) {
		const glm::vec2 p = renderPoint(temp->pos[i], mvp);
		const glm::vec2 op = renderPoint(temp->offsetPos[i], mvp);
		temp->uv[i] = p;
		temp->angle[i] = getOrientation(p - op);
	}
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

void Model::generarteObject(const std::string& fileName) {
	//TODO proper resource destruction
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	//TODO add loading bar
	Geometry geo = AssimpGeometry(fileName);
	Renderer renderer(geo);
	generate6DOFs();
	
	int c = 1;
	for (Template* i = templates.begin(); i < (templates.begin() + templates.getSize()); i++) {
		//TODO use CUDA with OpenGL directly on GPU
		// instead moving textures to CPU and using OpenCV
		// OpenMP??

		//TODO remove monitoring
		Logger::log("Rendered " + std::to_string(c++) +
			"\t frames out of "+ std::to_string(templates.getSize()) + "\r", true);
		renderer.setModel(i->sixDOF);
		renderer.render(textureMaps);
		cv::Mat temp;
		textureMaps[Renderer::MESH]->convertTo(temp, CV_32F, 255.0 / 1.0);
		cv::imshow("depth", temp);
		/*cv::imshow("high pos", *textureMaps[HPOS]);
		cv::imshow("high dir", *textureMaps[HDIR]);
		cv::imshow("low pos", *textureMaps[LPOS]);
		cv::imshow("low dir", *textureMaps[LDIR]);*/
		extractCandidates();
		rasterizeCandidates(i);
		renderTemplate(i, renderer.getMVP());
		cv::Mat canvas(cv::Size(800, 800), CV_8UC3);
		canvas = cv::Scalar::all(0);
		for (unsigned int x = 0; x < i->pos.size(); x++) {
			glm::vec2 uv1 = renderPoint(i->pos[x], renderer.getMVP());
			cv::Point p1((int)std::roundf(uv1.x * (canvas.cols - 1)),
						 (int)std::roundf(uv1.y * (canvas.rows - 1)));
			glm::vec2 uv2 = renderPoint(i->offsetPos[x], renderer.getMVP());
			cv::Point p2((int)std::roundf(uv2.x * (canvas.cols - 1)),
						 (int)std::roundf(uv2.y * (canvas.rows - 1)));
			line(canvas, p1, p2, cv::Scalar(255, 0, 0));
			circle(canvas, p1, 1, cv::Scalar(0, 255, 0), -1);
			circle(canvas, p2, 1, cv::Scalar(0, 0, 255), -1);
		}
		cv::imshow("rasterized", canvas);
		Logger::log("Waiting for key...");
		cv::waitKey(10000000);
	}
	Logger::log("\n");
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

Model::Model(std::string fileName) {
	getObjectName(fileName, objectName);
	
	bool loaded = load();
	if (!loaded) {
		generarteObject(fileName);
		save();
	}
}

std::string getSavePath(const std::string& objectName) {
	return LOADED_OBJECTS_FOLDER + objectName + LOADED_OBJECT_FILENAME_EXTENSION;
}

void Model::save(std::string fileName) {
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	if (fileName.empty())
		fileName = getSavePath(objectName);
	std::ofstream out(fileName, std::ios::out | std::ios::binary);
	out << bits(objectName);
	out << bits(templates);
	out.close();
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

bool Model::load() {
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	std::ifstream in(getSavePath(objectName), std::ios::in | std::ios::binary);
	if (!in.is_open()) {
		Logger::warning(objectName + " save file not found");
		return false;
	}
	in >> bits(objectName);
	in >> bits(templates);
	in.close();
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	return true;
}
