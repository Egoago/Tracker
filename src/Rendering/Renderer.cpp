#include "Renderer.h"
//#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

void Renderer::createFrameBuffer() {
    //Position buffer
    glGenTextures(1, &posBuffer);
    glBindTexture(GL_TEXTURE_2D, posBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, posBuffer, 0);

    //Index buffer
    glGenTextures(1, &indexBuffer);
    glBindTexture(GL_TEXTURE_2D, indexBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, resolution.x, resolution.y, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, indexBuffer, 0);

    //Depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution.x, resolution.y);

    //Frame buffer
    GLuint frameBuffer = 0;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, resolution.x, resolution.y);

    //Bind
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, DrawBuffers);
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
    shader.loadShader(GL_VERTEX_SHADER, "src/Shaders/pass.vert");
    shader.loadShader(GL_FRAGMENT_SHADER, "src/Shaders/simple.frag");
    shader.compile();

    createFrameBuffer();
}

Renderer::~Renderer()
{
    glDeleteRenderbuffers(1, &depthBuffer);
    GLuint buffers[2] = { posBuffer, indexBuffer };
    glDeleteBuffers(2, buffers);
    glDeleteFramebuffers(1, &frameBuffer);
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

glm::mat4 Renderer::renderModel(Geometry& geometry, void* posMap, void* indexMap)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPointSize(2.0);
    glLineWidth(2.0);

    shader.enable();
    shader.registerMVP(&MVP[0][0]);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    GLuint buffers[3];
    //vertices
    glGenBuffers(3, buffers);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, geometry.getVerticesCount() * geometry.getVertexSize(), geometry.getVertices(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    //face indices
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    const unsigned int faceCount = geometry.getIndecesCount() / 3;
    unsigned int* faceIndices = new unsigned int[faceCount];
    for (unsigned int i = 1; i <= faceCount; i++)
        faceIndices[i - 1] = i;
    glBufferData(GL_ARRAY_BUFFER, faceCount, faceIndices, GL_STATIC_DRAW);
    delete[] faceIndices;
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_UNSIGNED_INT, GL_FALSE, 0, 0);

    //triangle vertex indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndecesCount() * geometry.getIndexSize(), geometry.getIndices(), GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, (GLsizei)geometry.getIndecesCount(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDeleteBuffers(1, buffers);

    shader.disable();
    glFlush();

    glBindTexture(GL_TEXTURE_2D, indexBuffer);
    glGetTexImage(GL_TEXTURE_2D, 0 , GL_RED_INTEGER, GL_UNSIGNED_INT, indexMap);
    glBindTexture(GL_TEXTURE_2D, posBuffer);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB32F, GL_FLOAT, posMap);
    return MVP;
}
