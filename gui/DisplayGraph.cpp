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

  glDisable(GL_LIGHTING);

  int count = data->getNearestNeighbors().N();
  std::cout << "Initializing graph with " << count << " nodes." << std::endl;

  // Create the VBOs
  glGenBuffers(1, &m_positionsVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_positionsVBO);
  glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
  
  glGenBuffers(1, &m_colorsVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
  glBufferData(GL_ARRAY_BUFFER, 3*sizeof(GLfloat), &colors[0], GL_STATIC_DRAW);

  // Create the VAO
  glGenVertexArrays(1, &m_vertexArrayObject);
  glBindVertexArray(m_vertexArrayObject);
  glBindBuffer(GL_ARRAY_BUFFER, m_positionsVBO);  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  // Enable Vertex Arrays 0 and 1 in the VAO
  glEnableVertexAttribArray(0);  
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);


  // Create Shaders
}


/**
 *
 */
void DisplayGraph::setupOrtho(int w, int h) {
  int sx = 1;
  int sy = 1;
  
  if (w > h) {
    sx = (float)w/h;
  } else {
    sy = (float)h/w;
  }
  
  glOrtho(-sx, sx, -sy, sy, 1, -1);  
}


/**
 *
 */
void DisplayGraph::display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);   
  glLoadIdentity();
  //

  glBindVertexArray(m_vertexArrayObject);  
  glDrawArrays(GL_POINTS, 0, 1);
  
  // glBindVertexArray(0);

  //
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
