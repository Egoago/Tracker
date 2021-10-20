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
    CameraCalibration camCal = readConfig();
    return tr::perspective(camCal.FOV,
                           camCal.aspect,
                           camCal.nearPlane,
                           camCal.farPlane);
}

//TODO fuse two functions
Renderer::CameraCalibration Renderer::readConfig() {
    CameraCalibration camCal;
    auto res = config.getEntries<uint>("resolution", { 128,128});
    camCal.resolution = uvec2(res[0], res[1]);
    camCal.nearPlane = config.getEntry("near clipping pane", 200.0f);
    camCal.farPlane = config.getEntry("far clipping pane", 4500.0f);
    camCal.FOV = radian(config.getEntry("fov", 45.0f));
    camCal.aspect = (float)res[0] / res[1];
    return camCal;
}

Renderer::Renderer(const Geometry& geometry) {
    //TODO add back-face culling
    int argc=0;
    glutInit(&argc, nullptr);
    /* can not hide window due to
     * main loop not being called*/
    glutWindow = glutCreateWindow("OpenGL");
    glewInit();

    camCal = readConfig();

    //Shaders
    Shader meshVert("mesh.vert");
    Shader meshFrag("mesh.frag");
    Shader edgeVert("edge.vert");
    Shader edgeFrag("edge.frag");

    //Texture for contour mask
    textureMaps.reserve(5);
    textureMaps.emplace_back(new TextureMap(CV_8U, camCal.resolution));
    //Four textures for pos and dir maps for both thresholds
    for(uint i = 0; i < 4; i++)
        textureMaps.emplace_back(new TextureMap(CV_32FC3, camCal.resolution));

    // Z buffer
    glEnable(GL_DEPTH_TEST);
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, camCal.resolution.x(), camCal.resolution.y());
    glDepthFunc(GL_LEQUAL);
    //set up pipelines
    pipelines.reserve(3);
    pipelines.emplace_back(new Pipeline(
                                    { textureMaps[MESH] },
                                    { &meshVert, &meshFrag },
                                    GL_TRIANGLES,
                                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                                    depthBuffer,
                                    true));
    //High thresh
    pipelines.emplace_back(new Pipeline(
                                    {textureMaps[HPOS], textureMaps[HDIR]},
                                    { &edgeVert, &edgeFrag },
                                    GL_LINES,
                                    GL_COLOR_BUFFER_BIT,
                                    depthBuffer,
                                    true));
    //Low thresh
    
    pipelines.emplace_back(new Pipeline(
                                    {textureMaps[LPOS], textureMaps[LDIR]},
                                    { &edgeVert, &edgeFrag },
                                    GL_LINES,
                                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                                    depthBuffer,
                                    true));
    setGeometry(geometry);
    setProj(camCal.FOV,
            camCal.aspect,
            camCal.nearPlane,
            camCal.farPlane);
}

Renderer::~Renderer()
{
    //TODO proper resource management
    glDeleteRenderbuffers(1, &depthBuffer);
    glutDestroyWindow(glutWindow);
}

template<typename Type>
GLuint uploadDataToGPU(const std::vector<Type>& data, int bufferType = GL_ARRAY_BUFFER) {
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(bufferType, buffer);
    glBufferData(bufferType, data.size() * sizeof(Type), data.data(), GL_STATIC_DRAW);
    return buffer;
}

void Renderer::setGeometry(const Geometry& geometry) {
    if (pipelines.size() < 3) {
        std::cerr << "Have to create at least 3 pipelines before registering geometry" << std::endl;
        exit(1);
    }
    GLuint VAO;

    //====== Mesh =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //vertices
    uploadDataToGPU(geometry.vertices, GL_ARRAY_BUFFER);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //triangle vertex indices
    uploadDataToGPU(geometry.indices, GL_ELEMENT_ARRAY_BUFFER);

    //register
    pipelines.at(0)->setGeometry(VAO, (uint)geometry.indices.size());

    //====== Edge buffer ========
    GLuint edgeBuffer = uploadDataToGPU(geometry.edges, GL_ARRAY_BUFFER);

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
    uploadDataToGPU(geometry.highEdgeIndices, GL_ELEMENT_ARRAY_BUFFER);

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
    uploadDataToGPU(geometry.lowEdgeIndices, GL_ELEMENT_ARRAY_BUFFER);

    //register
    pipelines.at(2)->setGeometry(VAO, (uint)geometry.lowEdgeIndices.size());

    //Bounding box
    geoBBxRadius = geometry.boundingRadius;
}

void Renderer::updatePipelines() {
    for (auto& pipeline : pipelines) {
        if (scaling) pipeline->uniform("P", (ScaleMtx * ProjMtx).eval());
        else pipeline->uniform("P", ProjMtx);
        pipeline->uniform("VM", ViewModelMtx);
        pipeline->uniform("near", nearP);
        pipeline->uniform("far", farP);
    }
}

void Renderer::setProj(float fov, float aspect, float nearP, float farP) {
    this->nearP = nearP;
    this->farP = farP;
    setProj(tr::perspective(fov, aspect, nearP, farP));
}

void tr::Renderer::setProj(const mat4f& P) {
    ProjMtx = P;
    updatePipelines();
}

/// <summary>
/// Uploads the View-Model transformation matrix.
/// </summary>
/// <param name="VM">View-Model transformation matrix.</param>
/// <returns>translation and scaling paramaters applied, packed in a vector [tr.x, tr.y, sc]</returns>
vec3f Renderer::setVM(const mat4f& VM) {
    ViewModelMtx = VM;
    vec3f scalingParameters(0.0f, 0.0f, 1.0f);
    if (scaling) {
        const Eigen::Matrix<float, 4, 1> cam = VM.col(3);
        const float depth = -cam.hnormalized().z();
        scalingParameters = -(ProjMtx * cam).hnormalized();
        scalingParameters.z() = (depth * tanf(camCal.FOV / 2.0f)) / (geoBBxRadius * sqrtf(2.0f));
        ScaleMtx.setIdentity();
        ScaleMtx *= scale(scalingParameters.z(), scalingParameters.z(), 1.0f);
        ScaleMtx *= translate(scalingParameters.x(), scalingParameters.y(), 0.0f);
    }
    updatePipelines();
    return scalingParameters;
}

void Renderer::render() {
    //TODO use PBOs for downloading and double buffering
    //x2~3 model load speedup
    //see notes for details
    //Logger::logProcess(__FUNCTION__);
    glViewport(0, 0, camCal.resolution.x(), camCal.resolution.y());
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

