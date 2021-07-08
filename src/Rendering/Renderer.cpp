#include "Renderer.h"
//#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

void Renderer::createFrameBuffer() {
    //Color buffer
    glGenTextures(1, &colorBuffer);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Depth buffer
    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution.x, resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    //Frame buffer
    GLuint frameBuffer = 0;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, resolution.x, resolution.y);

    //Bind
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBuffer, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthBuffer, 0);
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers);
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

void Renderer::setProj(float fov, float nearP, float farP)
{
	Proj = glm::perspective(glm::radians(fov), (float)resolution.x/resolution.y, nearP, farP);
}

void Renderer::setModel(SixDOF sixDOF)
{
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, glm::vec3(sixDOF.position.x,
                                            sixDOF.position.y,
                                            sixDOF.position.z));
	Model = glm::rotate(Model, sixDOF.orientation.p, glm::vec3(1,0,0));
	Model = glm::rotate(Model, sixDOF.orientation.y, glm::vec3(0,1,0));
	Model = glm::rotate(Model, sixDOF.orientation.r, glm::vec3(0,0,1));
}

glm::mat4 Renderer::renderModel(Geometry& geometry, unsigned char* depthMap, unsigned char* colorMap)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPointSize(2.0);
    glLineWidth(2.0);

    shader.enable();
    glm::mat4 mvp = Proj * Model;
    shader.registerMVP(&mvp[0][0]);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    GLuint VB, IB;
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, geometry.getVerticesCount() * geometry.getVertexSize(), geometry.getVertices(), GL_STATIC_DRAW);

    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndecesCount() * geometry.getIndexSize(), geometry.getIndices(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)geometry.getVertexSize(), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (GLsizei)geometry.getVertexSize(), (const GLvoid*)12);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

    glDrawElements(GL_TRIANGLES, (GLsizei)geometry.getIndecesCount(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    shader.disable();
    glFlush();

    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glGetTexImage(GL_TEXTURE_2D, 0 , GL_DEPTH_COMPONENT, GL_FLOAT, depthMap);
    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, colorMap);
    return mvp;
}
