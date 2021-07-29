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
    Logger::logging = true;
    Logger::logProcess("testing save");
    Logger::log(tensor.toString());
    ofstream ofile("tensorTest.dat");
    ofile << bits(tensor);
    ofile.close();
    Logger::logProcess("testing save");
    Logger::logProcess("testing load");
    tr::Tensor<int> tensor2;
    Logger::log(tensor2.toString());
    ifstream ifile("tensorTest.dat");
    if (!ifile.is_open()) {
        Logger::error("save file not found");
        exit(1);
    }
    ifile >> bits(tensor2);
    ifile.close();
    Logger::log(tensor2.toString());
    Logger::logProcess("testing load");
    Logger::logProcess("testing tensor");
    /*PoseEstimator poseEstimator(frame.cols, frame.rows, model);
    namedWindow("jep");
    imshow("jep", frame);
    waitKey(1);
    poseEstimator.getPose(frame).print(std::cout);*/
    return 0;
}