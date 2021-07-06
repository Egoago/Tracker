
#include "Renderer.h"
//#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "../Object/AssimpGeometry.h"

Renderer::Renderer(int argc, char** argv, unsigned int width, unsigned int height)
{
    glutInit(&argc, argv);
    resolution = glm::uvec2(width, height);
    glutInitWindowSize(resolution.x, resolution.y);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("Assimp test");
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glEnable(GL_DEPTH_TEST);
    setProj();
    setModel();

    glewInit();
    shader.loadShader(GL_VERTEX_SHADER, "src/Shaders/pass.vert");
    shader.loadShader(GL_FRAGMENT_SHADER, "src/Shaders/simple.frag");
    shader.compile();
}

void Renderer::setProj(float fov, float nearP, float farP)
{
	Proj = glm::perspective(glm::radians(fov), (float)resolution.x/resolution.y, nearP, farP);
}

void Renderer::setModel(float x, float y, float z, float rotateX, float rotateY)
{
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, glm::vec3(x, y, z));
	Model = glm::rotate(Model, rotateY, glm::vec3(0,1,0));
	Model = glm::rotate(Model, rotateX, glm::vec3(1,0,0));
}

void Renderer::renderModel(const char* file)
{
	Geometry* geo = new AssimpGeometry(file);
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
    glBufferData(GL_ARRAY_BUFFER, geo->getVerticesCount() * geo->getVertexSize(), geo->getVertices(), GL_STATIC_DRAW);

    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geo->getIndecesCount() * geo->getIndexSize(), geo->getIndices(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, geo->getVertexSize(), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, geo->getVertexSize(), (const GLvoid*)12);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

    glDrawElements(GL_TRIANGLES, geo->getIndecesCount(), GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    shader.disable();
    glFlush();
}
