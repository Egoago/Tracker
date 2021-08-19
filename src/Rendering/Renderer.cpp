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

//TODO fuse two functions
mat4f tr::Renderer::getDefaultP() {
    const static auto res = config.getEntries<int>("frame resolution", { 1024, 1024 });
    const static float nearP = config.getEntry("near clipping pane", 200.0f);
    const static float farP = config.getEntry("far clipping pane", 4500.0f);
    const static float fov = radian(config.getEntry("fov", 45.0f));
    const static float aspect = (float)res[0] / res[1];
    return tr::perspective(fov, aspect, nearP, farP);
}

//TODO fuse two functions
void Renderer::readConfig() {
    const auto res = config.getEntries<int>("frame resolution", { 1024, 1024 });
    resolution = uvec2(res[0], res[1]);
    const float nearP = config.getEntry("near clipping pane", 200.0f);
    const float farP = config.getEntry("far clipping pane", 4500.0f);
    const float fov = radian(config.getEntry("fov", 45.0f));
    const float aspect = (float)res[0] / res[1];
    setProj(fov, nearP, farP, aspect);
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
                                    true));
    //Low thresh
    
    pipelines.emplace_back(new Pipeline("edge",
                                    {textureMaps[LPOS], textureMaps[LDIR]},
                                    GL_LINES,
                                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                                    depthBuffer,
                                    true));
    setGeometry(geometry);
}

Renderer::~Renderer()
{
    //TODO proper resource management
    glDeleteRenderbuffers(1, &depthBuffer);
    glutDestroyWindow(glutWindow);
}

template<typename Type>
GLuint upladDataToGPU(const std::vector<Type>& data, int bufferType = GL_ARRAY_BUFFER) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(bufferType, buffer);
    glBufferData(bufferType, data.size() * sizeof(Type), data.data(), GL_STATIC_DRAW);
    return buffer;
}

void Renderer::setGeometry(const Geometry& geometry) {
    if (pipelines.size() < 3) {
        std::cerr << "Have to creae at least 3 pipelines before registering geometry" << std::endl;
        exit(1);
    }
    GLuint VAO;

    //====== Mesh =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //vertices
    upladDataToGPU(geometry.vertices, GL_ARRAY_BUFFER);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //triangle vertex indices
    upladDataToGPU(geometry.indices, GL_ELEMENT_ARRAY_BUFFER);

    //register
    pipelines.at(0)->setGeometry(VAO, (uint)geometry.indices.size());

    //====== Edge buffer ========
    GLuint edgeBuffer = upladDataToGPU(geometry.edges, GL_ARRAY_BUFFER);

    //====== High tresh =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeBuffer);

    //vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3f) * 2, 0);

    //directions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3f) * 2, reinterpret_cast<void*>(sizeof(vec3f)));

    //indices
    upladDataToGPU(geometry.highEdgeIndices, GL_ELEMENT_ARRAY_BUFFER);

    //register
    pipelines.at(1)->setGeometry(VAO, (uint)geometry.highEdgeIndices.size());

    //====== Low Thresh =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, edgeBuffer);

    //vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3f) * 2, 0);

    //directions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3f) * 2, reinterpret_cast<void*>(sizeof(vec3f)));

    //indices
    upladDataToGPU(geometry.lowEdgeIndices, GL_ELEMENT_ARRAY_BUFFER);

    //register
    pipelines.at(2)->setGeometry(VAO, (uint)geometry.lowEdgeIndices.size());
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

