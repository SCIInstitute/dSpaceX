#include "DisplayGraph.h"

#include "dimred/MetricMDS.h"
#include "flinalg/LinalgIO.h"
#include "hdprocess/util/csv/loaders.h"
#include "metrics/EuclideanMetric.h"
#include "precision/Precision.h"

#include <png.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>


// FortranLinalg::DenseMatrix<Precision> tSneLayout tSNE_embedding(
//     FortranLinalg::DenseMatrix<Precision> dsubset, int dimensions) {

// }

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
 *    std::string imagesPathPrefix = "/home/sci/bronson/collab/shireen/new/xs/";

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

  // load precomputed layout data from t-SNE 
  FortranLinalg::DenseMatrix<Precision> tSneLayout = 
      //HDProcess::loadCSVMatrix("/home/sci/bronson/collab/mukund/tsne-layout-fnorm.csv");
      //HDProcess::loadCSVMatrix("/home/sci/bronson/collab/shireen/new/tsne_embedding.txt");
      //HDProcess::loadCSVMatrix("/home/sci/bronson/collab/shireen/new/tsne_embedding.txt");
        HDProcess::loadCSVMatrix("../../examples/cantilever_beam/CantileverBeam_tsne_layout.csv");
  
  m_currentLevel = persistenceLevel;
  m_currentCrystal = crystalIndex;

  vertices.clear();
  colors.clear();
  thumbnails.clear();
  edgeIndices.clear();

  // m_count = data->getNearestNeighbors().N();
  MorseSmaleComplex *complex = topoData->getComplex(persistenceLevel);
  Crystal *crystal = complex->getCrystals()[crystalIndex];
  std::vector<unsigned int> samples = crystal->getAllSamples();
  m_count = samples.size();

  // TODO(jonbronson): Add logic here to combine the subset of samples into a matrix
  //                   and use that as the input for the mds embedding for layout.
  auto &X = data->getX();
  auto &Y = data->getY();

  std::cout << "crystal->getAllSamples().size() = " << m_count << std::endl;
  std::cout << "xSubset is " << X.M() << " x " << samples.size() << " matrix." << std::endl;
  std::cout << "ySubset is a " << samples.size() << " elements long vector" << std::endl;
  std::cout << "X is a " << X.M() << " x " << X.N() << " matrix." << std::endl;


  FortranLinalg::DenseMatrix<Precision> xSubset(X.M(), samples.size());
  FortranLinalg::DenseVector<Precision> ySubset(samples.size());  
  

  for (int i=0; i < samples.size(); i++) {
     for (int j=0; j < X.M(); j++) {    
       // xSubset(j, i) = X(j, samples[i]);
     }
     ySubset(i) = Y(samples[i]);
  }


  FortranLinalg::DenseMatrix<Precision> dSubset(samples.size(), samples.size());
  for (int i=0; i < samples.size(); i++) {
    for (int j=0; j < samples.size(); j++) {    
      dSubset(j, i) = state->distances(samples[j], samples[i]);
    }    
  }

  // Normalize Y values for colormap.
  // float min_value = ySubset(0);
  // float max_value = ySubset(0);
  // for (int i=0; i < samples.size(); i++) {
  //   if (ySubset(i) < min_value) {
  //     min_value = ySubset(i);      
  //   }
  //   if (ySubset(i) > max_value) {
  //     max_value = ySubset(i);
  //   }
  // }
  // for (int i=0; i < samples.size(); i++) {
  //   ySubset(i) = (ySubset(i) - min_value) / (max_value - min_value);
  // }
  // std::cout << "current level: " << m_currentLevel << std::endl;
  ColorMapper<Precision> colorMapper = data->getColorMap(m_currentLevel);

  EuclideanMetric<Precision> metric;
  MetricMDS<Precision> mds;

  std::cout << "dSubset size = " << dSubset.M() << " x " << dSubset.N() << std::endl;
  FortranLinalg::DenseMatrix<Precision> layout =  
      FortranLinalg::DenseMatrix<Precision>(tSneLayout.N(), tSneLayout.M());
  for (int i=0; i < tSneLayout.M(); i++) {
    for (int j=0; j < tSneLayout.N(); j++) {
      layout(j,i) = tSneLayout(i,j);
    }
  }

  if (dSubset.N() == 1 && false) {
    std::cout << "Single element crystal. Using special rules for layout" << std::endl;
    layout = FortranLinalg::DenseMatrix<Precision>(2,1);
    layout(0,0) = 0;
    layout(1,0) = 0;
  } else {
    // layout = mds.embed(dSubset, 2);  
    // layout = tSNE_embedding(dsubset, 2);
    // layout = mds.embed(state->distances, 2);
    // layout = mds.embed(xSubset, metric, 2);
    // layout = mds.embed(X, metric, 2);
    std::cout << "Layout Matrix: " << layout.M() << " x " << layout.N() << std::endl;

    // scale to range of [0,1]
    float minX=layout(0,0);
    float maxX=layout(0,0);
    float minY=layout(0,0);
    float maxY=layout(0,0);
    for (int i=0; i < layout.N(); i++) {
      // std::cout << "layout(" << i << "):   x=" << layout(0,i) << ", y=" << layout(1,i) << std::endl;
      minX = layout(0,i) < minX ? layout(0,i) : minX;
      maxX = layout(0,i) > maxX ? layout(0,i) : maxX;
      minY = layout(1,i) < minY ? layout(1,i) : minY;
      maxY = layout(1,i) > maxY ? layout(1,i) : maxY;
    }
    for (int i=0; i < layout.N(); i++) {
      layout(0,i) = (layout(0,i) - minX) / (maxX - minX) - 0.5;
      layout(1,i) = (layout(1,i) - minY) / (maxY - minY) - 0.5;
    }
  }

  std::cout << "layout.N() = " << layout.N() << "" << std::endl;
  std::cout << "samples.size() = " << samples.size() << std::endl;

  float range = 50.0f;

  for (int i = 0; i < samples.size(); i++) {    
      int sampleIndex = samples[i];
      int one_dim = std::floor(sqrt(2000));
      float x_offset = (float)(sampleIndex % one_dim) / (float)one_dim;
      float y_offset = (float)std::floor(sampleIndex / one_dim) / (float)one_dim;

      if(m_useDebugLayout) {
        vertices.push_back(range * (x_offset - 0.5f));
        vertices.push_back(range * (y_offset - 0.5f));
      } else {
         //vertices.push_back(range*layout(0, i));
         //vertices.push_back(range*layout(1, i));
        vertices.push_back(range*layout(0, sampleIndex));
        vertices.push_back(range*layout(1, sampleIndex));
      }
      // vertices.push_back(range*layout(0, samples[i]));
      // vertices.push_back(range*layout(1, samples[i]));
      vertices.push_back(0.0f);       // z
      // colors.push_back(randf());   // r
      // colors.push_back(randf());   // g
      // colors.push_back(randf());   // b    
      // colors.push_back(100.0f / 255.0f); 
      // colors.push_back(146.0f / 255.0f);
      // colors.push_back(255.0f / 255.0f);
      // colors.push_back(100.0f / 255.0f); 
      // colors.push_back(146.0f / 255.0f);
      // colors.push_back(255.0f / 255.0f);

      std::vector<Precision> color = colorMapper.getColor(Y(sampleIndex));
      colors.push_back(color[0]);      
      colors.push_back(color[1]);
      colors.push_back(color[2]);
      thumbnails.push_back(sampleIndex);
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
  glBufferData(GL_ARRAY_BUFFER, m_count*3*sizeof(GLfloat), &vertices[0], GL_DYNAMIC_DRAW);
    
  glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
  glBufferData(GL_ARRAY_BUFFER, m_count*3*sizeof(GLfloat), &colors[0], GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_thumbnailsVBO);
  glBufferData(GL_ARRAY_BUFFER, m_count*sizeof(GLuint), &thumbnails[0], GL_DYNAMIC_DRAW);
  
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

  glGenBuffers(1, &m_thumbnailsVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_thumbnailsVBO);
  glBufferData(GL_ARRAY_BUFFER, m_count*sizeof(GLuint), &thumbnails[0], GL_STATIC_DRAW);

  glGenBuffers(1, &m_edgeElementVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_edgeElementVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, edgeIndices.size() * sizeof(GLuint), &edgeIndices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, m_positionsVBO);  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, m_colorsVBO);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, m_thumbnailsVBO);
  glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, NULL);

  // Enable Vertex Arrays 0 and 1 in the VAO
  glEnableVertexAttribArray(0);  
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  //glBindVertexArray(0);
  //glBindBuffer(GL_ARRAY_BUFFER, 0);

  std::cout << "Compiling GLSL shaders..." << std::endl;
  compileShaders();

  std::cout << "Loading Textures and Building Texture Atlas..." << std::endl;
  initTextures();
}

