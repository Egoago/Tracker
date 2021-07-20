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
    //====== face frame buffers =======
    //Position buffer
    glGenTextures(1, &posMapBuffer);
    glBindTexture(GL_TEXTURE_2D, posMapBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    //Normal buffer
    glGenTextures(1, &normalMapBuffer);
    glBindTexture(GL_TEXTURE_2D, normalMapBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //Depth buffer
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution.x, resolution.y);

    //Frame buffer
    glGenFramebuffers(1, &faceFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, faceFrameBuffer);
    glViewport(0, 0, resolution.x, resolution.y);

    //Bind
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, posMapBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normalMapBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, DrawBuffers);

    //====== edge frame buffers =======
    //Direction buffer
    glGenTextures(1, &dirMapBuffer);
    glBindTexture(GL_TEXTURE_2D, dirMapBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //Depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution.x, resolution.y);

    //Frame buffer
    glGenFramebuffers(1, &edgeFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, edgeFrameBuffer);
    glViewport(0, 0, resolution.x, resolution.y);

    //Bind
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dirMapBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

Renderer::Renderer(unsigned int width, unsigned int height)
{
    int argc;
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
    faceShader.loadShader(GL_VERTEX_SHADER, "src/Shaders/face.vert");
    faceShader.loadShader(GL_FRAGMENT_SHADER, "src/Shaders/face.frag");
    faceShader.compile();
    edgeShader.loadShader(GL_VERTEX_SHADER, "src/Shaders/edge.vert");
    edgeShader.loadShader(GL_FRAGMENT_SHADER, "src/Shaders/edge.frag");
    edgeShader.compile();

    createFrameBuffers();
}

Renderer::~Renderer()
{
    glDeleteFramebuffers(1, &faceFrameBuffer);
    glDeleteVertexArrays(1, &faceVAO);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Renderer::setGeometry(const Geometry& geometry)
{
    //====== face buffers =======
    glGenVertexArrays(1, &faceVAO);
    glBindVertexArray(faceVAO);

    //vertices
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, geometry.getVertexCount() * geometry.getVertexSize(), geometry.getVertices(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //normals
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, geometry.getNormalCount() * geometry.getNormalSize(), geometry.getNormals(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //triangle vertex indices
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndexCount() * geometry.getIndexSize(), geometry.getIndices(), GL_STATIC_DRAW);
    
    faceCount = geometry.getFaceCount();

    //====== edge buffers =======
    edgeCount = geometry.getEdgeCount();
    glGenVertexArrays(1, &edgeVAO);
    glBindVertexArray(edgeVAO);

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, edgeCount * geometry.getVertexSize() * 2, geometry.getEdges(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //directions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glm::vec3* directions = new glm::vec3[edgeCount * 2];
    for(unsigned int i = 0; i < edgeCount; i++) {
        glm::vec3 a = geometry.getEdges()[2 * i];
        glm::vec3 b = geometry.getEdges()[2 * i + 1];
        glm::vec3 dir = glm::normalize(a - b);
        directions[2 * i] = dir;
        directions[2 * i + 1] = dir;
    }
    glBufferData(GL_ARRAY_BUFFER, edgeCount * 2 * sizeof(glm::vec3), directions, GL_STATIC_DRAW);
    delete[] directions;
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

}

void Renderer::setProj(float fov, float nearP, float farP)
{
	Proj = glm::perspective(glm::radians(fov), (float)resolution.x/resolution.y, nearP, farP);
    MVP = Proj * Model;
}

void Renderer::setModel(SixDOF& sixDOF)
{
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, glm::vec3(sixDOF.position.x,
                                            sixDOF.position.y,
                                            sixDOF.position.z));
	Model = glm::rotate(Model, sixDOF.orientation.p, glm::vec3(1,0,0));
	Model = glm::rotate(Model, sixDOF.orientation.y, glm::vec3(0,1,0));
	Model = glm::rotate(Model, sixDOF.orientation.r, glm::vec3(0,0,1));
    MVP = Proj * Model;
}

glm::mat4 Renderer::render(void* posMap, void* normalMap, void* dirMap)
{
    //render faces
    glBindFramebuffer(GL_FRAMEBUFFER, faceFrameBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    faceShader.enable();
    glLineWidth(1.0);
    faceShader.registerMVP(&MVP[0][0]);
    glBindVertexArray(faceVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)faceCount*3, GL_UNSIGNED_INT, 0);
    glDrawArrays(GL_LINES, 0, edgeCount * 2);
    faceShader.disable();
    
    //render edges
    glBindFramebuffer(GL_FRAMEBUFFER, edgeFrameBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    edgeShader.enable();
    glLineWidth(3.0);
    edgeShader.registerMVP(&MVP[0][0]);
    glBindVertexArray(edgeVAO);
    glDrawArrays(GL_LINES, 0, edgeCount * 2);
    edgeShader.disable();

    glFlush();
    
    //Copy from frameBuffer to opencv mat
    glBindTexture(GL_TEXTURE_2D, posMapBuffer);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, posMap);
    glBindTexture(GL_TEXTURE_2D, normalMapBuffer);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, normalMap);
    glBindTexture(GL_TEXTURE_2D, dirMapBuffer);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_FLOAT, dirMap);
    return MVP;
}
