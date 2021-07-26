#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Renderer.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <opencv2/core/mat.hpp>
#include "../Misc/Links.h"

extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

ConfigParser Renderer::config(REND_CONFIG_FILE);

void Renderer::readConfig() {
    std::vector<std::string> str = config.getEntries("frame resolution", { "1000", "1000" });
    resolution = glm::uvec2(std::stoi(str[0]), std::stoi(str[1]));
    nearP = std::stof(config.getEntry("near clipping pane", "200.0"));
    farP = std::stof(config.getEntry("far clipping pane", "5000.0"));
    fov = std::stof(config.getEntry("fov", "45.0"));
    aspect = (float)resolution.x / resolution.y;
}

Renderer::Renderer(const Geometry& geometry) {
    //TODO add back-face culling
    int argc=0;
    //TODO argc and argv in release
    glutInit(&argc, nullptr);
    /* can not hide window due to
     * main loop not being called*/
    glutCreateWindow("OpenGL");
    glewInit();
    
    readConfig();

    textureMaps.push_back(new TextureMap(CV_8U, resolution));
    textureMaps.push_back(new TextureMap(CV_32FC3, resolution));
    textureMaps.push_back(new TextureMap(CV_32FC3, resolution));

    // Z buffer
    glEnable(GL_DEPTH_TEST);
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution.x, resolution.y);
    glDepthFunc(GL_LEQUAL);

    //set up pipelines
    pipelines.push_back(new Pipeline("mesh",
                                    std::vector<TextureMap*>{},
                                    GL_TRIANGLES,
                                    GL_DEPTH_BUFFER_BIT,
                                    depthBuffer));
    pipelines.push_back(new Pipeline("highThresh",
                                    {textureMaps.at(0)},
                                    GL_LINES,
                                    GL_COLOR_BUFFER_BIT,
                                    depthBuffer));
    pipelines.push_back(new Pipeline("lowThresh",
                                    {textureMaps.at(1), textureMaps.at(2)},
                                    GL_LINES,
                                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                                    depthBuffer));
    setGeometry(geometry);
    setProj(fov, nearP, farP, aspect);
}

Renderer::~Renderer()
{
    std::cout << "deleting renderer\n";
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void getOutlinerEdges(const Geometry& geometry,
                    std::vector<glm::vec3>& edges,
                    std::vector<glm::vec3>& directions,
                    const float threshold)
{
    edges.clear();
    directions.clear();
    for (unsigned int i = 0; i < geometry.getEdgeCount(); i++)
        if (geometry.getCurvatures()[i] > glm::radians(threshold)) {
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
    if (pipelines.size() < 3) {
        std::cerr << "Have to creae at least 3 pipelines before registering geometry" << std::endl;
        exit(1);
    }
    GLuint VAO;
    //TODO create gpu copy function

    //====== Mesh buffers =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

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
    
    //register
    pipelines.at(0)->setGeometry(VAO, geometry.getIndexCount());

    //====== High tresh buffers =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //generating outliner edges
    std::vector<glm::vec3> edges, directions;
    const float highThreshold = std::stof(config.getEntry("high threshold", "30.0"));
    getOutlinerEdges(geometry, edges, directions, highThreshold);
    std::cout << "edges: " << edges.size() << std::endl;

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, edges.size() * sizeof(glm::vec3), edges.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //register
    pipelines.at(1)->setGeometry(VAO, (unsigned int)edges.size());

    //====== Low Thresh buffers =======
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //generating outliner edges
    const float lowThreshold = std::stof(config.getEntry("low threshold", "1e-3"));
    getOutlinerEdges(geometry, edges, directions, lowThreshold);

    //vertices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, edges.size() * sizeof(glm::vec3), edges.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //directions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, edges.size() * sizeof(glm::vec3), directions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //register
    pipelines.at(2)->setGeometry(VAO, (unsigned int)edges.size());
}

void Renderer::updatePipelines()
{
    for (auto pipeline : pipelines) {
        Shader* shader = pipeline->getShader();
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
	ProjMtx = glm::perspective(glm::radians(fov), aspect, nearP, farP);
    updatePipelines();
}

void Renderer::setModel(SixDOF& sixDOF) {
	ViewModelMtx = glm::mat4(1.0f);
	ViewModelMtx = glm::translate(ViewModelMtx, glm::vec3(sixDOF.position.x,
                                            sixDOF.position.y,
                                            sixDOF.position.z));
	ViewModelMtx = glm::rotate(ViewModelMtx, sixDOF.orientation.p, glm::vec3(1,0,0));
	ViewModelMtx = glm::rotate(ViewModelMtx, sixDOF.orientation.y, glm::vec3(0,1,0));
	ViewModelMtx = glm::rotate(ViewModelMtx, sixDOF.orientation.r, glm::vec3(0,0,1));
    updatePipelines();
}

std::vector<cv::Mat*> Renderer::render() {
    glViewport(0, 0, resolution.x, resolution.y);
    std::vector<cv::Mat*> outTextures;
    for (auto pipeline : pipelines)
        pipeline->render(outTextures);

    return outTextures;
}