void DisplayGraph::initTextures() {  
  int thumbnailCount = 100;

  const int MAX_TEXTURE_SIZE = 2048;
  const int THUMBNAIL_WIDTH = 80;
  const int THUMBNAIL_HEIGHT = 40;

  int thumbnailsPerTextureRow = MAX_TEXTURE_SIZE / THUMBNAIL_WIDTH;
  int thumbnailsPerTextureCol = MAX_TEXTURE_SIZE / THUMBNAIL_HEIGHT;
  int thumbnailsPerTexture = thumbnailsPerTextureRow * thumbnailsPerTextureCol;
  int atlasCount = std::ceil(thumbnailCount / (float)thumbnailsPerTexture);

  std::cout << "Max Texture size: " << MAX_TEXTURE_SIZE << "x" << MAX_TEXTURE_SIZE << std::endl;
  std::cout << "Thumbnail size: " << THUMBNAIL_WIDTH << "x" << THUMBNAIL_HEIGHT << std::endl;
  std::cout << "Thumbnails per row: " << thumbnailsPerTextureRow << std::endl;
  std::cout << "Thumbnails per height: " << thumbnailsPerTextureCol << std::endl;
  std::cout << "Thumbnails per texture: " << thumbnailsPerTexture << std::endl;

  std::cout << "Building a Texture Atlas of " << thumbnailCount 
            << " thumbnails will require a total of " << atlasCount << " textures "
            << " of size " << MAX_TEXTURE_SIZE << "x" << MAX_TEXTURE_SIZE << std::endl;

  // Allocating Memory;
  GLubyte *textureAtlas = new GLubyte[MAX_TEXTURE_SIZE*MAX_TEXTURE_SIZE*4];

  // ----
  imageTextureID = new GLuint[atlasCount];
  for (int i=0; i < MAX_TEXTURE_SIZE; i++) {
    for (int j=0; j < MAX_TEXTURE_SIZE; j++) {
      textureAtlas[4*(MAX_TEXTURE_SIZE*j + i) + 0] = 255;
      textureAtlas[4*(MAX_TEXTURE_SIZE*j + i) + 1] = 255;
      textureAtlas[4*(MAX_TEXTURE_SIZE*j + i) + 2] = 255;
      textureAtlas[4*(MAX_TEXTURE_SIZE*j + i) + 3] = 255;
    }
  }
  glGenTextures(atlasCount, imageTextureID);

  buildTextureAtlas(textureAtlas, "../../examples/cantilever_beam/images/");
  createGLTexture(imageTextureID[0], MAX_TEXTURE_SIZE, textureAtlas);
}

