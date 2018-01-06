#pragma once

#include "HDVizData.h"
#include "HDVizState.h"
#include "Display.h"
#include "DenseVector.h"
#include "DenseMatrix.h"
#include "TopologyData.h"

#include <stdlib.h>
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
    void resetView();

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

    // OpenGL variables
    GLuint m_vertexArrayObject { 0 };
    GLuint m_positionsVBO { 0 };
    GLuint m_colorsVBO { 0 };
    GLuint m_edgeElementVBO { 0 };

    float m_nodeRadius { 0.5 };
    float m_nodeOutline { 0.025 };
    float m_nodeSmoothness { 0.05 };
    float m_edgeThickness { 0.075 };
    float m_edgeSmoothness { 0.1 };
    float m_edgeOpacity { 0.15 };

    int m_count { 0 };

    std::vector<GLuint> edgeIndices;
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> colors;

    GLuint m_vertexShader { 0 };
    GLuint m_geometryShader { 0 };
    GLuint m_fragmentShader { 0 };
    GLuint m_shaderProgram { 0 };

    GLuint m_edgeVertexShader { 0 };
    GLuint m_edgeGeometryShader { 0 };
    GLuint m_edgeFragmentShader { 0 };
    GLuint m_edgeShaderProgram { 0 };

    GLuint imageTextureID { 0 };
};
