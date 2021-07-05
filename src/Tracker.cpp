#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Camera/OpenCVCamera.h"
#include "Object/AssimpGeometry.h"
#include "Rendering/Shader.h"

using namespace cv;
using namespace std;

AssimpGeometry* geo;
Shader shader;

void init(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(1280, 1080);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("Assimp test");
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glEnable(GL_DEPTH_TEST);

    glewInit();
    int majorVersion = 3, minorVersion = 3;
    printf("GL Vendor    : %s\n", glGetString(GL_VENDOR));
    printf("GL Renderer  : %s\n", glGetString(GL_RENDERER));
    printf("GL Version (string)  : %s\n", glGetString(GL_VERSION));
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    printf("GL Version (integer) : %d.%d\n", majorVersion, minorVersion);
    printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -200.0);
    glRotated(-45.0, 0.0, 1.0, 0.0);

    GLfloat black[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat yellow[] = { 1.0, 1.0, 0.0, 1.0 };
    GLfloat cyan[] = { 0.0, 1.0, 1.0, 1.0 };
    GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat direction[] = { 1.0, 1.0, 1.0, 0.0 };

    shader.loadShader(GL_VERTEX_SHADER, "../../src/Shaders/pass.vert");
    shader.loadShader(GL_FRAGMENT_SHADER, "../../src/Shaders/simple.frag");
    shader.compile();


}
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPointSize(2.0);
    glLineWidth(1.0);

    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(40, 0, 0);
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 40, 0);
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 40);
    glEnd();
    shader.enable();

    glColor3f(1.0, 1.0, 1.0);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    GLuint VB, IB;
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, geo->getVerticesCount() * geo->getVertexSize(), geo->getVertices(), GL_STATIC_DRAW);

    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geo->getIndecesCount() * geo->getIndexSize(), geo->getIndices(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, geo->getVertexSize(), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, geo->getVertexSize(), (const GLvoid*)12);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

    glDrawElements(GL_TRIANGLES, geo->getIndecesCount(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    shader.disable();
    glFlush();
}

int main(int argc, char** argv) {
    geo = new AssimpGeometry("cylinder.STL");
    init(argc, argv);
    display();
    waitKey(1000000);
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