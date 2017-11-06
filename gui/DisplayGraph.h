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

    int width, height;

    void setupOrtho(int w, int h);
};
