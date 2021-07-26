#include "Pipeline.h"
#include <opencv2/core/mat.hpp>
//TODO remove
#include <iostream>

Pipeline::Pipeline(
    const char* name,
    const std::vector<TextureMap*>& textureMaps,
    GLenum drawPrimitive,
    GLbitfield clearMask,
    GLuint depthBuffer) :
    textureMaps(textureMaps),
    drawPrimitive(drawPrimitive),
    clearMask(clearMask) {
    shader = new Shader(name);

    //Frame buffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    //Bind
    unsigned int size = (unsigned int)textureMaps.size();
    GLenum* drawBuffers = new GLenum[size];
    for (unsigned int i = 0; i < size; i++)
        drawBuffers[i] = textureMaps[i]->bind(i);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    if (size > 0)
        glDrawBuffers(size, drawBuffers);
    else
        glDrawBuffer(GL_NONE);
    //glFlush();
    delete[] drawBuffers;
}

Pipeline::~Pipeline()
{
    std::cout << "deleting pipeline " << std::endl;
    glDeleteFramebuffers(1, &frameBuffer);
    delete shader;
}

void Pipeline::render(std::vector<cv::Mat*>& outTextures)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glClear(clearMask);
    shader->enable();
    glBindVertexArray(VAO);
    if (drawPrimitive == GL_LINES)
        glDrawArrays(drawPrimitive, 0, primitiveCount);
    else
        glDrawElements(drawPrimitive, primitiveCount, GL_UNSIGNED_INT, 0);
    shader->disable();
    glFlush();

    for (auto textureMap : textureMaps)
        outTextures.push_back(textureMap->copy());
}