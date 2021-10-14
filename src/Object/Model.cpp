#include "Model.hpp"
#include <mutex>
#include <random>
#include <numeric>
#include <opencv2/imgproc.hpp>
#include "AssimpLoader.hpp"
#include "../Rendering/Renderer.hpp"
#include "../Misc/Links.hpp"
#include "../Misc/Range.hpp"
#include "../Misc/Base.hpp"
#include "../Math/RasterPoint.hpp"
//TODO remove logging
#include "../Misc/Log.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>

//TODO break apart
using namespace tr;

ConfigParser Model::config(OBJ_CONFIG_FILE);

Range::Interploation getDepthInterpolation(ConfigParser& config) {
	switch (strHash(config.getEntry<std::string>("depth interpolation", "exp").c_str())) {
	case strHash("lin"): return Range::Interploation::LINEAR;
	case strHash("exp"): return Range::Interploation::EXPONENTIAL;
	case strHash("pol"): return Range::Interploation::POLINOMIAL;
	default: return Range::Interploation::LINEAR;
	}
}

void generate6DOFs(ConfigParser& config, Tensor<Template>& templates) {
	Logger::logProcess(__FUNCTION__);
	//TODO tune granularity
	const static Range width(config.getEntries<int>("width", { -70, 70, 5 }));//7
	const static Range height(config.getEntries<int>("height", { -70, 70, 5 }));//7
	const static Range depth(config.getEntries<int>("depth", { -250, -1500, 4 }), getDepthInterpolation(config));//5
	//TODO uniform sphere distr <=> homogenous tensor layout????
	const static Range yaw(config.getEntries<int>("yaw", { 0, 360, 6 }), Range::Interploation::LINEAR, true);//6
	const static Range pitch(config.getEntries<int>("pitch", { -60, 60, 3 }), Range::Interploation::LINEAR, false);//3
	const static Range roll(config.getEntries<int>("roll", { 0, 360, 3 }), Range::Interploation::LINEAR, true);//6
	templates.allocate({
		width.size(),
		height.size(),
		depth.size(),
		yaw.size(),
		pitch.size(),
		roll.size() });
	for (uint ya = 0; ya < yaw.size(); ya++)
	for (uint p = 0; p < pitch.size(); p++)
	for (uint r = 0; r < roll.size(); r++)
	for (uint x = 0; x < width.size(); x++)
	for (uint y = 0; y < height.size(); y++)
	for (uint z = 0; z < depth.size(); z++)
	{
		SixDOF& sixDOF = templates.at({x,y,z,ya,p,r}).sixDOF;
		sixDOF.position.x() = width[x] / depth[0] * depth[z];
		sixDOF.position.y() = height[y] / depth[0] * depth[z];
		sixDOF.position.z() = depth[z];
		sixDOF.orientation.x() = radian(yaw[ya]);
		sixDOF.orientation.y() = radian(pitch[p]);
		sixDOF.orientation.z() = radian(roll[r]);
	}
	Logger::logProcess(__FUNCTION__);
}

void floatToBinary(const cv::Mat& floatMap, cv::Mat& binary) {
	//Logger::logProcess(__FUNCTION__);
	cv::absdiff(floatMap, cv::Scalar::all(0), binary);
	switch (binary.channels()) {
		case 3: cv::transform(binary, binary, cv::Matx13f(1.0, 1.0, 1.0)); break;
		case 2: cv::transform(binary, binary, cv::Matx12f(1.0, 1.0)); break;
		case 4: cv::transform(binary, binary, cv::Matx14f(1.0, 1.0, 1.0, 1.0)); break;
	}
	cv::threshold(binary, binary, 1e-10, 255, cv::THRESH_BINARY);
	binary.convertTo(binary, CV_8U);
	//Logger::logProcess(__FUNCTION__);
}

