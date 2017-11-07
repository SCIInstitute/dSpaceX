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
    DisplayGraph(HDVizData *data, HDVizState *state);    
    std::string title();
    void reshape(int w, int h);
    void init();
    void printHelp();
    void display(void);
    void keyboard(unsigned char key, int x, int y);
    void mouse(int button, int state, int x, int y);    
    void motion(int x, int y); // catch mouse move events

  private:
    HDVizData *data;
    HDVizState *state;
    
    void setupOrtho(int w, int h);
    int width, height;

    // OpenGL variables
    GLuint m_vertexArrayObject { 0 };
    GLuint m_positionsVBO { 0 };
    GLuint m_colorsVBO { 0 };

    std::vector<GLfloat> vertices = { 0.0f, 0.0f, 0.0f };
    std::vector<GLfloat> colors = {1.0f, 0.0f, 0.0f };

    GLuint m_vertexShader { 0 };
    GLuint m_geometryShader { 0 };
    GLuint m_fragmentShader { 0 };
    GLuint m_shaderProgram { 0 };
};
