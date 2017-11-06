#include "DisplayGraph.h"
#include <iostream>
#include <string>

/**
 *
 */
DisplayGraph::DisplayGraph(HDVizData *data, HDVizState *state) : data(data), state(state) { 
  // intentionally left empty
};


/**
 *
 */
std::string DisplayGraph::title(){
  return "Graph View";
}


/**
 *
 */
void DisplayGraph::printHelp() {
  std::cout << "TODO: Print controls here..." << std::endl;
}


/**
 *
 */
void DisplayGraph::reshape(int w, int h){
  width = w;
  height = h;
  glViewport(0, 0, w, h);       
  glMatrixMode(GL_PROJECTION);  
  glLoadIdentity();
  setupOrtho(w, h);
}


/**
 *
 */
void DisplayGraph::init(){  
  // Clear to White.  
  glClearColor(1, 1, 1, 0);

  int count = data->getNearestNeighbors().N();
  std::cout << "Initializing graph with " << count << " nodes." << std::endl;

  glGenVertexArrays(1, &m_vertexArrayObject);
  glBindVertexArray(m_vertexArrayObject);

  glGenBuffers(1, &m_bufferPositions);
  glBindBuffer(GL_ARRAY_BUFFER, m_bufferPositions);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * count, nullptr, GL_STREAM_DRAW);
  glEnableVertexAttribArray(0);
}


/**
 *
 */
void DisplayGraph::setupOrtho(int w, int h) {

}


/**
 *
 */
void DisplayGraph::display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);   
  glLoadIdentity();

  glutSwapBuffers();
}


/**
 *
 */
void DisplayGraph::keyboard(unsigned char key, int x, int y) {
  glutPostRedisplay();
}


/**
 *
 */
void DisplayGraph::mouse(int button, int state, int x, int y) {
  
}


/**
 *
 */
void DisplayGraph::motion(int x, int y) {
  glutPostRedisplay();
}
