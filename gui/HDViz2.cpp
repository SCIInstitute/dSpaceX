#include "Precision.h"

#include "Display.h"
#include "DisplayTubes.h"
#ifdef DIMENSION
#include "DisplayImagePCA.h"
#endif
#include "DisplayRange.h"
#include "DisplayCurves.h"
#include "DisplayMolecule.h"

#include "HDProcessor.h"
#include "HDProcessResult.h"
#include "HDProcessResultSerializer.h"
#include "SimpleHDVizDataImpl.h"
#include <tclap/CmdLine.h>
#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"

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
  // Command line parsing
  TCLAP::CmdLine cmd("HDViz", ' ', "1");

  TCLAP::ValueArg<std::string> xArg("x" /* flag */, "domain" /* name */, 
      "Filename for Data points in domain" /* description */, 
      true /* required */, "", "string");
  cmd.add(xArg);

  TCLAP::ValueArg<std::string> fArg("f" /* flag */, "function" /* name */, 
      "f(x), Filename for function value for each data point in X" /* description */, 
      true /* required */, "", "string");
  cmd.add(fArg);
  
  TCLAP::ValueArg<int> pArg("p" /* flag */, "persistence",
      "Number of persistence levels to compute; all = -1 , default = 20" /* description */, 
      false /* required */, 70 /* default */, "integer");
  cmd.add(pArg);  
  
  TCLAP::ValueArg<int> samplesArg("n" /* flag */, "samples" /* name */,
      "Number of samples for each regression curve, default = 50" /* description */, 
      false /* required */, 50 /* default */,  "integer" /* type */);
  cmd.add(samplesArg);  

  TCLAP::ValueArg<int> knnArg("k" /* flag */, "knn" /* name */,
      "Number of nearest neighbors for Morse-Smale approximation, default = 50" /* description */, 
      false /* required */, 40,  "integer" /* type */);
  cmd.add(knnArg); 

  TCLAP::SwitchArg randArg("r" /* flag */, "random" /* name */, 
      "Adds 0.0001 * range(f) uniform random noise to f, " 
      "in case of 0 gradients due to equivivalent values" /* description */, 
      false /* required */); 
  cmd.add(randArg);

  TCLAP::ValueArg<double> smoothArg("" /* flag */, "smooth" /* name */, 
      "Smooth function values to nearest nieghbor averages" /* description */, 
      false /* required */, 0 /* default */, "double" /* type */); 
  cmd.add(smoothArg);

  TCLAP::ValueArg<Precision> sigmaArg("s" /* flag */, "sigma" /* name */,
      "Kernel regression bandwith (sigma for Gaussian)" /* description */, 
      false /* required */, 0.5 /* default */, "float" /* type */);
  cmd.add(sigmaArg);  
  
  try {
    cmd.parse( argc, argv );
  } catch (TCLAP::ArgException &e) { 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }
  
  // Load Input Data
  // TODO: Move into a data loading library
  FortranLinalg::DenseMatrix<Precision> x = 
      FortranLinalg::LinalgIO<Precision>::readMatrix(xArg.getValue());
  FortranLinalg::DenseVector<Precision> y = 
      FortranLinalg::LinalgIO<Precision>::readVector(fArg.getValue());

  HDProcessResult *result = nullptr;
  try {
    HDProcessor processor;
    result = processor.process(
        x /* domain */,
        y /* function */,
        knnArg.getValue() /* knn */,        
        samplesArg.getValue() /* samples */,
        pArg.getValue() /* persistence */,        
        randArg.getValue() /* random */,
        sigmaArg.getValue() /* sigma */,
        smoothArg.getValue() /* smooth */);
  } catch (const char *err) {
    std::cerr << err << std::endl;
  }

  // Check result and maybe terminate early if something went wrong.
  if (result == nullptr) {
    std::cout << "Something went wrong. Process returned null result." << std::endl;
    return 0;
  }
        
  HDVizData *data = new SimpleHDVizDataImpl(result);
  HDVizState state(data);

  // Init GL stuff. Initialize Visualization Windows
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

  try {  
    std::string fontname = MAKE_STRING(FONTNAME);
    mainD = new DisplayTubes<Precision>(data, &state, fontname);
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
    reinterpret_cast<DisplayTubes<Precision>*>(mainD)->addWindow(mainWindow);

    printHelp();
    glutMainLoop();
  } catch (std::bad_alloc& ba) {
    std::cout << ba.what() << std::endl;
  }

  return 0;
}
