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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
  const char* vertex_shader_src = 
  "in vec3 vertex_position;                     "
  "in vec3 vertex_color;                        "
  "                                             "
  "varying out vec3 color;                      "
  "                                             " 
  "void main() {                                "
  "  color = vertex_color;                      "
  "  gl_Position = vec4(vertex_position, 1.0);  "
  "}                                            ";

  const char* geometry_shader_src = 
  "#version 150 core\n                                                      "
  "layout(points) in;                                                       "
  "layout(triangle_strip, max_vertices = 4) out;                            "
  "                                                                         "
  "out vec2 Vertex_UV;                                                      "
  "                                                                         "
  "const float radius = 0.1;                                                "
  "                                                                         "
  "void main() {                                                            "
  "  gl_Position = gl_in[0].gl_Position + vec4(-1 * radius, -1 * radius, 0.0, 0.0); "  
  "  Vertex_UV = vec2(0.0, 0.0);"
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4(-1 * radius,  1 * radius, 0.0, 0.0);  "  
  "  Vertex_UV = vec2(0.0, 1.0);"
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4( 1 * radius, -1 * radius, 0.0, 0.0); "  
  "  Vertex_UV = vec2(1.0, 0.0);"
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4( 1 * radius,  1 * radius, 0.0, 0.0);  "  
  "  Vertex_UV = vec2(1.0, 1.0);"
  "  EmitVertex();                                                          "
  "                                                                         "    
  "                                                                         "  
  "  EndPrimitive();                                                        "
  "}                                                                        ";

  const char* fragment_shader_src = 
  "#version 150\n"
  "in vec2 Vertex_UV;"
  "in vec3 color;"
  "varying out vec4 frag_color;"
  "void main() {"
  "  vec2 uv = Vertex_UV.xy;"
  "  vec2 c = vec2(0.5);"
  "  float d = distance(uv, c);"
  "  if (d > 0.5) {"
  "     d = 0.0;   "
  "  }"
  "  else if (d < 0.45) {"
  "     d = 0.0;   "
  "  }"
  "  else {"
  "    d = 1.0;"
  "  }"
  "  frag_color = vec4(0.0, 0.0, 0.0, d);"
  "}";

  // Compile Vertex Shader
  m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(m_vertexShader, 1, &vertex_shader_src, NULL);
  glCompileShader(m_vertexShader);

  // Check for Vertex Shader Errors
  GLint success = 0;
  glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logSize = 0;
    glGetShaderiv(m_vertexShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar *errorLog = new GLchar[logSize];
    glGetShaderInfoLog(m_vertexShader, logSize, &logSize, &errorLog[0]);

    std::cout << errorLog << std::endl;
    exit(0);
  }

  // Compile Fragment Shader
  m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(m_fragmentShader, 1, &fragment_shader_src, NULL);
  glCompileShader(m_fragmentShader);

  // Check for Fragment Shader Errors
  glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logSize = 0;
    glGetShaderiv(m_fragmentShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar *errorLog = new GLchar[logSize];
    glGetShaderInfoLog(m_fragmentShader, logSize, &logSize, &errorLog[0]);

    std::cout << errorLog << std::endl;
    exit(0);
  }

  // Compile Geometry Shader
  m_geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
  glShaderSource(m_geometryShader, 1, &geometry_shader_src, NULL);
  glCompileShader(m_geometryShader);

  // Check for Geometry Shader Errors
  glGetShaderiv(m_geometryShader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logSize = 0;
    glGetShaderiv(m_geometryShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar *errorLog = new GLchar[logSize];
    glGetShaderInfoLog(m_geometryShader, logSize, &logSize, &errorLog[0]);

    std::cout << errorLog << std::endl;
    exit(0);
  }


  m_shaderProgram = glCreateProgram();
  glAttachShader(m_shaderProgram, m_vertexShader);
  glAttachShader(m_shaderProgram, m_fragmentShader);
  glAttachShader(m_shaderProgram, m_geometryShader);

  glBindAttribLocation(m_shaderProgram, 0, "vertex_position");
  glBindAttribLocation(m_shaderProgram, 1, "vertex_color");
  glLinkProgram(m_shaderProgram);
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
  glUseProgram(m_shaderProgram);
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
