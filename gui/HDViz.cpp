#include "Precision.h"

#include "Display.h"
#include "DisplayTubes.h"
#ifdef DIMENSION
#include "DisplayImagePCA.h"
#endif
#include "DisplayRange.h"
#include "DisplayCurves.h"
#include "DisplayMolecule.h"

#include "HDVizData.h"
#include <tclap/CmdLine.h>

#include <iostream>
#include <iomanip>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#define MAKE_STRING_(x) #x
#define MAKE_STRING(x) MAKE_STRING_(x)

Display *mainD, *auxD, *auxD2, *auxD3;


void printHelp() {
  std::cout << mainD->title() << " Window" << std::endl << std::endl;
  mainD->printHelp();	
  std::cout <<  std::endl << std::endl;
}

/**
 * HDVis application entry point.
 */ 
int main(int argc, char **argv) {	
  //Command line parsing
  TCLAP::CmdLine cmd("HDViz", ' ', "1");

  TCLAP::SwitchArg boxArg("b" /* flag */, "boxplots" /* name */,
      "Display domain with boxplots" /* description */, false /* required */);
  cmd.add(boxArg);
 
  TCLAP::SwitchArg curvesArg("c" /* flag */, "curves" /* name */, 
      "Display inverse regression curves" /* description */, false /* required */);
  cmd.add(curvesArg);
  
  TCLAP::SwitchArg molArg("m" /* flag */, "molecule" /* name */, 
      "Display domain as molecule" /* description */, false /* required */);
  cmd.add(molArg);
     
  TCLAP::ValueArg<std::string> fontArg("f" /* flag */, "font" /* name */, 
      "Absolute path of a ttf font" /* description */, false /* required */, "", "");
  cmd.add(fontArg);
  
  try {
    cmd.parse( argc, argv );
  } catch (TCLAP::ArgException &e) { 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }
  
  // GL stuff
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);


  // Load data
  try { 
    HDVizData data;

    std::string fontname = fontArg.getValue();
    if (fontname.size() == 0){
      fontname= MAKE_STRING(FONTNAME);
    }

    //Windows
    mainD = new DisplayTubes<Precision>(&data, fontname);

    #ifdef DIMENSION    
    auxD = new DisplayImagePCA<Image, Precision>(&data, fontname);
    if (!auxD->loadAdditionalData()) {
      return 1;
    }

    #else
   
    if (boxArg.getValue()) {
      auxD = new DisplayRange<Precision>(&data, fontname);
    } else {
      auxD = nullptr;
    }

 
    if (curvesArg.getValue()) {
      auxD2 = new DisplayCurves<Precision>(&data, fontname);
      if (!auxD2->loadAdditionalData()) {
        auxD2 = nullptr;
      }
    } else{
      auxD2 = nullptr;
    }

 
    if (molArg.getValue()) {
      auxD3 = new DisplayMolecule<Precision>(&data, fontname);
      if (!auxD3->loadAdditionalData()) {
        auxD3 = nullptr;
      }
    } else {
      auxD3 = nullptr;
    }
    #endif 

 
    glutInitWindowSize(1000, 1000); 
    int mainWindow = glutCreateWindow(mainD->title().c_str());
    glutDisplayFunc([]() { 
      mainD->display(); 
    });
    glutReshapeFunc([](int w, int h) {
      mainD->reshape(w, h);
    });
    glutMouseFunc([](int button, int state, int x, int y) {
      mainD->mouse(button, state, x, y);
    });
  	glutMotionFunc([](int x, int y) {
      mainD->motion(x, y);
    });
    glutKeyboardFunc([](unsigned char key, int x, int y) {
      mainD->keyboard(key, x, y);
    });
    
    mainD->init();
    data.addWindow(mainWindow);
 
    if (auxD != nullptr) { 
      glutInitWindowSize(500, 500); 
      int auxWindow = glutCreateWindow(auxD->title().c_str());
      glutDisplayFunc([]() {
        auxD->display();
      });
      glutReshapeFunc([](int w, int h) {
        auxD->reshape(w, h);
      });
      glutMouseFunc([](int button, int state, int x, int y) {
        auxD->mouse(button, state, x, y);
      });
      glutMotionFunc([](int x, int y) {
        auxD->motion(x, y);
      });
      glutKeyboardFunc([](unsigned char key, int x, int y) {
        auxD->keyboard(key, x, y);
      });

      auxD->init();
      data.addWindow(auxWindow);
    }

    if (auxD2 != nullptr) {
      glutInitWindowSize(500, 500); 
      int auxWindow2 = glutCreateWindow(auxD2->title().c_str());
      glutDisplayFunc([]() {
        auxD2->display();
      });
      glutReshapeFunc([](int w, int h) {
        auxD2->reshape(w, h);
      });
      glutMouseFunc([](int button, int state, int x, int y) {
        auxD2->mouse(button, state, x, y);
      });
      glutMotionFunc([](int x, int y) {
        auxD2->motion(x, y);
      });
      glutKeyboardFunc([](unsigned char key, int x, int y) {
        auxD2->keyboard(key, x, y);
      });

      auxD2->init();
      data.addWindow(auxWindow2);
    }

    if (auxD3 != nullptr) {
      glutInitWindowSize(500, 500); 
      int auxWindow3 = glutCreateWindow(auxD3->title().c_str());
      glutDisplayFunc([]() {
        auxD3->display();
      });
      glutReshapeFunc([](int w, int h) {
        auxD3->reshape(w, h);
      });
      glutMouseFunc([](int button, int state, int x, int y) {
        auxD3->mouse(button, state, x, y);
      });
      glutMotionFunc([](int x, int y){
        auxD3->motion(x, y);
      });
      glutKeyboardFunc([](unsigned char key, int x, int y) {
        auxD3->keyboard(key, x, y);
      });
      
      auxD3->init();
      data.addWindow(auxWindow3);
    }

    printHelp();
    glutMainLoop();
  } catch (std::bad_alloc& ba) {
    std::cout << ba.what() << std::endl;
  }

  return 0;
}
