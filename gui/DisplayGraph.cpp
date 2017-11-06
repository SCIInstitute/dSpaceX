#include "DisplayGraph.h"
#include <string>


DisplayGraph::DisplayGraph(HDVizData *data, HDVizState *state) : data(data), state(state) { 
  // intentionally left empty
};


std::string DisplayGraph::title(){
  return "Graph View";
}

void DisplayGraph::printHelp() {
  std::cout << "TODO: Print controls here..." << std::endl;
}

void DisplayGraph::reshape(int w, int h){
  width = w;
  height = h;
  glViewport(0, 0, w, h);       
  glMatrixMode(GL_PROJECTION);  
  glLoadIdentity();
  setupOrtho(w, h);
}

void DisplayGraph::init(){  

}

void DisplayGraph::setupOrtho(int w, int h) {

}

void DisplayGraph::display(void) {
  glutSwapBuffers();
}

void DisplayGraph::keyboard(unsigned char key, int x, int y) {
  glutPostRedisplay();
}

void DisplayGraph::mouse(int button, int state, int x, int y) {
  
}


// catch mouse move events
void DisplayGraph::motion(int x, int y) {
  glutPostRedisplay();
}
