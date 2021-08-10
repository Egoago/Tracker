#include "Model.hpp"
#include <mutex>
#include <random>
#include <numeric>
#include <opencv2/imgproc.hpp>
#include "AssimpGeometry.hpp"
#include "../Rendering/Renderer.hpp"
#include "../Misc/Links.hpp"
#include "../Misc/Range.hpp"
#include "../Misc/Base.hpp"
#include "../Math/RasterPoint.hpp"
//TODO remove logging
#include "../Misc/Log.hpp"
#include <opencv2/highgui.hpp>
#include "../PoseEstimation/CeresRegistrator.hpp"

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
	const static Range width(config.getEntries("width", { "-120.0", "120.0", "1" }));//7
	const static Range height(config.getEntries("height", { "-120.0", "120.0", "1" }));//7
	const static Range depth(config.getEntries("depth", { "-325.0", "-4000.0", "3" }));//5
	//TODO uniform sphere distr <=> homogenous tensor layout????
	const static Range roll(config.getEntries("roll", { "0", "0", "1" }));//6
	const static Range yaw(config.getEntries("yaw", { "0", "0", "1" }));//6
	const static Range pitch(config.getEntries("pitch", { "0", "180", "3" }));//3
	templates.allocate({
		width.resolution,
		height.resolution,
		depth.resolution,
		yaw.resolution,
		pitch.resolution,
		roll.resolution });
	for (uint ya = 0; ya < yaw.resolution; ya++)
	for (uint p = 0; p < pitch.resolution; p++)
	for (uint r = 0; r < roll.resolution; r++)
	for (uint x = 0; x < width.resolution; x++)
	for (uint y = 0; y < height.resolution; y++)
	for (uint z = 0; z < depth.resolution; z++)
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

