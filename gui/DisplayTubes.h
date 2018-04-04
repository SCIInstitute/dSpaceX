#ifndef DISPLAYTUBES_H
#define DISPLAYTUBES_H

#include "Display.h"
#include "hdprocess/HDVizData.h"
#include "hdprocess/TopologyData.h"
#include "HDVizState.h"
#include "flinalg/DenseVector.h"
#include "flinalg/DenseMatrix.h"
#include "FTGL/FTGLPixmapFont.h"

#include <cstdlib>
#include <string>

#define BUFSIZE 512
#define RENDER_NOTHING         0
#define RENDER_TUBE            1
#define RENDER_FADE_MIN_TO_MAX 2
#define RENDER_FADE_MAX_TO_MIN 3

template<typename TPrecision>
class DisplayTubes : public Display{

  public:
    DisplayTubes(HDVizData *data, TopologyData *topoData, HDVizState *state, std::string fontname);    
    std::string title();
    void reshape(int w, int h);
    void init();
    void printHelp();
    void display(void);
    void keyboard(unsigned char key, int x, int y);
    void mouse(int button, int state, int x, int y);    
    void motion(int x, int y); // catch mouse move events

    // controller methods
    void addWindow(int w);
    void notifyChange();
    void increasePersistanceLevel();        
    void decreasePersistanceLevel();
    void setPersistenceLevel(int pl, bool update = true);
    int getPersistanceLevel();

  private:
    void initState();
    void setFontSize(int fsize);
    void setRenderModesFromPeaks();
    void doPick();
    void setupOrtho(int w, int h);
    void renderTubes(bool selectedOnly = false);
    void renderWidths();
    void renderExtrema();
    void renderMS();

    bool tubesOn;
    bool drawOverlay;
    int width, height;
    Precision dw, dh;

    TPrecision scale;

    // Naviagtion
    TPrecision zoom;
    TPrecision tx;
    TPrecision ty;
    TPrecision rotation[3];
    int rotationAxis;

    bool showPosition; 
    bool showSamples;
    bool extremaOnly;

    // Mouse 
    int last_x;
    int last_y;
    int cur_button;
    int mod;
    int oldN;

    FortranLinalg::DenseVector<bool> selectedCTubes;
    FortranLinalg::DenseVector<int>  renderMode;
    FortranLinalg::DenseVector<bool> selectedPeaks;
    FortranLinalg::DenseVector<bool> selectedTubes;

    HDVizData *data;
    TopologyData *topoData;
    HDVizState *state;
    FTGLPixmapFont font; 
    
    std::vector<int> windows;    
};
#endif