void DisplayGraph::createGLTexture(const GLuint textureID, const int maxTextureSize, const GLubyte *atlas) {
  glBindTexture(GL_TEXTURE_2D, textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, true /*hasAlpha*/ ? 4 : 3, maxTextureSize,
               maxTextureSize, 0, true /*hasAlpha*/ ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
               atlas);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void DisplayGraph::buildTextureAtlas(GLubyte *textureAtlas, const std::string imageDir,
  const int thumbnailCount, const int thumbnailWidth, const int thumbnailHeight,
  const int maxTextureSize) {
  if(textureAtlas == nullptr)
    return;

  int thumbnailsPerTextureRow = maxTextureSize / thumbnailWidth;
  int thumbnailsPerTextureCol = maxTextureSize / thumbnailHeight;
  int thumbnailsPerTexture = thumbnailsPerTextureRow * thumbnailsPerTextureCol;

  for (int i = 0; i < thumbnailCount; i++) {
    std::string imagesPathPrefix = imageDir;
    std::string pngSuffix = ".png";
    std::string filename = imagesPathPrefix + std::to_string(i+1) + pngSuffix;
    std::cout << "Loading image: " << filename << std::endl;

    Image image = m_imageLoader.loadImage(filename, ImageLoader::Format::PNG);

    // Copy texture into atlas
    int atlasOffsetY = i / thumbnailsPerTextureRow;
    int atlasOffsetX = i % thumbnailsPerTextureRow;
    int y = (atlasOffsetY * thumbnailHeight);
    int x = (atlasOffsetX * thumbnailWidth);
    for (int h=0; h < image.getHeight(); h++) {
      for (int w=0; w < image.getWidth(); w++) {        
        int index = ((y+h)*maxTextureSize) + x + w;
        // TODO: Move pixel lookup logic into Image class method.
        textureAtlas[4*index+0] = image.getData()[4*(image.getWidth()*h + w) + 0];
        textureAtlas[4*index+1] = image.getData()[4*(image.getWidth()*h + w) + 1];
        textureAtlas[4*index+2] = image.getData()[4*(image.getWidth()*h + w) + 2];
      }
    }
  }
}

void DisplayGraph::compileShaders() {
  compileNodeShaders();
  compileEdgeShaders();
}

void DisplayGraph::compileNodeShaders() {
  // Create Shaders
  const char* vertex_shader_src =
  "#version 150 core\n                          "
  "in vec3 vertex_position;                     "
  "in vec3 vertex_color;                        "
  "in uint vertex_thumbnail;                    "
  "                                             "
  "out vec3 color;                              "
  "flat out uint thumbnail;                     "
  "                                             " 
  "void main() {                                "
  "  color = vertex_color;                      "
  "  thumbnail = vertex_thumbnail;              "
  "  gl_Position = vec4(vertex_position, 1.0);  "
  "}                                            ";


  const char* geometry_shader_src = 
  "#version 150 core\n                                                      "
  "layout(points) in;                                                       "
  "layout(triangle_strip, max_vertices = 4) out;                            "
  "                                                                         "
  "uniform float nodeRadius;                                                "
  "uniform mat4 projectionMatrix;                                           "
  "in vec3 color[];                                                         "
  "flat in uint thumbnail[];                                                "
  "out vec2 Vertex_UV;                                                      "
  "out vec3 geom_color;                                                     "
  "flat out uint geom_thumbnail;                                            "
  "                                                                         "
  "const float radius = 0.5;                                                "
  "                                                                         "
  "void main() {                                                            "
  "  gl_Position = gl_in[0].gl_Position + vec4  (-1 * nodeRadius, -1 * nodeRadius, 0.0, 0.0); "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(0.0, 0.0);"
  "  geom_color = color[0];                                                 "
  "  geom_thumbnail = thumbnail[0];                                         "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4(-1 * nodeRadius,  1 * nodeRadius, 0.0, 0.0);  "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(0.0, 1.0);"
  "  geom_color = color[0];                                                 "
  "  geom_thumbnail = thumbnail[0];                                         "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4( 1 * nodeRadius, -1 * nodeRadius, 0.0, 0.0); "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(1.0, 0.0);"
  "  geom_color = color[0];                                                 "
  "  geom_thumbnail = thumbnail[0];                                         "
  "  EmitVertex();                                                          "
  "                                                                         "  
  "  gl_Position = gl_in[0].gl_Position + vec4( 1 * nodeRadius,  1 * nodeRadius, 0.0, 0.0);  "  
  "  gl_Position = projectionMatrix * gl_Position;"
  "  Vertex_UV = vec2(1.0, 1.0);"
  "  geom_color = color[0];                                                 "
  "  geom_thumbnail = thumbnail[0];                                         "
  "  EmitVertex();                                                          "
  "                                                                         "    
  "                                                                         "  
  "  EndPrimitive();                                                        "
  "}                                                                        ";

  const char* fragment_shader_src = 
  "#version 150\n"
  "uniform float nodeOutline;                                               "
  "uniform float nodeSmoothness;                                            "
  "in vec2 Vertex_UV;"
  "in vec3 geom_color;"
  "out vec4 frag_color;"
  "void main() {"
  "  vec2 uv = Vertex_UV.xy;"
  "  vec2 center = vec2(0.5);"
  "  float radius = 0.425;"
  "  float thickness = nodeOutline;"  // 0.025;"  
  "  float blur = nodeSmoothness;"    //0.05;"
  "  float t = distance(uv, center) - radius;"
  "  vec4 fillColor = vec4(1.0, 1.0, 1.0, 1.0);"
  "  vec4 black = vec4(0.0, 0.0, 0.0, 1.0);"
  "  vec4 lineColor = vec4(mix(black.xyz, geom_color, 0.4), 1.0);"
  "  vec4 clear = vec4(1.0, 1.0, 1.0, 0.0);"
  "  vec4 fill = clear;"
  "  if (t < 0.0) {"
  "    t = abs(t);"
  "    fill = vec4(geom_color, 1.0);"
  "  }"
  "  "
  "  float step1 = thickness;"
  "  float step2 = thickness + blur;"  
  "  frag_color = mix(lineColor, fill, smoothstep(step1, step2, t));"
  "}                                                             ";
  
  const char* thumbnail_shader_src = 
  "#version 150\n                                                           "
  "uniform float nodeOutline;                                               "
  "uniform float nodeSmoothness;                                            "
  "uniform sampler2D imageTex;                                              "
  "in vec2 Vertex_UV;                                                       "
  "in vec3 geom_color;                                                      "
  "flat in uint geom_thumbnail;                                             "
  "out vec4 frag_color;                                                     "
  "                                                                         "
  "void main() {                                                            "
  "  vec2 uv = Vertex_UV.xy;                                                "
  "  int MAX_TEXTURE_SIZE = 2048;                                           "
  "  int THUMBNAIL_WIDTH = 80;                                              "
  "  int THUMBNAIL_HEIGHT = 40;                                             "  
  "  int thumbnailsPerTextureRow = 25;                                      "
  "  float aspect_ratio = float(THUMBNAIL_HEIGHT) / float(THUMBNAIL_WIDTH); "
  "  float inv_aspect = 1.0 / aspect_ratio;                                 "
  "  float uscale = float(THUMBNAIL_WIDTH) / float(MAX_TEXTURE_SIZE);       "
  "  float vscale = float(THUMBNAIL_HEIGHT) / float(MAX_TEXTURE_SIZE);      "
  "  int atlasOffsetX = int(geom_thumbnail) % thumbnailsPerTextureRow;      "  
  "  int atlasOffsetY = int(geom_thumbnail) / thumbnailsPerTextureRow;      "
  "                                                                         "
  "  // Account for Thumbnail Aspect Ratio - Scale to Fit\n                 "
  "  float aspect_u = uv.x;                                                 "  
  "  float aspect_v = (uv.y - 0.5*(1 - aspect_ratio)) / aspect_ratio;       "
  "                                                                         "
  "  // Account for Lookup into Texture Atlas\n                             "
  "  float atlas_u = (atlasOffsetX + aspect_u) * uscale;                    "
  "  float atlas_v = (atlasOffsetY + aspect_v) * vscale;                    "
  "  vec4 black = vec4(0.0, 0.0, 0.0, 1.0);                                 "
  "  vec4 transparent = vec4(0.0, 0.0, 0.0, 0.0);                           "
  "  vec4 lineColor = vec4(mix(black.xyz, geom_color, 0.4), 1.0);           "
  "  frag_color = vec4(texture2D(imageTex, vec2(atlas_u,atlas_v)).rgb, 1);  "
  "                                                                         "
  "  // Add QoI outline color\n                                             "
  "  if (aspect_u <= nodeOutline || aspect_u >= (1- nodeOutline) ||         "
  "      aspect_v <= inv_aspect*nodeOutline ||                              "
  "      aspect_v >= (1 - inv_aspect*nodeOutline)) {                        "
  "    frag_color = vec4(geom_color, 1.0);                                  "
  "  }                                                                      "
  "                                                                         "
  "  // Make area outside of thumbnail transparent.\n                       "
  "  if (aspect_u < 0 || aspect_u > 1 || aspect_v < 0 || aspect_v > 1) {    "
  "    frag_color = transparent;                                            "
  "  }                                                                      "
  "}                                                                        ";



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

  // Compile Thumbnail Shader
  m_thumbnailFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(m_thumbnailFragmentShader, 1, &thumbnail_shader_src, NULL);
  glCompileShader(m_thumbnailFragmentShader);

   // Check for Thumbnail Shader Errors
  glGetShaderiv(m_thumbnailFragmentShader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint logSize = 0;
    glGetShaderiv(m_thumbnailFragmentShader, GL_INFO_LOG_LENGTH, &logSize);
    GLchar *errorLog = new GLchar[logSize];
    glGetShaderInfoLog(m_thumbnailFragmentShader, logSize, &logSize, &errorLog[0]);
    std::cout << errorLog << std::endl;
    exit(0);
  }

  // Compile Shader Program
  m_shaderProgram = glCreateProgram();
  glAttachShader(m_shaderProgram, m_vertexShader);
  glAttachShader(m_shaderProgram, m_fragmentShader);
  glAttachShader(m_shaderProgram, m_geometryShader);

  glBindAttribLocation(m_shaderProgram, 0, "vertex_position");
  glBindAttribLocation(m_shaderProgram, 1, "vertex_color");
  glBindAttribLocation(m_shaderProgram, 2, "vertex_thumbnail");
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

  // Compile Thumbnail Shader Program
  m_thumbnailShaderProgram = glCreateProgram();
  glAttachShader(m_thumbnailShaderProgram, m_vertexShader);
  glAttachShader(m_thumbnailShaderProgram, m_thumbnailFragmentShader);
  glAttachShader(m_thumbnailShaderProgram, m_geometryShader);
  glBindAttribLocation(m_thumbnailShaderProgram, 0, "vertex_position");
  glBindAttribLocation(m_thumbnailShaderProgram, 1, "vertex_color");
  glBindAttribLocation(m_thumbnailShaderProgram, 2, "vertex_thumbnail");
  glLinkProgram(m_thumbnailShaderProgram);

  isLinked = 0;
  glGetProgramiv(m_thumbnailShaderProgram, GL_LINK_STATUS, &isLinked);
  if(isLinked == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetProgramiv(m_thumbnailShaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
    
    GLchar *errorLog = new GLchar[maxLength];    
    glGetProgramInfoLog(m_thumbnailShaderProgram, maxLength, &maxLength, &errorLog[0]);
    
    glDeleteProgram(m_thumbnailShaderProgram);    
    std::cout << errorLog << std::endl;
    exit(0);
  }

  // Set Active Node Shader
  m_activeNodeShader = m_shaderProgram;
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
  "uniform float edgeThickness;                                            "
  "in vec3 color[];                                                         "
  "out vec2 Vertex_UV;                                                      "
  "out vec3 geom_color;                                                     "
  "                                                                         "
  // "const float thickness = 0.075;"
  "                                                                         "
  "void main() {                                                            "
  "  vec3 a = vec3(gl_in[0].gl_Position.xy, 0.0);"
  "  vec3 b = vec3(gl_in[1].gl_Position.xy, 0.0);"
  "  vec3 line = b - a;"
  "  vec3 perp = normalize(vec3(-line.y, line.x, 0.0));"
  "  vec3 v1 = a + edgeThickness*perp;"
  "  vec3 v2 = a - edgeThickness*perp;"
  "  vec3 v3 = b + edgeThickness*perp;"
  "  vec3 v4 = b - edgeThickness*perp;"
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
  "uniform float edgeOpacity;                                            "
  "in vec2 Vertex_UV;"
  "in vec3 geom_color;"
  "out vec4 frag_color;"
  "void main() {"
  "  float center = 0.5;"
  // "  float thickness = 0.45;"
  "  float thickness = 0.25;"
  "  float blur = 0.2;"
  "  float t = abs(Vertex_UV.y - 0.5);"
  "  vec4 black = vec4(0.0, 0.0, 0.0, edgeOpacity);"
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
           1 * sy * m_scale + m_yOffset, // top
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

  GLuint projectionMatrixID = glGetUniformLocation(m_edgeShaderProgram, "projectionMatrix");  
  GLuint edgeThicknessID = glGetUniformLocation(m_edgeShaderProgram, "edgeThickness");  
  GLuint edgeOpacityID = glGetUniformLocation(m_edgeShaderProgram, "edgeOpacity");  

  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, imageTextureID[0]);
   

  GLuint nodeRadiusID = glGetUniformLocation(m_activeNodeShader, "nodeRadius");  
  GLuint nodeOutlineID = glGetUniformLocation(m_activeNodeShader, "nodeOutline");  
  GLuint nodeSmoothnessID = glGetUniformLocation(m_activeNodeShader, "nodeSmoothness");  
  GLint texLoc = glGetUniformLocation(m_activeNodeShader, "imageTex");
  glUniform1i(texLoc, 0);

  glBindVertexArray(m_vertexArrayObject);  

  // render edges  
  glBindVertexArray(m_vertexArrayObject);  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_edgeElementVBO);
  glUseProgram(m_edgeShaderProgram);  

  GLfloat thickness[1] = { m_edgeThickness };
  glUniform1fv(edgeThicknessID, 1, thickness);

  GLfloat opacity[1] = { m_edgeOpacity };
  glUniform1fv(edgeOpacityID, 1, opacity);

  glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, &projectionMatrix[0]);
  glDrawElements(GL_LINES, edgeIndices.size(), GL_UNSIGNED_INT, 0);

  // render nodes
  glUseProgram(m_activeNodeShader);
  projectionMatrixID = glGetUniformLocation(m_activeNodeShader, "projectionMatrix");  
  GLfloat radius[1] = { m_nodeRadius };
  glUniform1fv(nodeRadiusID, 1, radius);

  GLfloat outline[1] = { m_nodeOutline };
  glUniform1fv(nodeOutlineID, 1, outline);

  GLfloat smoothness[1] = { m_nodeSmoothness };
  glUniform1fv(nodeSmoothnessID, 1, smoothness);

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
    case 'd':
      m_useDebugLayout = !m_useDebugLayout;
      setCrystal(m_currentLevel, m_currentCrystal);
      break; 
    case '/':
      m_nodeRadius = std::max(0.1, m_nodeRadius / 1.1);
      break;
    case '\'':
      m_nodeRadius *= 1.1;
      break;
    case '.':
      m_nodeOutline = std::max(0.001, m_nodeOutline / 1.1);
      break;
    case ';':
      m_nodeOutline *= 1.1;
      break;
    case ',':
      m_nodeSmoothness = std::max(0.01, m_nodeSmoothness / 1.1);
      break;
    case 'l':
      m_nodeSmoothness *= 1.1;
      break;
    case 'm':
      m_edgeThickness = std::max(0.005, m_edgeThickness / 1.1);
      std::cout << "m_edgeThickness = " << m_edgeThickness << std::endl;
      break;
    case 'k':
      m_edgeThickness *= 1.1;
      std::cout << "m_edgeThickness = " << m_edgeThickness << std::endl;
      break;
    case 'n':
      m_edgeOpacity = std::max(0.01, m_edgeOpacity / 1.1);
      std::cout << "m_edgeOpacity = " << m_edgeOpacity << std::endl;
      break;
    case 'j':
      m_edgeOpacity *= 1.1;
      std::cout << "m_edgeOpacity = " << m_edgeOpacity << std::endl;
      break;
    case 't':
      if (m_activeNodeShader == m_shaderProgram) {
        m_activeNodeShader = m_thumbnailShaderProgram;
      } else {
        m_activeNodeShader = m_shaderProgram;
      }
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
