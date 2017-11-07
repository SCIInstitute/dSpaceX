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
  glEnable(GL_MULTISAMPLE);
  glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

  // int count = data->getNearestNeighbors().N();
  // std::cout << "Initializing graph with " << count << " nodes." << std::endl;

  int count = 3;

  // Create the VBOs
  glGenBuffers(1, &m_positionsVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_positionsVBO);
  glBufferData(GL_ARRAY_BUFFER, count*3*sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
  
  glGenBuffers(1, &m_colorsVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
  glBufferData(GL_ARRAY_BUFFER, count*3*sizeof(GLfloat), &colors[0], GL_STATIC_DRAW);

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
  // "  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
  "}                                            ";

  const char* geometry_shader_src = 
  "#version 150 core\n                                                      "
  "layout(points) in;                                                       "
  "layout(triangle_strip, max_vertices = 4) out;                            "
  "                                                                         "
  "uniform mat4 projectionMatrix;                                           "
  "out vec2 Vertex_UV;                                                      "
  "                                                                         "
  "const float radius = 0.5;                                                "
  "                                                                         "
  "void main() {                                                            "
  "  gl_Position = gl_in[0].gl_Position + vec4  (-1 * radius, -1 * radius, 0.0, 0.0); "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(0.0, 0.0);"
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4(-1 * radius,  1 * radius, 0.0, 0.0);  "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(0.0, 1.0);"
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4( 1 * radius, -1 * radius, 0.0, 0.0); "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(1.0, 0.0);"
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4( 1 * radius,  1 * radius, 0.0, 0.0);  "  
  "  gl_Position = projectionMatrix * gl_Position;"
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
  "out vec4 frag_color;"
  "void main() {"
  "  vec2 uv = Vertex_UV.xy;"
  "  vec2 center = vec2(0.5);"
  "  float radius = 0.45;"
  "  float thickness = 0.01;"  
  "  float blur = 0.05;"
  "  float t = distance(uv, center) - radius;"
  "  vec4 fillColor = vec4(1.0, 1.0, 1.0, 1.0);"
  "  vec4 black = vec4(0.0, 0.0, 0.0, 1.0);"
  "  vec4 clear = vec4(1.0, 1.0, 1.0, 0.0);"
  "  vec4 fill = clear;"
  "  if (t < 0.0) {"
  "    t = abs(t);"
  "    fill = fillColor;"
  "  }"
  "  "
  "  float step1 = thickness;"
  "  float step2 = thickness + blur;"  
  "  frag_color = mix(black, fill, smoothstep(step1, step2, t));"
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
  
  glOrtho(-1*m_scale*sx, 1*m_scale*sx, -1*m_scale*sy, 1*m_scale*sy, 1, -1);  
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

  GLfloat modelViewMatrix[16];
  GLfloat projectionMatrix[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix);
  glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);

  GLuint projectionMatrixID = glGetUniformLocation(m_shaderProgram, "projectionMatrix");
  glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, &projectionMatrix[0]);

  glBindVertexArray(m_vertexArrayObject);  
  glDrawArrays(GL_POINTS, 0, 3);
  
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
  if (button == 3 || button == 4) {
    if (state == GLUT_UP) return;

    if (button == 3) {
      m_scale = m_scale / m_scaleFactor;
    } else { 
      m_scale = m_scale * m_scaleFactor;
    }
    m_scale = std::min(std::max(m_scale, m_minScale), m_maxScale);
  }
  std::cout << "Scale: " << m_scale << std::endl;

  reshape(width, height);
  glutPostRedisplay();
}


/**
 *
 */
void DisplayGraph::motion(int x, int y) {
  glutPostRedisplay();
}
