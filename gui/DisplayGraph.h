#pragma once

#include "Display.h"
#include "flinalg/DenseVector.h"
#include "flinalg/DenseMatrix.h"
#include "hdprocess/HDVizData.h"
#include "hdprocess/TopologyData.h"
#include "HDVizState.h"

#include <cstdlib>
#include <string>


class DisplayGraph : public Display{

  public:
    DisplayGraph(HDVizData *data, TopologyData *topoData, HDVizState *state);    
    std::string title();
    void reshape(int w, int h);
    void init();
    void printHelp();
    void display(void);
    void keyboard(unsigned char key, int x, int y);
    void mouse(int button, int state, int x, int y);    
    void motion(int x, int y);

    void setCrystal(int persistenceLevel, int crystalIndex);

  private:
    HDVizData *data;
    HDVizState *state;
    TopologyData *topoData;

    void compileShaders();
    void compileNodeShaders();
    void compileEdgeShaders();
    void initTextures();
    void resetView();

    // Image atlas helper functions
    void createGLTexture(const GLuint textureID, const int maxTextureSize, const GLubyte *atlas);
    void buildTextureAtlas(GLubyte *textureAtlas, const std::string imageDir,
      const int thumbnailCount = 1000, const int thumbnailWidth = 80, const int thumbnailHeight = 40,
      const int maxTextureSize = 2048);

    int m_currentLevel = -1;
    int m_currentCrystal = -1;

    // Mouse 
    int m_previousX;
    int m_previousY;
    int m_currentButton;

    float m_scale { 20.0f };
    float m_minScale { 0.1f };
    float m_maxScale { 100.0f };
    float m_scaleFactor = { 1.2f };

    float m_xOffset { 0 };
    float m_yOffset { 0 };
    
    void setupOrtho(int w, int h);
    int width, height;

    bool m_useDebugLayout = false;

    // OpenGL variables
    GLuint m_vertexArrayObject { 0 };
    GLuint m_positionsVBO { 0 };
    GLuint m_colorsVBO { 0 };
    GLuint m_thumbnailsVBO { 0 };
    GLuint m_edgeElementVBO { 0 };

    float m_nodeRadius { 0.5 };
    float m_nodeOutline { 0.025 };
    float m_nodeSmoothness { 0.05 };
    float m_edgeThickness { 0.075 };
    float m_edgeSmoothness { 0.1 };
    float m_edgeOpacity { 0.05 };

    int m_count { 0 };

    std::vector<GLuint> edgeIndices;
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;
    std::vector<GLuint> thumbnails;

    GLuint m_thumbnailFragmentShader { 0 };
    GLuint m_thumbnailShaderProgram { 0 };

    GLuint m_vertexShader { 0 };
    GLuint m_geometryShader { 0 };
    GLuint m_fragmentShader { 0 };
    GLuint m_shaderProgram { 0 };

    GLuint m_edgeVertexShader { 0 };
    GLuint m_edgeGeometryShader { 0 };
    GLuint m_edgeFragmentShader { 0 };
    GLuint m_edgeShaderProgram { 0 };

    GLuint m_activeNodeShader { 0 };

    GLuint *imageTextureID;
};
