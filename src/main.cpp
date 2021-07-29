#include "Misc/Links.h"
#include "PoseEstimation/PoseEstimator.h"
#include <opencv2/highgui.hpp>
#include "Misc/Log.h"
#include "Math/Tensor.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    Mat frame = imread(TEST_FRAME);
    //Model model("cylinder.stl");

    Logger::logProcess("testing tensor");
    Logger::logProcess("init");
    tr::Tensor<tr::Template> tensor({ 1,2,3 });
    Logger::logProcess("init");
    Logger::logProcess("modify");
    tensor.at({ 0,0,0 }).sixDOF.orientation.b = 3.0f;
    tensor({ 0,1,2 }).pos.push_back(glm::vec3(1.0f, 2.0f, 3.0f));
    Logger::logProcess("modify");
    ostringstream str;
    str << tensor;
    Logger::logProcess("testing save");
    ofstream ofile("tensorTest.dat");
    ofile << bits(tensor);
    ofile.close();
    Logger::logProcess("testing save");
    Logger::logProcess("testing load");
    tr::Tensor<tr::Template> tensor2;
    ifstream ifile("tensorTest.dat");
    if (!ifile.is_open()) {
        Logger::error("save file not found");
        exit(1);
    }
    ifile >> bits(tensor2);
    ifile.close();
    str << tensor2;
    Logger::logProcess("testing load");
    Logger::logProcess("testing tensor");
    Logger::log("Result:");
    Logger::log(str.str());
    const glm::vec3& p = (tensor2.end() - 1)->pos.at(0);
    Logger::log("new pos: "
        + to_string(p.x) + " "
        + to_string(p.y) + " "
        + to_string(p.z));
    /*PoseEstimator poseEstimator(frame.cols, frame.rows, model);
    namedWindow("jep");
    imshow("jep", frame);
    waitKey(1);
    poseEstimator.getPose(frame).print(std::cout);*/
    return 0;
}