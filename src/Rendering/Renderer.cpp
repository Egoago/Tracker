#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Renderer.hpp"
#include "../Misc/Links.hpp"
#include "../Math/EigenTransform.hpp"

using namespace tr;

extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

ConfigParser Renderer::config(REND_CONFIG_FILE);

mat4f tr::Renderer::getDefaultP() {
    const static auto res = config.getEntries<int>("frame resolution", { 1024, 1024 });
    const static float nearP = config.getEntry("near clipping pane", 200.0f);
    const static float farP = config.getEntry("far clipping pane", 4500.0f);
    const static float fov = radian(config.getEntry("fov", 45.0f));
    const static float aspect = (float)res[0] / res[1];
    return tr::perspective(fov, aspect, nearP, farP);
}

void Renderer::readConfig() {
    const auto res = config.getEntries<int>("frame resolution", { 1024, 1024 });
    resolution = uvec2(res[0], res[1]);
    nearP = config.getEntry("near clipping pane", 200.0f);
    farP = config.getEntry("far clipping pane", 4500.0f);
    const float fov = radian(config.getEntry("fov", 45.0f));
    const float aspect = (float)res[0] / res[1];
    ProjMtx = tr::perspective(fov, aspect, nearP, farP);
}

Renderer::Renderer(const Geometry& geometry) {
    //TODO add back-face culling
    int argc=0;
    glutInit(&argc, nullptr);
    /* can not hide window due to
     * main loop not being called*/
    glutWindow = glutCreateWindow("OpenGL");
    glewInit();
    
    readConfig();

    //Texture for contour mask
    textureMaps.reserve(5);
    textureMaps.emplace_back(new TextureMap(CV_8U, resolution));
    //Four textures for pos and dir maps for both thresholds
    for(uint i = 0; i < 4; i++)
        textureMaps.emplace_back(new TextureMap(CV_32FC3, resolution));

    // Z buffer
    glEnable(GL_DEPTH_TEST);
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution.x(), resolution.y());
    glDepthFunc(GL_LEQUAL);
    //set up pipelines
    pipelines.reserve(3);
    pipelines.emplace_back(new Pipeline("mesh",
                                    { textureMaps[MESH] },
                                    GL_TRIANGLES,
                                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                                    depthBuffer,
                                    true));
    //High thresh
    pipelines.emplace_back(new Pipeline("edge",
                                    {textureMaps[HPOS], textureMaps[HDIR]},
                                    GL_LINES,
                                    GL_COLOR_BUFFER_BIT,
                                    depthBuffer,
                                    false));
    //Low thresh
    
    pipelines.emplace_back(new Pipeline("edge",
                                    {textureMaps[LPOS], textureMaps[LDIR]},
                                    GL_LINES,
                                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                                    depthBuffer,
                                    false));
    setGeometry(geometry);
}

Renderer::~Renderer()
{
    //TODO proper resource management
    glDeleteRenderbuffers(1, &depthBuffer);
    glutDestroyWindow(glutWindow);

}

void Renderer::setGeometry(const Geometry& geometry)
{
    if (pipelines.size() < 3) {
        std::cerr << "Have to creae at least 3 pipelines before registering geometry" << std::endl;
        exit(1);
    }
    GLuint VAO;
    GLuint buffer;
    //TODO create gpu copy function

    //====== Mesh buffers =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, geometry.vertices.size() * sizeof(vec3f), geometry.vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //triangle vertex indices
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.indices.size() * sizeof(uint), geometry.indices.data(), GL_STATIC_DRAW);
    
    //register
    pipelines.at(0)->setGeometry(VAO, (uint)geometry.indices.size());

    //====== High tresh buffers =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, highEdges.size() * sizeof(vec3f), highEdges.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //directions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, highDirections.size() * sizeof(vec3f), highDirections.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //register
    pipelines.at(1)->setGeometry(VAO, (uint)highEdges.size());

    //====== Low Thresh buffers =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, lowEdges.size() * sizeof(vec3f), lowEdges.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //directions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, lowDirections.size() * sizeof(vec3f), lowDirections.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //register
    pipelines.at(2)->setGeometry(VAO, (uint)lowEdges.size());
}

void Renderer::updatePipelines() {
    for (auto& pipeline : pipelines) {
        std::shared_ptr<Shader> shader = pipeline->getShader();
        shader->enable();
        shader->registerFloat4x4("P", ProjMtx.data());
        shader->registerFloat4x4("VM", ViewModelMtx.data());
        shader->registerFloat("near", nearP);
        shader->registerFloat("far", farP);
    }
}

void Renderer::setProj(float fov, float nearP, float farP, float aspect) {
    this->nearP = nearP;
    this->farP = farP;
    setProj(tr::perspective(fov, aspect, nearP, farP));
}

void tr::Renderer::setProj(const mat4f& P) {
    ProjMtx = P;
    updatePipelines();
}

void Renderer::setVM(const mat4f& MV) {
    ViewModelMtx = MV;
    updatePipelines();
}

void Renderer::render() {
    //TODO use PBOs for downloading and double buffering
    //x2~3 model load speedup
    //see notes for details
    //Logger::logProcess(__FUNCTION__);
    glViewport(0, 0, resolution.x(), resolution.y());
    for (auto& pipeline : pipelines)
        pipeline->render();
    glFlush();
    getTextures();
    //Logger::logProcess(__FUNCTION__);
}

std::vector<cv::Mat*> tr::Renderer::getTextures() {
    //Logger::logProcess(__FUNCTION__);
    std::vector<cv::Mat*> outTextures;
    for (auto& textureMap : textureMaps)
        outTextures.push_back(textureMap->copyToCPU());
    //Logger::logProcess(__FUNCTION__);
    return outTextures;
}

