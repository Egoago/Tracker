#include "Object/Object.h"

int main(int argc, char** argv) {
    
    Object object("cube.STL");
    //object.print();

    /*std::ifstream ifs("kacsa.txt");
    ifs >> bits(object2);
    ifs.close();
    object2.print();*/

    /*Camera* cam = new OpenCVCamera();
    namedWindow("Original", WINDOW_NORMAL);
    resizeWindow("Original", cam->getWidth() / 2, cam->getHeight() / 2);
    moveWindow("Original", 50, 50);
    namedWindow("Canny", WINDOW_NORMAL);
    resizeWindow("Canny", cam->getWidth()/2, cam->getHeight()/2);
    moveWindow("Canny", cam->getWidth() / 2+50, 50);
    while (1)
    {
        char* frame = cam->getNextFrame();
        Mat wrapped(Size(cam->getWidth(), cam->getHeight()), cam->getFormat(), frame);
        cout <<"\r"<<cam->getFPS();
        imshow("Original", wrapped);
        Mat dst, detected_edges;
        blur(wrapped, detected_edges, Size(5, 5));
        Canny(wrapped, detected_edges, 200, 30 * 3, 3);
        dst = Scalar::all(0);
        wrapped.copyTo(dst, detected_edges);
        imshow("Canny", dst);
        waitKey(1);
    }
    delete cam;*/
    return 0;
}