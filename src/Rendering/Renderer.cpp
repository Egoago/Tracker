#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Renderer.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "../Misc/Links.hpp"

using namespace tr;

extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

ConfigParser Renderer::config(REND_CONFIG_FILE);

void Renderer::readConfig() {
    const auto res = config.getEntries<int>("frame resolution", { 1024, 1024 });
    resolution = glm::uvec2(res[0], res[1]);
    nearP = config.getEntry("near clipping pane", 200.0f);
    farP = config.getEntry("far clipping pane", 4500.0f);
    fov = glm::radians(config.getEntry("fov", 45.0f));
    aspect = (float)resolution.x / resolution.y;
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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution.x, resolution.y);
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
    setProj(fov, nearP, farP, aspect);
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
    glBufferData(GL_ARRAY_BUFFER, geometry.getVertexCount() * geometry.getVertexSize(), geometry.getVertices(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //triangle vertex indices
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndexCount() * geometry.getIndexSize(), geometry.getIndices(), GL_STATIC_DRAW);
    
    //register
    pipelines.at(0)->setGeometry(VAO, geometry.getIndexCount());

    //generating outliner edges
    std::vector<glm::vec3> lowEdges, highEdges, lowDirections, highDirections;
    const float lowThreshold = config.getEntry("low threshold", 1e-3f);
    const float highThreshold = config.getEntry("high threshold", 30.0f);
    for (uint i = 0; i < geometry.getEdgeCount(); i++) {
        float curvature = geometry.getCurvatures()[i];
        if (curvature > glm::radians(lowThreshold)) {
            glm::vec3 a = geometry.getEdges()[2 * i];
            glm::vec3 b = geometry.getEdges()[2 * i + 1];
            lowEdges.push_back(a);
            lowEdges.push_back(b);
            glm::vec3 dir = -glm::normalize(a - b);
            //opposite directions are the same
            if (glm::dot(glm::normalize(glm::vec3(0.9f, 0.65f, 0.56f)), dir) < 0.0f)
                dir = -dir;
            lowDirections.push_back(dir);
            lowDirections.push_back(dir);
            if (curvature > glm::radians(highThreshold)) {
                highEdges.push_back(a);
                highEdges.push_back(b);
                highDirections.push_back(dir);
                highDirections.push_back(dir);
            }
        }
    }

    //====== High tresh buffers =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, highEdges.size() * sizeof(glm::vec3), highEdges.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //directions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, highDirections.size() * sizeof(glm::vec3), highDirections.data(), GL_STATIC_DRAW);
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
    glBufferData(GL_ARRAY_BUFFER, lowEdges.size() * sizeof(glm::vec3), lowEdges.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //directions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, lowDirections.size() * sizeof(glm::vec3), lowDirections.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //register
    pipelines.at(2)->setGeometry(VAO, (uint)lowEdges.size());
}

void Renderer::updatePipelines()
{
    for (auto& pipeline : pipelines) {
        std::shared_ptr<Shader> shader = pipeline->getShader();
        shader->enable();
        shader->registerFloat4x4("P", &ProjMtx[0][0]);
        shader->registerFloat4x4("VM", &ViewModelMtx[0][0]);
        shader->registerFloat("near", nearP);
        shader->registerFloat("far", farP);
    }
}

void Renderer::setProj(float fov, float nearP, float farP, float aspect)
{
    this->nearP = nearP;
    this->farP = farP;
	ProjMtx = glm::perspective(fov, aspect, nearP, farP);
    updatePipelines();
}

void Renderer::setVM(const glm::mat4& MV) {
    ViewModelMtx = MV;
    updatePipelines();
}

void Renderer::render() {
    //TODO use PBOs for downloading and double buffering
    //x2~3 model load speedup
    //see notes for details
    //Logger::logProcess(__FUNCTION__);	//TODO remove logging
    glViewport(0, 0, resolution.x, resolution.y);
    for (auto& pipeline : pipelines)
        pipeline->render();
    glFlush();
    getTextures();
    //Logger::logProcess(__FUNCTION__);	//TODO remove logging
}

std::vector<cv::Mat*> tr::Renderer::getTextures() {
    //Logger::logProcess(__FUNCTION__);	//TODO remove logging
    std::vector<cv::Mat*> outTextures;
    for (auto& textureMap : textureMaps)
        outTextures.push_back(textureMap->copyToCPU());
    //Logger::logProcess(__FUNCTION__);	//TODO remove logging
    return outTextures;
}