void getContour(const cv::Mat& maskMap, cv::Mat& contourMap) {
	//Logger::logProcess(__FUNCTION__);
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(maskMap, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	cv::drawContours(contourMap, contours, 0, cv::Scalar(255, 255, 255));
	//Logger::logProcess(__FUNCTION__);
}


struct Candidate {
	vec3f pos, dir;
	Candidate(vec3f pos, vec3f dir) : pos(pos), dir(dir) {}
};

const std::vector<Candidate> extractCandidates(const std::vector<cv::Mat*>& textureMaps) {
	//Logger::logProcess(__FUNCTION__);
	std::vector<Candidate> candidates;
	cv::Mat maskMap;
	floatToBinary(*textureMaps[Renderer::HDIR], maskMap);
	getContour(*textureMaps[Renderer::MESH], maskMap);
	std::mutex mtx;
	maskMap.forEach<uchar>(
		[&textureMaps = textureMaps, &candidates = candidates, &mtx]
		(uchar& pixel, const int* p) -> void {
			if (pixel == 0) return;

			//test high thres
			Eigen::Vector3f pos = textureMaps[Renderer::HPOS]->at<Eigen::Vector3f>(p[0], p[1]);
			Eigen::Vector3f dir = textureMaps[Renderer::HDIR]->at<Eigen::Vector3f>(p[0], p[1]);
			if (pos.dot(pos) > 1e-5 && dir.dot(dir) > 1e-5) {
				mtx.lock();
				candidates.push_back(Candidate(pos, dir));
				mtx.unlock();
				return;
			}
			//test low thres
			pos = textureMaps[Renderer::LPOS]->at<Eigen::Vector3f>(p[0], p[1]);
			dir = textureMaps[Renderer::LDIR]->at<Eigen::Vector3f>(p[0], p[1]);
			if (pos.dot(pos) > 1e-5 && dir.dot(dir) > 1e-5) {
				mtx.lock();
				candidates.push_back(Candidate(pos, dir));
				mtx.unlock();
			}
		});
	//Logger::logProcess(__FUNCTION__);
	return candidates;
}

void rasterizeCandidates(const std::vector<Candidate>& candidates,
						Template* temp,
						ConfigParser& config,
						const mat4f& PVM) {
	//Logger::logProcess(__FUNCTION__);

	const uint bufferSize = (uint)candidates.size();
	const static int rasterCount = config.getEntry("rasterization count", 100);
	const static float rasterOffset = config.getEntry("rasterization offset", 0.005f);
	std::vector<Candidate> chosenCandidates;
	std::sample(candidates.begin(), candidates.end(), std::back_inserter(chosenCandidates),
		rasterCount, std::mt19937{ std::random_device{}()});
	temp->rasterPoints.reserve(chosenCandidates.size());
	//TODO parallelization-compact
	for (const auto& candidate : chosenCandidates) {
		const vec3f p = candidate.pos;
		const vec3f op = p + (candidate.dir * rasterOffset);
		temp->rasterPoints.emplace_back(p, op);
		if (!temp->rasterPoints.back().render(PVM))
			temp->rasterPoints.pop_back();
	}
	//Logger::logProcess(__FUNCTION__);
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

void generarteObject(const Geometry& geo, Tensor<Template>& templates, ConfigParser& config, const mat4f& P) {
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
	//TODO proper resource destruction
	Logger::logProcess(__FUNCTION__);
	//TODO add loading bar
	
	Renderer renderer(geo);
	renderer.setProj(P);
	std::vector<cv::Mat*> textureMaps = renderer.getTextures(); //TODO smart pointer
	generate6DOFs(config, templates);
	int c = 1;
	std::vector<tr::uint> rasterCounts;
	for (Template* temp = templates.begin(); temp < (templates.begin() + templates.getSize()); temp++) {
		//TODO use CUDA with OpenGL directly on GPU
		// instead moving textures to CPU and using OpenCV
		// OpenMP??

		const vec3f scalingParameters = renderer.setVM(temp->sixDOF.getModelTransformMatrix());	//TODO scale instead of rerender?
		renderer.render();
		const std::vector<Candidate> candidates = extractCandidates(textureMaps);
		rasterizeCandidates(candidates, temp, config, renderer.getPVM());
		//TODO remove logging
		Logger::drawFrame(textureMaps[Renderer::MESH], "mesh");
		Logger::drawFrame(textureMaps[Renderer::HPOS], "hpos");
		//Logger::drawFrame(textureMaps[Renderer::HDIR], "hgir");
		//cv::Mat canvas(cv::Size(800, 800), CV_8UC3);
		//canvas = cv::Scalar::all(0);
		//for (const auto& rasterPoint : temp->rasterPoints) {
		//	vec2f uv1 = rasterPoint.uv;
		//	cv::Point p1((int)std::roundf(uv1.x() * (canvas.cols - 1)),
		//				 (int)std::roundf(uv1.y() * (canvas.rows - 1)));
		//	/*vec2d uv2;
		//	reg.project(i->sixDOF.data, &rasterPoint.pos[0], uv2.data());
		//	cv::Point p2((int)std::roundf(uv2.x() * (canvas.cols - 1)),
		//				 (int)std::roundf(uv2.y() * (canvas.rows - 1)));*/
		//	//line(canvas, p1, p2, cv::Scalar(255, 0, 0));
		//	circle(canvas, p1, 1, cv::Scalar(0, 255, 0), -1);
		//	//circle(canvas, p2, 1, cv::Scalar(0, 0, 255), -1);
		//	//Logger::log(std::to_string(uv1.x-uv2.x()) + " " + std::to_string(uv1.y - uv2.y()));
		//}
		//Logger::drawFrame(&canvas, "rasterized");
		//cv::waitKey(10000000);
		//TODO remove monitoring
		rasterCounts.push_back((tr::uint)temp->rasterPoints.size());
		Logger::log("Rendered " + std::to_string(c++)
			+ "\t frames out of "+ std::to_string(templates.getSize())
			+ " \r", true);
	}
	analizeRasterization(rasterCounts);
	Logger::log("\n");
	Logger::logProcess(__FUNCTION__);
}

std::string getObjectName(const std::string& fName) {
	size_t dot = fName.find('.');
	return (dot == std::string::npos) ? fName : fName.substr(0, dot);
}

std::string getSavePath(const std::string& objectName) {
	return LOADED_OBJECTS_FOLDER + objectName + LOADED_OBJECT_FILENAME_EXTENSION;
}

Model::Model(const std::string& fileName) : Model(fileName, Renderer::getDefaultP()) {}
Model::Model(const std::string& fileName, const mat4f& P) : P(P) {
	std:: string objectName = getObjectName(fileName);
	std:: string filePath = getSavePath(objectName);
	if (!load(filePath)) {
		Geometry geo;
		AssimpLoader::load(objectName, geo);
		generarteObject(geo, templates, config, P);
		save(filePath);
	}
}

void Model::save(const std::string& fileName) {
	Logger::logProcess(__FUNCTION__);
	std::ofstream out(fileName, std::ios::out | std::ios::binary);
	out << bits(P)
		<< bits(templates);
	out.close();
	Logger::logProcess(__FUNCTION__);
}

bool Model::load(const std::string& filename) {
	Logger::logProcess(__FUNCTION__);
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (!in.is_open()) {
		Logger::warning(filename + " save file not found");
		return false;
	}
	in	>> bits(P)
		>> bits(templates);
	in.close();
	Logger::logProcess(__FUNCTION__);
	return true;
}
