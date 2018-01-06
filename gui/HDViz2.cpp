#include "Precision.h"

#include "Display.h"
#include "DisplayTubes.h"
#ifdef DIMENSION
#include "DisplayImagePCA.h"
#endif
#include "DisplayGraph.h"
#include "DisplayRange.h"
#include "DisplayCurves.h"
#include "DisplayMolecule.h"

#include "HDProcessor.h"
#include "HDProcessResult.h"
#include "HDProcessResultSerializer.h"
#include "HDGenericProcessor.h"
#include "SimpleHDVizDataImpl.h"
#include "TopologyData.h"
#include "LegacyTopologyDataImpl.h"
#include <tclap/CmdLine.h>
#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "util/DenseVectorSample.h"
#include "util/csv/csv.h"

#include <cassert>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#define MAKE_STRING_(x) #x
#define MAKE_STRING(x) MAKE_STRING_(x)

Display *mainD, *graphD;

FortranLinalg::DenseMatrix<Precision> loadCSVMatrix(std::string filename) {
  std::ifstream fstream(filename);
  std::string line; 
  std::string token;
  
  std::vector<std::vector<double>> matrix;
  while (std::getline(fstream, line)) {
    std::istringstream ss(line);
    std::vector<double> row;
    while (std::getline(ss, token, ',')) {
      row.push_back(std::stod(token));
    }
    matrix.push_back(row);    
  }

  assert(matrix.size() == matrix[0].size());
  FortranLinalg::DenseMatrix<Precision> m(matrix.size(),matrix.size());
  for (int i = 0; i < matrix.size(); i++) {
    for (int j = 0; j < matrix.size(); j++) {
      m(i,j) = matrix[i][j];
    }
  }

  return m;
}

FortranLinalg::DenseVector<Precision> loadCSVColumn(std::string filename, std::string columnName) {
  io::CSVReader<1> in(filename.c_str());
  in.read_header(io::ignore_extra_column, columnName);
  double value;  
  std::vector<double> vec;
  while (in.read_row(value)) {
    vec.push_back(value);
  }
    
  FortranLinalg::DenseVector<Precision> v(vec.size());
  for (int i = 0; i < vec.size(); i++) {
    v(i) = vec[i];
  }

  return v;
}


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
  
  // Build Sample Vector from Input Data
  std::vector<DenseVectorSample*> samples;
  for (int j=0; j < x.N(); j++) {
    FortranLinalg::DenseVector<Precision> vector(x.M());
    for (int i=0; i < x.M(); i++) {
      vector(i) = x(i, j);
    }
    DenseVectorSample *sample = new DenseVectorSample(vector);
    samples.push_back(sample);
  }

  // Load temporary CSV dataset.
  FortranLinalg::DenseMatrix<Precision> md = 
      loadCSVMatrix("/home/sci/bronson/collab/mukund/dist.csv");
  std::cout << "Truss data contains " << md.N() << " samples." << std::endl;

  FortranLinalg::DenseVector<Precision> mv = 
      loadCSVColumn("/home/sci/bronson/collab/mukund/results.csv", "max stress");  

  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
  DenseVectorEuclideanMetric metric;
  FortranLinalg::DenseMatrix<Precision> distances = 
        genericProcessor.computeDistances(samples, metric);

	//-------------------------------------------------------------
  // Isomap Distance Function
  //-------------------------------------------------------------
  // 1. Begin with an NxN euclidean distance matrix.
  //           and an NxM k-nearest neighbor matrix.
  // 2. Create a new NxN manifold distance matrix by
  //    only copying values if the neighbor pair exists
  //    in the k-nearerst neighbor matrix.
  // 3. Fill in missing distances using bread-first search.
  //-------------------------------------------------------------
  int n = md.N();
  int knn = knnArg.getValue();
  auto KNN = FortranLinalg::DenseMatrix<int>(knn, n);
  auto KNND = FortranLinalg::DenseMatrix<Precision>(knn, n);
  Distance<Precision>::findKNN(md, KNN, KNND);



  // Create Sparse Matrix of KNN distances.
	FortranLinalg::DenseMatrix<Precision> isomapDistance(n, n);
  // Initialize Distances
  
  for (int i=0; i < isomapDistance.M(); i++) {
    for (int j=0; j < isomapDistance.N(); j++) {
      isomapDistance(i,j) = (i==j) ? 0 : 1e12;
    }
  }

  for (int i=0; i < KNN.N(); i++) {
    for (int k=0; k < KNN.M(); k++) {
      int node1 = i;
      int node2 = KNN(k, i);
      isomapDistance(node1, node2) = KNND(k, i);
      isomapDistance(node2, node1) = KNND(k, i);
    }
  }

  // Go back through and compute shortest distances.
  // Add each vertex to the active set of intermediate pairs.
  for (int k=0; k < n; k++) {
    std::cout << "Floyd–Warshall progress: " << (float)k*100.0f/(float)n << "%" << std::endl;
    // Pick all vertices as source one by one
    for (int i=0; i < n; i++) {
      // Pick all vertices for destination of the above picked source.
      for (int j=0; j < n; j++) {
        // If vertex k is on the shortest path from i to j, then update
        // the value of dist[i][j]
        double pathDistance = isomapDistance(i,k) + isomapDistance(k,j);
        if (pathDistance < isomapDistance(i,j)) {
          isomapDistance(i,j) = pathDistance;
        }
      }
    }
  }
  
  
  //-------------------------------------------------------------
  // End Isomap Logic
  //-------------------------------------------------------------

  HDProcessResult *result = nullptr;  
  try {        
    result = genericProcessor.processOnMetric(
        isomapDistance, // md /* distance matrix */,
        mv, // y /* qoi */,
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
  TopologyData *topoData = new LegacyTopologyDataImpl(data);
  HDVizState state(data, isomapDistance); // distances);

  // Init GL stuff. Initialize Visualization Windows
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);

  try {  
    std::string fontname = MAKE_STRING(FONTNAME);
    mainD = new DisplayTubes<Precision>(data, topoData, &state, fontname);
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

    // Create Graph Window
    graphD = new DisplayGraph(data, topoData, &state);
    int graphWindow = glutCreateWindow(graphD->title().c_str());
    glutDisplayFunc([]() {
      graphD->display();
    });
    glutReshapeFunc([](int w, int h) {
      graphD->reshape(w, h);
    });
    glutMouseFunc([](int button, int state, int x, int y) {
      graphD->mouse(button, state, x, y);
    });
    glutMotionFunc([](int x, int y) {
      graphD->motion(x, y);
    });
    glutKeyboardFunc([](unsigned char key, int x, int y) {
      graphD->keyboard(key, x, y);
    });
    graphD->init();
    reinterpret_cast<DisplayTubes<Precision>*>(mainD)->addWindow(graphWindow);

    printHelp();
    glutMainLoop();
  } catch (std::bad_alloc& ba) {
    std::cout << ba.what() << std::endl;
  }

  return 0;
}
