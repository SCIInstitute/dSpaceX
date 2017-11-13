#include "DisplayGraph.h"
#include "MetricMDS.h"

#include "Precision.h"
#include "LinalgIO.h"
#include "EuclideanMetric.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

/**
 *
 */
DisplayGraph::DisplayGraph(HDVizData *data, TopologyData *topoData, HDVizState *state) : 
    data(data), topoData(topoData), state(state) { 
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

float randf() {
  return static_cast<float>(rand() / (static_cast<float>(RAND_MAX)));
}

void DisplayGraph::setCrystal(int persistenceLevel, int crystalIndex) {
  m_currentLevel = persistenceLevel;
  m_currentCrystal = crystalIndex;

  vertices.clear();
  colors.clear();
  edgeIndices.clear();

  // m_count = data->getNearestNeighbors().N();
  MorseSmaleComplex *complex = topoData->getComplex(persistenceLevel);
  Crystal *crystal = complex->getCrystals()[crystalIndex];
  std::vector<unsigned int> samples = crystal->getAllSamples();
  m_count = samples.size();

  // TODO(jonbronson): Add logic here to combine the subset of samples into a matrix
  //                   and use that as the input for the mds embedding for layout.

  // EuclideanMetric<Precision> metric;
  // MetricMDS<Precision> mds;
  // FortranLinalg::DenseMatrix<Precision> layout = mds.embed(data->getX(), metric, 2);
  // std::cout << "Layout Matrix: " << layout.M() << " x " << layout.N() << std::endl;

  float range = 50.0f;

  for (int i = 0; i < samples.size(); i++) {    
      // vertices.push_back(range*(randf() - 0.5f));   // x
      int sampleIndex = samples[i];
      // int one_dim = std::floor(sqrt(m_count));
      int one_dim = std::floor(sqrt(2000));
      float x_offset = (float)(sampleIndex % one_dim) / (float)one_dim;
      float y_offset = (float)std::floor(sampleIndex / one_dim) / (float)one_dim;

      //vertices.push_back(range*(randf() - 0.5f));
      //vertices.push_back(range*(randf() - 0.5f));   // y
      vertices.push_back(range * (x_offset - 0.5f));
       vertices.push_back(range * (y_offset - 0.5f));
      // vertices.push_back(range*layout(0, i));
      // vertices.push_back(range*layout(1, i));
      vertices.push_back(0.0f);   // z
      // colors.push_back(randf());   // r
      // colors.push_back(randf());   // g
      // colors.push_back(randf());   // b    
      colors.push_back(100.0f / 255.0f); 
      colors.push_back(146.0f / 255.0f);
      colors.push_back(255.0f / 255.0f);
  }

  for (int i = 0; i < data->getNearestNeighbors().N(); i++) {
    for (int j = 0; j < data->getNearestNeighbors().M(); j++) {      
      int neighbor = data->getNearestNeighbors()(j, i);

      std::vector<unsigned int>::iterator iter1 = find (samples.begin(), samples.end(), neighbor);
      std::vector<unsigned int>::iterator iter2 = find (samples.begin(), samples.end(), i);
      if (iter1 != samples.end() && iter2 != samples.end()) {            
        edgeIndices.push_back((GLuint) std::distance(samples.begin(), iter1));
        edgeIndices.push_back((GLuint) std::distance(samples.begin(), iter2));
      }
    }
  }

  // bind new data to buffers
  glBindBuffer(GL_ARRAY_BUFFER, m_positionsVBO);
  glBufferData(GL_ARRAY_BUFFER, m_count*3*sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
  
  glGenBuffers(1, &m_colorsVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
  glBufferData(GL_ARRAY_BUFFER, m_count*3*sizeof(GLfloat), &colors[0], GL_STATIC_DRAW);

  glGenBuffers(1, &m_edgeElementVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_edgeElementVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(GLuint), &edgeIndices[0], GL_DYNAMIC_DRAW);
}

/**
 *
 */
void DisplayGraph::init(){  
  this->setCrystal(state->currentLevel, state->selectedCell);

  // Clear to White.  
  glClearColor(1, 1, 1, 0);

  glDisable(GL_LIGHTING);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_MULTISAMPLE);
  glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

  // Create the VAO
  glGenVertexArrays(1, &m_vertexArrayObject);
  glBindVertexArray(m_vertexArrayObject);

  // Create the VBOs
  glGenBuffers(1, &m_positionsVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_positionsVBO);
  glBufferData(GL_ARRAY_BUFFER, m_count*3*sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
  
  glGenBuffers(1, &m_colorsVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
  glBufferData(GL_ARRAY_BUFFER, m_count*3*sizeof(GLfloat), &colors[0], GL_STATIC_DRAW);

  glGenBuffers(1, &m_edgeElementVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_edgeElementVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(GLuint), &edgeIndices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_positionsVBO);  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  // Enable Vertex Arrays 0 and 1 in the VAO
  glEnableVertexAttribArray(0);  
  glEnableVertexAttribArray(1);

  //glBindVertexArray(0);
  //glBindBuffer(GL_ARRAY_BUFFER, 0);

  compileShaders();
}


void DisplayGraph::compileShaders() {
  compileNodeShaders();
  compileEdgeShaders();
}

void DisplayGraph::compileNodeShaders() {
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
  "uniform mat4 projectionMatrix;                                           "
  "in vec3 color[];                                                         "
  "out vec2 Vertex_UV;                                                      "
  "out vec3 geom_color;                                                     "
  "                                                                         "
  "const float radius = 0.5;                                                "
  "                                                                         "
  "void main() {                                                            "
  "  gl_Position = gl_in[0].gl_Position + vec4  (-1 * radius, -1 * radius, 0.0, 0.0); "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(0.0, 0.0);"
  "  geom_color = color[0];                                                 "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4(-1 * radius,  1 * radius, 0.0, 0.0);  "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(0.0, 1.0);"
  "  geom_color = color[0];                                                 "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4( 1 * radius, -1 * radius, 0.0, 0.0); "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(1.0, 0.0);"
  "  geom_color = color[0];                                                 "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4( 1 * radius,  1 * radius, 0.0, 0.0);  "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(1.0, 1.0);"
  "  geom_color = color[0];                                                 "
  "  EmitVertex();                                                          "
  "                                                                         "    
  "                                                                         "  
  "  EndPrimitive();                                                        "
  "}                                                                        ";

  const char* fragment_shader_src = 
  "#version 150\n"
  "in vec2 Vertex_UV;"
  "in vec3 geom_color;"
  "out vec4 frag_color;"
  "void main() {"
  "  vec2 uv = Vertex_UV.xy;"
  "  vec2 center = vec2(0.5);"
  "  float radius = 0.425;"
  "  float thickness = 0.025;"  
  "  float blur = 0.05;"
  "  float t = distance(uv, center) - radius;"
  "  vec4 fillColor = vec4(1.0, 1.0, 1.0, 1.0);"
  "  vec4 black = vec4(0.0, 0.0, 0.0, 1.0);"
  "  vec4 clear = vec4(1.0, 1.0, 1.0, 0.0);"
  "  vec4 fill = clear;"
  "  if (t < 0.0) {"
  "    t = abs(t);"
  "    fill = vec4(geom_color, 1.0);"
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

  GLint isLinked = 0;
  glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &isLinked);
  if(isLinked == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
    
    GLchar *errorLog = new GLchar[maxLength];    
    glGetProgramInfoLog(m_shaderProgram, maxLength, &maxLength, &errorLog[0]);
    
    glDeleteProgram(m_shaderProgram);    
    std::cout << errorLog << std::endl;
    exit(0);
  }
}

void DisplayGraph::compileEdgeShaders() {
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
  "layout(lines) in;                                                       "
  "layout(triangle_strip, max_vertices = 4) out;                            "
  "                                                                         "
  "uniform mat4 projectionMatrix;                                           "
  "in vec3 color[];                                                         "
  "out vec2 Vertex_UV;                                                      "
  "out vec3 geom_color;                                                     "
  "                                                                         "
  "const float thickness = 0.075;"
  "                                                                         "
  "void main() {                                                            "
  "  vec3 a = vec3(gl_in[0].gl_Position.xy, 0.0);"
  "  vec3 b = vec3(gl_in[1].gl_Position.xy, 0.0);"
  "  vec3 line = b - a;"
  "  vec3 perp = normalize(vec3(-line.y, line.x, 0.0));"
  "  vec3 v1 = a + thickness*perp;"
  "  vec3 v2 = a - thickness*perp;"
  "  vec3 v3 = b + thickness*perp;"
  "  vec3 v4 = b - thickness*perp;"
  "  "
  "  gl_Position = projectionMatrix * vec4(v1, 1);"
  "  Vertex_UV = vec2(0.0, 0.0);"
  "  geom_color = color[0];                                                 "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = projectionMatrix * vec4(v2, 1);"
  "  Vertex_UV = vec2(0.0, 1.0);"
  "  geom_color = color[0];                                                 "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = projectionMatrix * vec4(v3, 1);"
  "  Vertex_UV = vec2(1.0, 0.0);"
  "  geom_color = color[0];                                                 "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = projectionMatrix * vec4(v4, 1);"
  "  Vertex_UV = vec2(1.0, 1.0);"
  "  geom_color = color[0];                                                 "
  "  EmitVertex();                                                          "
  "                                                                         "    
  "                                                                         "  
  "  EndPrimitive();                                                        "
  "}                                                                  ";


  const char* fragment_shader_src = 
  "#version 150\n"
  "in vec2 Vertex_UV;"
  "in vec3 geom_color;"
  "out vec4 frag_color;"
  "void main() {"
  "  float center = 0.5;"
  // "  float thickness = 0.45;"
  "  float thickness = 0.25;"
  "  float blur = 0.2;"
  "  float t = abs(Vertex_UV.y - 0.5);"
  "  vec4 black = vec4(0.0, 0.0, 0.0, 0.15);"
  "  vec4 clear = vec4(1.0, 1.0, 1.0, 0.0);"
  "  float step1 = thickness;"
  "  float step2 = thickness + blur;"  
  "  frag_color = mix(black, clear, smoothstep(step1, step2, t));"
  "}";

  // Compile Vertex Shader
  m_edgeVertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(m_edgeVertexShader, 1, &vertex_shader_src, NULL);
  glCompileShader(m_edgeVertexShader);

  // Check for Vertex Shader Errors
  GLint success = 0;
  glGetShaderiv(m_edgeVertexShader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logSize = 0;
    glGetShaderiv(m_edgeVertexShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar *errorLog = new GLchar[logSize];
    glGetShaderInfoLog(m_edgeVertexShader, logSize, &logSize, &errorLog[0]);

    std::cout << errorLog << std::endl;
    exit(0);
  }

  // Compile Fragment Shader
  m_edgeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(m_edgeFragmentShader, 1, &fragment_shader_src, NULL);
  glCompileShader(m_edgeFragmentShader);

  // Check for Fragment Shader Errors
  glGetShaderiv(m_edgeFragmentShader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logSize = 0;
    glGetShaderiv(m_edgeFragmentShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar *errorLog = new GLchar[logSize];
    glGetShaderInfoLog(m_edgeFragmentShader, logSize, &logSize, &errorLog[0]);

    std::cout << errorLog << std::endl;
    exit(0);
  }

  // Compile Geometry Shader
  m_edgeGeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
  glShaderSource(m_edgeGeometryShader, 1, &geometry_shader_src, NULL);
  glCompileShader(m_edgeGeometryShader);

  // Check for Geometry Shader Errors
  glGetShaderiv(m_edgeGeometryShader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logSize = 0;
    glGetShaderiv(m_edgeGeometryShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar *errorLog = new GLchar[logSize];
    glGetShaderInfoLog(m_edgeGeometryShader, logSize, &logSize, &errorLog[0]);

    std::cout << errorLog << std::endl;
    exit(0);
  }


  m_edgeShaderProgram = glCreateProgram();
  glAttachShader(m_edgeShaderProgram, m_edgeVertexShader);
  glAttachShader(m_edgeShaderProgram, m_edgeFragmentShader);
  glAttachShader(m_edgeShaderProgram, m_edgeGeometryShader);

 
  glBindAttribLocation(m_edgeShaderProgram, 0, "vertex_position");
  glBindAttribLocation(m_edgeShaderProgram, 1, "vertex_color");
  glLinkProgram(m_edgeShaderProgram);

  GLint isLinked = 0;
  glGetProgramiv(m_edgeShaderProgram, GL_LINK_STATUS, &isLinked);
  if(isLinked == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetProgramiv(m_edgeShaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
    
    GLchar *errorLog = new GLchar[maxLength];    
    glGetProgramInfoLog(m_edgeShaderProgram, maxLength, &maxLength, &errorLog[0]);
    
    glDeleteProgram(m_edgeShaderProgram);    
    std::cout << errorLog << std::endl;
    exit(0);
  }  

  std::cout << "Shaders built" << std::endl;
}

/**
 *
 */
void DisplayGraph::setupOrtho(int w, int h) {
  float sx = 1;
  float sy = 1;
  
  if (w > h) {
    sx = (float)w/h;
  } else {
    sy = (float)h/w;
  }

  glOrtho(-1 * sx * m_scale + m_xOffset, // left
           1 * sx * m_scale + m_xOffset, // right
          -1 * sy * m_scale + m_yOffset, // bottom
           1 + sy * m_scale + m_yOffset, // top
           1,    // near
           -1);  // far
}


/**
 *
 */
void DisplayGraph::display(void) {

  if (state->currentLevel != m_currentLevel ||
      state->selectedCell != m_currentCrystal) {
    this->setCrystal(state->currentLevel, state->selectedCell);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);   
  glLoadIdentity();
  //
  

  GLfloat modelViewMatrix[16];
  GLfloat projectionMatrix[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix);
  glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);

  GLuint projectionMatrixID = glGetUniformLocation(m_shaderProgram, "projectionMatrix");

  glBindVertexArray(m_vertexArrayObject);  

  // render edges  
  glBindVertexArray(m_vertexArrayObject);  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_edgeElementVBO);
  glUseProgram(m_edgeShaderProgram);  
  glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, &projectionMatrix[0]);
  glDrawElements(GL_LINES, edgeIndices.size(), GL_UNSIGNED_INT, 0);

  // render nodes
  glUseProgram(m_shaderProgram);
  glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, &projectionMatrix[0]);
  glDrawArrays(GL_POINTS, 0, m_count);

  
  //glBindVertexArray(0); 

  //
  glutSwapBuffers();
}


/**
 *
 */
void DisplayGraph::resetView() {
  m_scale = 20.0f; 
  m_minScale = 0.1f;
  m_maxScale = 100.0f;
  m_scaleFactor = 1.2f;

  m_xOffset = 0;
  m_yOffset = 0;
  
  glMatrixMode(GL_PROJECTION);  
  glLoadIdentity();
  setupOrtho(width, height);
}


/**
 *
 */
void DisplayGraph::keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'q':
    case 'Q':
      exit(0);
      break;
    case ' ':  // spacebar    
      resetView();
      break;
  }
  glutPostRedisplay();
}


/**
 *
 */
void DisplayGraph::mouse(int button, int state, int x, int y) {
  m_previousX = x;
  m_previousY = y;

  if (button == 3 || button == 4) {
    if (state == GLUT_UP) return;

    if (button == 3) {
      m_scale = m_scale / m_scaleFactor;
    } else { 
      m_scale = m_scale * m_scaleFactor;
    }
    m_scale = std::min(std::max(m_scale, m_minScale), m_maxScale);
  }

  if (state == GLUT_DOWN) {    
    m_currentButton = button;    
  } else {
    m_currentButton = -1;
  }

  reshape(width, height);
  glutPostRedisplay();
}


/**
 *
 */
void DisplayGraph::motion(int x, int y) {  
  int dx = x - m_previousX;
  int dy = y - m_previousY;
  m_previousX = x;
  m_previousY = y;
 
  if (m_currentButton == GLUT_RIGHT_BUTTON) {
    m_xOffset -= 0.1*dx;
    m_yOffset += 0.1*dy;
    reshape(width, height);
  }
  glutPostRedisplay();
}