void floatToBinary(const cv::Mat& floatMap, cv::Mat& binary) {
	//Logger::logProcess(__FUNCTION__);	//TODO remove logging
	cv::absdiff(floatMap, cv::Scalar::all(0), binary);
	switch (binary.channels()) {
		case 3: cv::transform(binary, binary, cv::Matx13f(1.0, 1.0, 1.0)); break;
		case 2: cv::transform(binary, binary, cv::Matx12f(1.0, 1.0)); break;
		case 4: cv::transform(binary, binary, cv::Matx14f(1.0, 1.0, 1.0, 1.0)); break;
	}
	cv::threshold(binary, binary, 1e-10, 255, cv::THRESH_BINARY);
	binary.convertTo(binary, CV_8U);
	//Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

void getContour(const cv::Mat& maskMap, cv::Mat& contourMap) {
	//Logger::logProcess(__FUNCTION__);	//TODO remove logging
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(maskMap, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	cv::drawContours(contourMap, contours, 0, cv::Scalar(255, 255, 255));
	//Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

void Model::extractCandidates() {
	//Logger::logProcess(__FUNCTION__);	//TODO remove logging
	candidates.clear();
	cv::Mat maskMap;
	floatToBinary(*textureMaps[Renderer::HDIR], maskMap);
	getContour(*textureMaps[Renderer::MESH], maskMap);
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
	//Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

void Model::rasterizeCandidates(Template* temp, const glm::mat4& mvp) {
	//Logger::logProcess(__FUNCTION__);	//TODO remove logging

	const uint bufferSize = (uint)candidates.size();
	const static int rasterCount = std::stoi(config.getEntry("rasterization count", "100"));
	const static float rasterOffset = stof(config.getEntry("rasterization offset", "0.0015"));
	float rasterProb = (float)rasterCount / (float)bufferSize;
	if (rasterProb > 1.0f) rasterProb = 1.0f;
	static std::mt19937 gen(std::random_device{}());
	static std::bernoulli_distribution dist(rasterProb);
	std::vector<bool> mask(bufferSize); //mask generation needed for efficient allocation
	uint chosenCount = 0;
	std::generate(mask.begin(), mask.end(), [&] {
		if(dist(gen)){
			chosenCount++;
			return true;
		}
		return false; 
	});
	temp->rasterPoints.reserve(chosenCount);
	//TODO parallelization-compact
	for(uint i = 0; i < bufferSize; i++)
		if (mask[i]) {
			const glm::vec3 p = candidates[i].pos;
			const glm::vec3 op = p + (candidates[i].dir * rasterOffset);
			temp->rasterPoints.emplace_back(p, op);
			if (!temp->rasterPoints.back().render(mvp)) {
				//can be due to pixel out of bounds or nan angle.
				//it happens quite rarely but needs to be handled.
				Logger::warning("Unsuccessfull raster point render");
				temp->rasterPoints.pop_back();
			}
		}
	//Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

//TODO remove logging
void analizeRasterization(const std::vector<tr::uint>& rasterCounts) {
	double avg = std::accumulate(rasterCounts.begin(), rasterCounts.end(), 0.0) / (double)rasterCounts.size();
	double sq_sum = std::inner_product(rasterCounts.begin(), rasterCounts.end(), rasterCounts.begin(), 0.0);
	double stdev = std::sqrt(sq_sum / rasterCounts.size() - avg * avg);
	tr::uint max = *std::max_element(std::begin(rasterCounts), std::end(rasterCounts));
	tr::uint min = *std::min_element(std::begin(rasterCounts), std::end(rasterCounts));
	Logger::log("Rasterization max " + std::to_string(max)
							+ " min " + std::to_string(min)
							+ " mean " + std::to_string(avg)
							+ " stdev " + std::to_string(stdev));
}

void Model::generarteObject(const std::string& fileName) {
	//TODO proper resource destruction
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	//TODO add loading bar
	Geometry geo = AssimpGeometry(fileName);
	Renderer renderer(geo);
	textureMaps = renderer.getTextures();
	P = GlmMatToEigen(renderer.getP());
	CeresRegistrator reg(P);
	generate6DOFs();
	
	int c = 1;
	std::vector<tr::uint> rasterCounts;
	for (Template* i = templates.begin(); i < (templates.begin() + templates.getSize()); i++) {
		//TODO use CUDA with OpenGL directly on GPU
		// instead moving textures to CPU and using OpenCV
		// OpenMP??

		renderer.setVM(i->sixDOF.getModelTransformMatrix());
		renderer.render();
		extractCandidates();
		rasterizeCandidates(i, renderer.getPVM());
		//TODO remove logging
		cv::Mat canvas(cv::Size(800, 800), CV_8UC3);
		canvas = cv::Scalar::all(0);
		for (const auto& rasterPoint : i->rasterPoints) {
			glm::vec2 uv1 = rasterPoint.getUV();
			cv::Point p1((int)std::roundf(uv1.x * (canvas.cols - 1)),
						 (int)std::roundf(uv1.y * (canvas.rows - 1)));
			evec2 uv2;
			reg.project(i->sixDOF.data, &rasterPoint.pos[0], uv2.data());
			cv::Point p2((int)std::roundf(uv2.x() * (canvas.cols - 1)),
						 (int)std::roundf(uv2.y() * (canvas.rows - 1)));
			//line(canvas, p1, p2, cv::Scalar(255, 0, 0));
			circle(canvas, p1, 1, cv::Scalar(0, 255, 0), -1);
			circle(canvas, p2, 1, cv::Scalar(0, 0, 255), -1);
			Logger::log(std::to_string(uv1.x-uv2.x()) + " " + std::to_string(uv1.y - uv2.y()));
		}
		cv::imshow("rasterized", canvas);
		cv::waitKey(10000000);
		//TODO remove monitoring
		rasterCounts.push_back((tr::uint)i->rasterPoints.size());
		Logger::log("Rendered " + std::to_string(c++)
			+ "\t frames out of "+ std::to_string(templates.getSize())
			+ " \r", true);
	}
	analizeRasterization(rasterCounts);
	Logger::log("\n");
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

Model::Model(std::string fileName) {
	getObjectName(fileName, objectName);
	
	if (!load()) {
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
	out << bits(objectName)
		<< bits(P)
		<< bits(templates);
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
	in	>> bits(objectName)
		>> bits(P)
		>> bits(templates);
	in.close();
	Logger::logProcess(__FUNCTION__);	//TODO remove logging
	return true;
}
