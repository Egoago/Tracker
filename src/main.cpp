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
    Logger::logging = false;
    tr::Tensor<int> tensor({ 1,2,3 }, 0);
    int count = 0;
    for (unsigned int i = 0; i < 2; i++)
        for (unsigned int x = 0; x < 3; x++)
            tensor({0, i, x}) = count++;
    std::vector<tr::Tensor<int>> tensors(10, tensor);
    Logger::logging = true;
    Logger::logProcess("testing save");
    for(auto& t : tensors)
        Logger::log(t.toString());
    ofstream ofile("tensorTest.dat");
    ofile << bits(tensors);
    ofile.close();
    Logger::logProcess("testing save");
    Logger::logProcess("testing load");
    std::vector<tr::Tensor<int>> tensors2;
    ifstream ifile("tensorTest.dat");
    if (!ifile.is_open()) {
        Logger::error("save file not found");
        exit(1);
    }
    ifile >> bits(tensors2);
    ifile.close();
    for(auto& t : tensors2)
        Logger::log(t.toString());
    Logger::logProcess("testing load");
    Logger::logProcess("testing tensor");
    /*PoseEstimator poseEstimator(frame.cols, frame.rows, model);
    namedWindow("jep");
    imshow("jep", frame);
    waitKey(1);
    poseEstimator.getPose(frame).print(std::cout);*/
    return 0;
}