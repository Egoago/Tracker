#include "Renderer.h"
//#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void Renderer::createFrameBuffers() {
    glGenTextures(3, mapBuffers);
    glGenFramebuffers(3, frameBuffers);

    //====== Pos buffers =======
    //Position buffer
    glBindTexture(GL_TEXTURE_2D, mapBuffers[POS]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    //Depth buffer
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, resolution.x, resolution.y);
    //glDepthFunc(GL_LEQUAL);

    //Frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[POS]);
    glViewport(0, 0, resolution.x, resolution.y);

    //Bind
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mapBuffers[POS], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    //====== Mask frame buffers =======
    //Direction buffer
    glBindTexture(GL_TEXTURE_2D, mapBuffers[MASK]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, resolution.x, resolution.y, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //Frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[MASK]);
    glViewport(0, 0, resolution.x, resolution.y);

    //Bind
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mapBuffers[MASK], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    //====== Dir frame buffers =======
    //Direction buffer
    glBindTexture(GL_TEXTURE_2D, mapBuffers[DIR]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //Frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[DIR]);
    glViewport(0, 0, resolution.x, resolution.y);

    //Bind
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mapBuffers[DIR], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

Renderer::Renderer(float highThreshold, float lowThreshold, unsigned int width, unsigned int height) :
    highThreshold(highThreshold),
    lowThreshold(lowThreshold)
{
    //TODO add back-face culling
    int argc=0;
    //TODO argc and argv in release
    glutInit(&argc, nullptr);
    resolution = glm::uvec2(width, height);
    //glutInitWindowSize(1, 1);
    //glutInitWindowPosition(10, 10);
    glutCreateWindow("OpenGL");
    /*glutHideWindow();
    * can not hide window due to
    * main loop not being called*/
    
    //glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glEnable(GL_DEPTH_TEST);
    setProj();
    setModel(SixDOF());

    glewInit();

    faceShader = new Shader("face");
    edgeShader = new Shader("edge");

    createFrameBuffers();
}

Renderer::~Renderer()
{
    glDeleteFramebuffers(3, frameBuffers);
    glDeleteVertexArrays(3, VAOs);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    delete faceShader;
    delete edgeShader;
}

void getOutlinerEdges(const Geometry& geometry,
                    std::vector<glm::vec3>& edges,
                    std::vector<glm::vec3>& directions,
                    const float threshold)
{
    edges.clear();
    directions.clear();
    for (unsigned int i = 0; i < geometry.getEdgeCount(); i++)
        if (geometry.getCurvatures()[i] > threshold) {
            glm::vec3 a = geometry.getEdges()[2 * i];
            glm::vec3 b = geometry.getEdges()[2 * i + 1];
            edges.push_back(a);
            edges.push_back(b);
            glm::vec3 dir = -glm::normalize(a - b);
            //opposite directions are the same
            if (glm::dot(glm::normalize(glm::vec3(0.9f, 0.65f, 0.56f)), dir) < 0.0f)
                dir = -dir;
            directions.push_back(dir);
            directions.push_back(dir);
        }
}

void Renderer::setGeometry(const Geometry& geometry)
{
    glGenVertexArrays(3, VAOs);

    //====== Pos buffers =======
    glBindVertexArray(VAOs[POS]);

    //vertices
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, geometry.getVertexCount() * geometry.getVertexSize(), geometry.getVertices(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //triangle vertex indices
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndexCount() * geometry.getIndexSize(), geometry.getIndices(), GL_STATIC_DRAW);
    
    faceCount = geometry.getFaceCount();

    //====== Mask edge buffers =======
    glBindVertexArray(VAOs[MASK]);

    //generating outliner edges
    std::vector<glm::vec3> edges, directions;
    getOutlinerEdges(geometry, edges, directions, highThreshold);
    highEgdeCount = (unsigned int)edges.size() / 2;

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, highEgdeCount * 2 * sizeof(glm::vec3), edges.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //directions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, highEgdeCount * 2 * sizeof(glm::vec3), directions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //====== Dir edge buffers =======
    glBindVertexArray(VAOs[DIR]);

    //generating outliner edges
    getOutlinerEdges(geometry, edges, directions, lowThreshold);
    lowEdgeCount = (unsigned int)edges.size() / 2;

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, lowEdgeCount * 2 * sizeof(glm::vec3), edges.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //directions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, lowEdgeCount * 2 * sizeof(glm::vec3), directions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void Renderer::setProj(float fov, float nearP, float farP)
{
    this->nearP = nearP;
    this->farP = farP;
	ProjMtx = glm::perspective(glm::radians(fov), (float)resolution.x/resolution.y, nearP, farP);
}

void Renderer::setModel(SixDOF& sixDOF)
{
	ViewModelMtx = glm::mat4(1.0f);
	ViewModelMtx = glm::translate(ViewModelMtx, glm::vec3(sixDOF.position.x,
                                            sixDOF.position.y,
                                            sixDOF.position.z));
	ViewModelMtx = glm::rotate(ViewModelMtx, sixDOF.orientation.p, glm::vec3(1,0,0));
	ViewModelMtx = glm::rotate(ViewModelMtx, sixDOF.orientation.y, glm::vec3(0,1,0));
	ViewModelMtx = glm::rotate(ViewModelMtx, sixDOF.orientation.r, glm::vec3(0,0,1));
}

glm::mat4 Renderer::render(void* posMap, void* maskMap,void* dirMap)
{
    //render pos
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[POS]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int bits = 0;
    glGetIntegerv(GL_DEPTH_BITS, &bits);
    //TODO remove unnecessary pos background
    static const float max = 1000000.0f * 32.5f;
    static const float background[] =  { max, max, max };
    glClearTexImage(mapBuffers[POS], 0, GL_RGBA, GL_FLOAT, background);
    faceShader->enable();
    faceShader->registerFloat4x4("P", &ProjMtx[0][0]);
    faceShader->registerFloat4x4("VM", &ViewModelMtx[0][0]);
    faceShader->registerFloat("near", nearP);
    faceShader->registerFloat("far", farP);
    glBindVertexArray(VAOs[POS]);
    glDrawElements(GL_TRIANGLES, (GLsizei)faceCount*3, GL_UNSIGNED_INT, 0);
    faceShader->disable();
    
    //render mask
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[MASK]);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);
    edgeShader->enable();
    edgeShader->registerFloat4x4("P" , &ProjMtx[0][0]);
    edgeShader->registerFloat4x4("VM", &ViewModelMtx[0][0]);
    edgeShader->registerFloat("near", nearP);
    edgeShader->registerFloat("far", farP);
    glBindVertexArray(VAOs[MASK]);
    glDrawArrays(GL_LINES, 0, highEgdeCount * 2);

    //render dir
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffers[DIR]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VAOs[DIR]);
    glDrawArrays(GL_LINES, 0, lowEdgeCount * 2);
    edgeShader->disable();

    glFlush();
    
    //Copy from frameBuffer to opencv mat
    glBindTexture(GL_TEXTURE_2D, mapBuffers[POS]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, posMap);
    glBindTexture(GL_TEXTURE_2D, mapBuffers[MASK]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, maskMap);
    glBindTexture(GL_TEXTURE_2D, mapBuffers[DIR]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, dirMap);
    return getMVP();
}
