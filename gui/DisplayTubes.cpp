#include "DisplayTubes.h"
#include "Precision.h"
#include <string>

template<typename TPrecision> DisplayTubes<TPrecision>::DisplayTubes(
    HDVizData *data, HDVizState *state, std::string fontname) : data(data), 
    state(state), font(fontname.c_str()){ 
  drawOverlay = true;     
  tubesOn = true;
  showPosition = true;
  showSamples = false;
  extremaOnly = false;

  oldN = data->getEdges().N();
  mod = 0;
  cur_button = -1;

  scale = 1;

  //naviagtion
  zoom = 1;
  tx = 0;
  ty = 0;
  rotation[0] = -90;
  rotation[1] = 0;
  rotation[2] = 0;
  rotationAxis = 0;

  initState();
  setFontSize(20);
};


template<typename TPrecision>
std::string DisplayTubes<TPrecision>::title(){
  return "Morse-Smale Complex";
};

template<typename TPrecision>
void DisplayTubes<TPrecision>::reshape(int w, int h){
  width = w;
  height = h;
  glViewport(0, 0, w, h);       
  glMatrixMode(GL_PROJECTION);  
  glLoadIdentity();
  setupOrtho(w, h);
};

template<typename TPrecision>
void DisplayTubes<TPrecision>::init(){  
  glEnable(GL_LIGHTING); 	
  GLfloat mat_spec[]={1, 1, 1, 1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 

  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);

  //Global ambient light
  GLfloat light_ambient[] = {0.3, 0.3, 0.3, 1};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);

  //Local Viewer
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

  GLfloat white_light[] = {1, 1, 1, 1};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
  glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
  //glLightf(GL_LIGHT0, GL_SPECULAR, 1);
  glEnable(GL_LIGHT0);



  glClearColor(1, 1, 1, 0);
  glClearStencil(0x0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);

  gleSetJoinStyle (TUBE_NORM_PATH_EDGE | TUBE_JN_ROUND); 
  gleSetNumSides(25);

  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glHint(GL_FOG_HINT, GL_NICEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glEnable(GL_RESCALE_NORMAL);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

};

template<typename TPrecision>
void DisplayTubes<TPrecision>::printHelp(){
  std::cout <<"Mouse controls:\n\n";
  std::cout <<"- Navigation around scene\n";
  std::cout <<"    Press and drag mouse in drawing area  use\n";
  std::cout <<"    o Left mouse button for translation.\n";
  std::cout <<"    o Middle mouse button for zoom.\n";
  std::cout <<"    o Right mouse button for rotation.\n";
  std::cout <<"- Ctrl left click to emphasize/demphasize colored tube and select peak\n\n\n";
  std::cout <<"- Ctrl right mosue button down to move heat plot location along curve\n\n\n";

  std::cout <<"Keyboard controls:\n\n";
  std::cout <<"  h - help\n";
  std::cout <<"  q - quit\n";
  std::cout <<"  + - increase scale (thickness of tubes etc)\n";
  std::cout <<"  - - decrease scale\n";
  std::cout <<"  p - 2 step PCA layout \n";
  std::cout <<"  w - PCA layout \n";
  std::cout <<"  i - 2 step Isomap layout \n";
  std::cout <<"  n - show only layout \n";
  std::cout <<"  e - toogle extrema only \n";
  std::cout <<"  t - toogle transparent tubes \n";
  std::cout <<"  x - rotate around x axis \n";
  std::cout <<"  y - rotate around y axis \n";
  std::cout <<"  z - rotate around z axis \n";
  std::cout <<"  o - toggle overlay on/off \n";
  std::cout <<"  l - show cell location ball \n";
  std::cout <<"  ] - next cell for input domain window  \n";
  std::cout <<"  [ - previous cell for input domain window \n";      
  std::cout <<"  > - increase persistance level  \n";
  std::cout <<"  < - decrease persistance level \n";
};

template<typename TPrecision>
void DisplayTubes<TPrecision>::display(void){
  if((int)data->getEdges().N() != oldN){
    oldN = data->getEdges().N();
    initState(); 
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW); 	
  glLoadIdentity();

  GLfloat view_light_pos[] = {5, -30, -15, 1};
  glLightfv(GL_LIGHT0, GL_POSITION, view_light_pos);

  glScalef(zoom, zoom, zoom);
  glTranslatef(tx,ty, 5); 
  glRotatef(rotation[0], 1, 0, 0); 
  glRotatef(rotation[1], 0, 1, 0); 
  glRotatef(rotation[2], 0, 0, 1); 
  glTranslatef(0,0, 0.5); 

  renderMS();

  // Draw coordinates.
  Precision sx = 1;    // TODO(jonbronson): Shouldn't this be TPrecision?
  Precision sy = 1;    // TODO(jonbronson): Shouldn't this be TPrecision? 
  if(width>height){
    sx = (Precision)width/height;
  }
  else{
    sy = (Precision)height/width;
  }

  glLoadIdentity(); 
  glTranslatef(-4*sx + 0.7, -4*sy + 0.7, 0);  
  glRotatef(rotation[0], 1, 0, 0);
  glRotatef(rotation[1], 0, 1, 0); 
  glRotatef(rotation[2], 0, 0, 1); 

  glDisable(GL_LIGHTING); 	
  glColor3f(0, 0, 0);
  glLineWidth(4.f);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, 0.5);
  glEnd();  

  setFontSize(width/50.f);
	glEnable(GL_BLEND);
  glRasterPos3f(0, 0, 0.7f);
  font.Render("f(x)");
  
  glColor3f(0.5, 0.5, 0.5);
  glLineWidth(2.f);
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0.5, 0);
  glVertex3f(0, 0, 0);
  glVertex3f(0.5, 0, 0);
  glEnd();   
  
  glRasterPos3f(0, 0.7f, 0);
  font.Render("p1");

  glRasterPos3f(0.7f, 0, 0);
  font.Render("p2");


  // Draw persistance graph.
  if (drawOverlay) {
    glDisable(GL_DEPTH_TEST);
    glLoadIdentity(); 
    glTranslatef(-1.5, -4*sy+0.5, 0);  

    Precision pmax = 1; 
    Precision emax = data->getPersistence().N()+1;
    glColor3f(1, 0, 0);
    int curL = getPersistanceLevel();
    Precision prev = 0;
    Precision next = data->getPersistence()(curL);
    if (curL != 0) {
      prev = data->getPersistence()(curL-1);
    }
    if (next>1) {
      next =1;
    } 

    Precision s = sx*5;
    Precision hs = 1.5;
    glBegin(GL_QUADS);
    glVertex2f(0 + prev/pmax*s, hs-curL/emax*hs);
    glVertex2f(0 + next/pmax*s, hs-curL/emax*hs);
    glVertex2f(0 + next/pmax*s, 0);
    glVertex2f(0 + prev/pmax*s, 0);
    glEnd();

    glColor3f(0.5, 0.5, 0.5);
    glLineWidth(2.f);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, hs, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(s, 0, 0);      
    glVertex3f(s/2.f, 0.05, 0);
    glVertex3f(s/2.f, -0.05, 0);
    glEnd();       

    glColor3f(1.f, 0, 0);
    glLineWidth(2.f);
    glBegin(GL_LINES);
    prev =0;
    for(unsigned int i=0; i<data->getPersistence().N()-1; i++){
      glVertex2f(0 + prev/pmax*s, hs - i/emax*hs);
      prev = data->getPersistence()(i);
      glVertex2f(0 + prev/pmax*s, hs - i/emax*hs);
    }   
    glVertex2f(0 + prev/pmax*s, 2/emax*hs);
    glVertex2f(s, 2/emax*hs);
    glEnd(); 


    // labels
    glColor3f(0, 0, 0);

    setFontSize(width/50.f);
    Precision l = font.Advance("0.0")/width*dw;
    glRasterPos2f(0-l/2.f, -0.15);
    font.Render("0.0");

    l = font.Advance("0.5")/width*dw;
    glRasterPos2f(s/2.f-l/2.f, -0.15);
    font.Render("0.5");

    l = font.Advance("1.0")/width*dw;
    glRasterPos2f(s-l/2.f, -0.15);
    font.Render("1.0");

   
    //glRasterPos2f(0-l, 2/emax*hs);
    //font.Render("2");

    glRasterPos2f(0-l, hs);
    std::stringstream ss123;
    ss123 << emax;
    font.Render( ss123.str().c_str() );

    // Write info
    glLoadIdentity(); 
    glTranslatef(-4*sx+0.2, 4*sy-0.4, 0); 
   
    glRasterPos2f(0, 0); 
    setFontSize(20);
    std::stringstream sse;
    sse << "Value: ";
    sse << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    sse << data->yc[state->selectedCell](state->selectedPoint) ;
    font.Render(sse.str().c_str());

    glRasterPos2f(0, -0.2f); 
    std::stringstream ssv;
    ssv << "Input std: ";
    ssv << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    ssv << (data->yw[state->selectedCell](state->selectedPoint)-0.03) * data->zmax/0.3;
    font.Render(ssv.str().c_str());
    
    glRasterPos2f(0, -0.4f);       
    std::stringstream sss;
    sss << "Density: ";
    sss << std::setiosflags(std::ios::fixed) << std::setprecision(4);
    sss << data->yd[state->selectedCell](state->selectedPoint);
    font.Render(sss.str().c_str());


    // Draw color map.
    glLoadIdentity(); 
    glTranslatef(4*sx-0.7, 4*sy-2.3, 0);  
     

    glBegin(GL_QUAD_STRIP);
    std::vector<Precision> color =
      data->colormap.getColor(data->efmin);
    glColor3f(color[0], color[1], color[2]);
    glVertex2f(0, 0);
    glVertex2f(0.4, 0);

    color = data->colormap.getColor((data->efmin+data->efmax)/2.f);
    glColor3f(color[0], color[1], color[2]);
    glVertex2f(0, 1);
    glVertex2f(0.4, 1);      
    
    color = data->colormap.getColor(data->efmax);
    glColor3f(color[0], color[1], color[2]);
    glVertex2f(0, 2);
    glVertex2f(0.4, 2);
    glEnd(); 

    glColor3f(0, 0, 0);
    setFontSize(20.f);
    
    
    std::stringstream ssmin;
    ssmin << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    ssmin << data->efmin ;
    l = font.Advance(ssmin.str().c_str())/width*dw;
    glRasterPos2f(0 - l - 0.03, 0.03);
    font.Render(ssmin.str().c_str());
    
    std::stringstream ssmax;
    ssmax << std::setiosflags(std::ios::fixed) << std::setprecision(2);
    ssmax << data->efmax ;
    l = font.Advance(ssmax.str().c_str())/width*dw;
    glRasterPos2f(0 - l - 0.03, 2.03);
    font.Render(ssmax.str().c_str());

    glBegin(GL_LINES);
    glVertex2f(-0.15, 2);
    glVertex2f(0.4, 2);
    glVertex2f(0, data->z[state->selectedCell](state->selectedPoint)*2);
    glVertex2f(0.4, data->z[state->selectedCell](state->selectedPoint)*2);
    glVertex2f(-0.15, 0);
    glVertex2f(0.4, 0);
    glEnd();
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING); 

  glutSwapBuffers();
};


template<typename TPrecision>
void DisplayTubes<TPrecision>::keyboard(unsigned char key, int x, int y) {
  switch(key) {
    // quit
    case 27:   
    case 'q':
    case 'Q':
      exit(0);
      break;
    case 'h':
    case 'H':
      printHelp();
      break;  
    case '+':
      scale += 0.1;  
      break;
    case '-':
      scale -= 0.1;  
      break;        
    case 'w':
    case 'W':
      data->setLayout(HDVizLayout::PCA, state->currentLevel);
      notifyChange();  
      break;
    case 'p':
    case 'P':
      data->setLayout(HDVizLayout::PCA2, state->currentLevel);
      notifyChange();  
      break;
    case 'i':
    case 'I':
      data->setLayout(HDVizLayout::ISOMAP, state->currentLevel);
      notifyChange();  
      break;
    case 'a':
    case 'A':
      for(unsigned int i=0; i<selectedTubes.N(); i++){
        selectedTubes(i) = false;
        renderMode(i) = RENDER_TUBE;
      }    
      for(unsigned int i=0; i<selectedPeaks.N(); i++){
        selectedPeaks(i) = true;
      }
      break;  
    case 'n':
    case 'N':
      for(unsigned int i=0; i<selectedTubes.N(); i++){
        selectedTubes(i) = false;      
        renderMode(i) = RENDER_NOTHING;
      }  
      for(unsigned int i=0; i<selectedPeaks.N(); i++){
        selectedPeaks(i) = false;
      }
      break;
    case 't':
    case 'T':
      for(unsigned int i=0; i<selectedCTubes.N(); i++){
        selectedCTubes(i) = tubesOn;
      }
      tubesOn = !tubesOn;
      break;  
    case 'e':
    case 'E':
      extremaOnly = !extremaOnly;    
      break;  
    case 'x':
    case 'X':
      rotationAxis = 0;
      break;
    case 'y':
    case 'Y':
      rotationAxis = 1;
      break;
    case 'z':
    case 'Z':
      rotationAxis = 2;
      break;
    case 'l':
    case 'L':
      showPosition = !showPosition;    
      break;
    case 'k':
    case 'K':
      showSamples = !showSamples;    
      break;        
    case 'o':
    case 'O':
      drawOverlay = !drawOverlay;    
      break;
    case ']':
      state->selectedCell++;
      if((unsigned int) state->selectedCell >= data->getEdges().N()){
        state->selectedCell = 0;
      }
      notifyChange();
      break;
    case '[':
      state->selectedCell--;
      if(state->selectedCell < 0){
        state->selectedCell = data->getEdges().N()-1;
      }
      notifyChange();
      break;        
    case '>':
      increasePersistanceLevel();
      notifyChange();
      break;
    case '<':
      decreasePersistanceLevel();
      notifyChange();
      break;
  }
  glutPostRedisplay();
};

template<typename TPrecision>
void DisplayTubes<TPrecision>::mouse(int button, int state, int x, int y) {
  last_x = x;
  last_y = y;
  mod = glutGetModifiers();
  if (state == GLUT_DOWN) {
    if (button == GLUT_LEFT_BUTTON && (mod == GLUT_ACTIVE_CTRL || mod  == GLUT_ACTIVE_SHIFT)) {
      cur_button = button;
      doPick();
    } else {
      cur_button = button;
    }
  } else if (button == cur_button) {
    cur_button = -1;
  }
};



// catch mouse move events
template<typename TPrecision>
void DisplayTubes<TPrecision>::motion(int x, int y) {
  int dx = x-last_x;
  int dy = y-last_y;

  switch (cur_button) {
    case GLUT_LEFT_BUTTON:
      if(mod == GLUT_ACTIVE_CTRL){
        zoom += dx*0.01;
        if(zoom < 0){
          zoom = 0;
        }
      }
      else{
        // translate
        tx -= dx*0.01;
        ty += dy*0.01;  
      }
      break;

    case GLUT_MIDDLE_BUTTON:
      zoom += dx*0.01;
      if(zoom < 0){
        zoom = 0;
      }
      break;

    case GLUT_RIGHT_BUTTON:
      // rotate
      if (mod == GLUT_ACTIVE_CTRL) {
        state->selectedPoint += (int) dx;
        if (state->selectedPoint < 0 ) {
          state->selectedPoint = 0;
        }
        if (state->selectedPoint >= data->getNumberOfSamples() ) {
          state->selectedPoint = data->getNumberOfSamples() - 1;
        }
        notifyChange();
      } else {
        Precision rot = rotation[rotationAxis] ;
        rot += dx;
        int r = (int)(rot/360);
        rotation[rotationAxis]= rot - r*360;
      }
      break;
  }
  //}
  last_x = x;
  last_y = y;
  glutPostRedisplay();
}


/**
 *
 */
template<typename TPrecision>
void DisplayTubes<TPrecision>::addWindow(int w) {
  windows.push_back(w);
};

/**
 *
 */
template<typename TPrecision>
void DisplayTubes<TPrecision>::notifyChange() {
  for (unsigned int i = 0; i < windows.size(); i++) {
    glutPostWindowRedisplay(windows[i]);
  }
};



/**
 *
 */
template<typename TPrecision>
void DisplayTubes<TPrecision>::increasePersistanceLevel() {
  setPersistenceLevel(state->currentLevel+1);
}
    
/**
 *
 */
template<typename TPrecision>
void DisplayTubes<TPrecision>::decreasePersistanceLevel() {
  setPersistenceLevel(state->currentLevel-1);
};

/**
 *
 */
template<typename TPrecision>
int DisplayTubes<TPrecision>::getPersistanceLevel() {
  return state->currentLevel;
};

/**
 *
 */
template<typename TPrecision>
void DisplayTubes<TPrecision>::setPersistenceLevel(int pl, bool update) {
  state->currentLevel = pl;
  if (state->currentLevel > data->getMaxPersistenceLevel()) {
    state->currentLevel = data->getMaxPersistenceLevel(); 
  } else if(state->currentLevel < data->getMinPersistenceLevel()) {
    state->currentLevel = data->getMinPersistenceLevel();
  }

  data->loadData(state->currentLevel);

  if (state->selectedCell >= (int) data->getEdges().N()) {
    state->selectedCell = data->getEdges().N() - 1;
  }
  if (update) {
    notifyChange();
  }
}


/**
 *
 */
template<typename TPrecision>
void DisplayTubes<TPrecision>::setRenderModesFromPeaks() {
  for (unsigned int i = 0; i < selectedPeaks.N(); i++) {
    selectedPeaks(i) = false;
  }

  for (unsigned int i = 0; i < data->getEdges().N(); i++) {
    if (selectedTubes(i)) {
      selectedPeaks(data->getEdges()(0, i)) = true;
      selectedPeaks(data->getEdges()(1, i)) = true;
    }
  }

  for (unsigned int i=0; i<data->getEdges().N(); i++) {
    int e1 =data->getEdges()(0, i);
    int e2 =data->getEdges()(1, i);
    if (selectedPeaks(e1) && selectedPeaks(e2)) {
      renderMode(i) =  RENDER_TUBE;
    } else if(selectedPeaks(e1)) {
      if (data->getExtremaValues()(e1) > data->getExtremaValues()(e2)) {
        renderMode(i) =  RENDER_FADE_MAX_TO_MIN;
      } else {
        renderMode(i) =  RENDER_FADE_MIN_TO_MAX;
      }
    } else if (selectedPeaks(e2)) {
      if (data->getExtremaValues()(e2) > data->getExtremaValues()(e1)) {
        renderMode(i) =  RENDER_FADE_MAX_TO_MIN;
      } else {
        renderMode(i) =  RENDER_FADE_MIN_TO_MAX;
      }
    } else {
      renderMode(i) = RENDER_NOTHING;
    }
  }
}

template<typename TPrecision>
void DisplayTubes<TPrecision>::doPick() {
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  GLuint selectBuf[BUFSIZE];
  glSelectBuffer(BUFSIZE, selectBuf);
  glRenderMode(GL_SELECT);
  glInitNames();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(last_x, vp[3]-last_y, 5, 5, vp);
  setupOrtho(width, height); 

  display();
  GLint hits = glRenderMode(GL_RENDER);
  if (hits > 0){
    int index = selectBuf[3];
    if (mod == GLUT_ACTIVE_CTRL) {
      if (index >=0 && index < (int)data->getEdges().N()) {
        selectedCTubes(index) = !selectedCTubes(index);
        std::cout << "Edge: " << index << std::endl;
      }
    }    
    else if (mod == GLUT_ACTIVE_SHIFT) {
      if (index >=0 && index < (int) data->getEdges().N()) {
        selectedTubes(index) = !selectedTubes(index);
        if (selectedTubes(index)) {
          state->selectedCell = index;
        }
        setRenderModesFromPeaks();
        notifyChange();
      }
    }
  }  

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glutPostRedisplay();
};


template<typename TPrecision>
void DisplayTubes<TPrecision>::setupOrtho(int w, int h) {
  Precision sx=1;
  Precision sy=1;
  if (w > h) {
    sx = (Precision)w/h;
  } else {
    sy = (Precision)h/w;
  }
  glOrtho(-4*sx, 4*sx, -4*sy, 4*sy, 16, -16);
  dh = 8*sy;
  dw = 8*sx;
}


template<typename TPrecision>
void DisplayTubes<TPrecision>::renderTubes(bool selectedOnly) {
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);
  gleDouble points[data->getNumberOfSamples()+2][3];
  float colors[data->getNumberOfSamples()+2][4];
  gleDouble radii[data->getNumberOfSamples()+2];

  for (unsigned int i = 0; i < data->getEdges().N(); i++) {

    /*if(renderMode(i) == RENDER_NOTHING) continue;
      if( renderMode(i) == RENDER_FADE_MIN_TO_MAX || 
      renderMode(i) == RENDER_FADE_MAX_TO_MIN    ){*/
    if (renderMode(i) != RENDER_TUBE) continue;

    for (unsigned int k = 0; k < data->L[i].N(); k++) {
      std::vector<Precision> color = data->colormap.getColor(data->yc[i](k));
      colors[k+1][0] = color[0];
      colors[k+1][1] = color[1];
      colors[k+1][2] = color[2];
      colors[k+1][3] = 1;

      for (unsigned int m = 0; m < data->L[i].M(); m++) {
        points[k+1][m] = data->L[i](m, k);
      }
      points[k+1][2] = data->z[i](k);
      radii[k+1] = 0.02;    
    }     
    radii[0] = radii[1];
    radii[data->getNumberOfSamples()+1] = radii[data->getNumberOfSamples()];
    for (unsigned int m = 0; m < 3; m++) {
      points[0][m] = points[1][m]+ points[2][m] - points[1][m];
      points[data->getNumberOfSamples() + 1][m] = 
          points[data->getNumberOfSamples()][m] + 
          points[data->getNumberOfSamples()][m] - 
          points[data->getNumberOfSamples() - 1][m];
    }
    for (unsigned int m = 0; m < 4; m++) {
      colors[0][m] = colors[1][m];
      colors[data->getNumberOfSamples() + 1][m] = 
          colors[data->getNumberOfSamples()][m];
    }
    glPushName(i);
    glePolyCone_c4f(data->getNumberOfSamples() + 2, points, colors, radii);
    glPopName();     
  } 




  //Draw extremal points
  for (unsigned int i = 0; i < data->getExtremaLayout().N(); i++) {
    if (selectedPeaks(i)) {
      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i), 
          data->getExtremaLayout()(1, i), 
          data->getExtremaNormalized()(i)); 
      std::vector<Precision> color = data->colormap.getColor(data->getExtremaValues()(i));
      glColor4f(color[0], color[1], color[2], 0.3);
      glutSolidSphere(0.028, 100, 100);
      glPopMatrix();
    }
  }  

  if (selectedOnly) return;

  glEnable(GL_BLEND);
  for (unsigned int i = 0; i < data->getEdges().N(); i++) {
    if (renderMode(i) == RENDER_TUBE || renderMode(i) == RENDER_NOTHING) continue;

    for (unsigned int k = 0; k < data->L[i].N(); k++) {
      std::vector<Precision> color = data->colormap.getColor(data->yc[i](k));
      colors[k+1][0] = color[0];
      colors[k+1][1] = color[1];
      colors[k+1][2] = color[2];
      if (renderMode(i) == RENDER_FADE_MIN_TO_MAX) {
        colors[k+1][3] = 1 - k/ (data->L[i].N()-1.f);
      }
      else if (renderMode(i) == RENDER_FADE_MAX_TO_MIN) {
        colors[k+1][3] = k/ (data->L[i].N()-1.f);
      }

      for (unsigned int m = 0; m < data->L[i].M(); m++) {
        points[k+1][m] = data->L[i](m, k);
      }
      points[k+1][2] = data->z[i](k);
      radii[k+1] = 0.02;    
    }     
    radii[0] = radii[1];
    radii[data->getNumberOfSamples() + 1] = radii[data->getNumberOfSamples()];
    for (unsigned int m=0; m<3; m++) {
      points[0][m] = points[1][m]+ points[2][m] - points[1][m];
      points[data->getNumberOfSamples()+1][m] = 
          points[data->getNumberOfSamples()][m] + 
          points[data->getNumberOfSamples()][m] - 
          points[data->getNumberOfSamples() - 1][m];
    }
    for (unsigned int m = 0; m < 4; m++) {
      colors[0][m] = colors[1][m];
      colors[data->getNumberOfSamples()+1][m] = colors[data->getNumberOfSamples()][m];
    }

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glPushName(i);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2, 0.1);
    glePolyCone_c4f(data->getNumberOfSamples() + 2, points, nullptr, radii);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDepthMask(GL_FALSE);
    glePolyCone_c4f(data->getNumberOfSamples() + 2, points, colors, radii);
    glPopName();     
    glDepthMask(GL_TRUE);
  }  

  glColor4f(0.9, 0.9, 0.9, 0.1f);
  for (unsigned int i = 0; i < data->getEdges().N(); i++) {
    if (renderMode(i) != RENDER_NOTHING) continue;

    for (unsigned int k = 0; k < data->L[i].N(); k++) {
      for (unsigned int m = 0; m < data->L[i].M(); m++) {
        points[k+1][m] = data->L[i](m, k);
      }
      points[k+1][2] = data->z[i](k);
      radii[k+1] = 0.02;    
    }     
    radii[0] = radii[1];
    radii[data->getNumberOfSamples() + 1] = radii[data->getNumberOfSamples()];
    for (unsigned int m = 0; m < 3; m++) {
      points[0][m] = points[1][m]+ points[2][m] - points[1][m];
      points[data->getNumberOfSamples() + 1][m] = 
          points[data->getNumberOfSamples()][m] + 
          points[data->getNumberOfSamples()][m] - 
          points[data->getNumberOfSamples() - 1][m];
    }


    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glPushName(i);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2, 0.1);
    glePolyCone_c4f(data->getNumberOfSamples() + 2, points, nullptr, radii);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDepthMask(GL_FALSE);
    glePolyCone_c4f(data->getNumberOfSamples() + 2, points, nullptr, radii);
    glPopName();     
    glDepthMask(GL_TRUE);
  }   

  //Draw extremal points
  for (unsigned int i = 0; i < data->getExtremaLayout().N(); i++) {
    if (!selectedPeaks(i)) { 
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glPushName(i);

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2, 0.1);

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i), 
          data->getExtremaLayout()(1, i), 
          data->getExtremaNormalized()(i)); 
      glutSolidSphere(0.028, 100, 100);
      glPopMatrix();

      glDisable(GL_POLYGON_OFFSET_FILL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      glDepthMask(GL_FALSE);
      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i), 
          data->getExtremaLayout()(1, i), 
          data->getExtremaNormalized()(i)); 
      glutSolidSphere(0.028, 100, 100);
      glPopMatrix();
      glDepthMask(GL_TRUE);
    }
  }  

  glDisable(GL_BLEND);
}

template<typename TPrecision>
void DisplayTubes<TPrecision>::renderWidths() {
  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
  glStencilFunc(GL_ALWAYS, 1, 1);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60);

  gleDouble points[data->getNumberOfSamples() + 2][3];
  gleDouble radii[data->getNumberOfSamples() + 2];
  float colors[data->getNumberOfSamples() + 2][4];

  for (unsigned int i=0; i<data->getEdges().N(); i++) {      
    if (renderMode(i) != RENDER_TUBE) continue;

    if (selectedCTubes(i)) {

      int i1 = data->getEdges()(0, i);
      int i2 = data->getEdges()(1, i);


      for (unsigned int k = 0; k < data->L[i].N(); k++) {
        for (unsigned int m = 0; m < data->L[i].M(); m++) {
          points[k+1][m] = data->L[i](m, k);
        }
        points[k+1][2] = data->z[i](k);
        radii[k+1] = scale*data->yw[i](k); 
      }    
      radii[0] = radii[1];
      radii[data->getNumberOfSamples() + 1] = radii[data->getNumberOfSamples()];
      for (unsigned int m=0; m<3; m++) {
        points[0][m] = points[1][m]+ points[2][m] - points[1][m];
        points[data->getNumberOfSamples() + 1][m] = 
            points[data->getNumberOfSamples()][m] + 
            points[data->getNumberOfSamples()][m] - 
            points[data->getNumberOfSamples() - 1][m];
      }

      glColor4f(0.8, 0.8, 0.8, 0.1);


      //Draw outer tube depths only

      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2, 0.1);

      glePolyCone_c4f(data->getNumberOfSamples() + 2, points, nullptr, radii);

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i1), 
          data->getExtremaLayout()(1, i1), 
          data->getExtremaNormalized()(i1)); 
      glutSolidSphere(scale * data->getExtremaWidths()(i1), 50, 50);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i2), 
          data->getExtremaLayout()(1, i2), 
          data->getExtremaNormalized()(i2)); 
      glutSolidSphere(scale * data->getExtremaWidths()(i2), 50, 50);
      glPopMatrix();

      glDisable(GL_POLYGON_OFFSET_FILL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);



      //draw transparent outer tube but only front faces 
      glEnable(GL_BLEND);
      glDepthMask(GL_FALSE);
      glDepthFunc(GL_LEQUAL);

      glePolyCone_c4f(data->getNumberOfSamples() + 2, points, nullptr, radii);

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i1), 
          data->getExtremaLayout()(1, i1), 
          data->getExtremaNormalized()(i1)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i1), 50, 50);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i2), 
          data->getExtremaLayout()(1, i2), 
          data->getExtremaNormalized()(i2)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i2), 50, 50);
      glPopMatrix();

      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);
    }
  }


  glClear(GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT);
  glDisable(GL_STENCIL_TEST);  
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);  
  renderTubes(true);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    
  glEnable(GL_STENCIL_TEST);  

  glDisable(GL_LIGHTING);

  for (unsigned int i = 0; i < data->getEdges().N(); i++) {
    if ( renderMode(i) != RENDER_TUBE) continue;
    if (selectedCTubes(i)) {
      int i1 = data->getEdges()(0, i);
      int i2 = data->getEdges()(1, i);      


      for (unsigned int k = 0; k < data->L[i].N(); k++) {
        std::vector<Precision> color = data->dcolormap.getColor(data->yd[i](k));
        colors[k+1][0] = color[0];
        colors[k+1][1] = color[1];
        colors[k+1][2] = color[2];
        colors[k+1][3] = 1;
      }    
      for (unsigned int m = 0; m < 4; m++){
        colors[0][m] = colors[1][m];
        colors[data->getNumberOfSamples() + 1][m] = colors[data->getNumberOfSamples()][m];
      }

      for (unsigned int k = 0; k < data->L[i].N(); k++) {
        for (unsigned int m = 0; m < data->L[i].M(); m++) {
          points[k+1][m] = data->L[i](m, k);
        }
        points[k+1][2] = data->z[i](k);
        radii[k+1] = scale*data->yw[i](k); 
      }    
      radii[0] = radii[1];
      radii[data->getNumberOfSamples() + 1] = radii[data->getNumberOfSamples()];
      for (unsigned int m = 0; m < 3; m++) {
        points[0][m] = points[1][m]+ points[2][m] - points[1][m];
        points[data->getNumberOfSamples() + 1][m] = 
            points[data->getNumberOfSamples()][m] + 
            points[data->getNumberOfSamples()][m] - 
            points[data->getNumberOfSamples() - 1][m];
      }




      glColor4f(1, 1, 1, 0);

      glStencilFunc(GL_ALWAYS, 0x1, 0x1);
      glEnable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);

      glePolyCone_c4f(data->getNumberOfSamples() + 2, points, nullptr, radii);

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i1), 
          data->getExtremaLayout()(1, i1), 
          data->getExtremaNormalized()(i1)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i1), 50, 50);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i2), 
          data->getExtremaLayout()(1, i2), 
          data->getExtremaNormalized()(i2)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i2), 50, 50);
      glPopMatrix();

      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);


      for (int k = 0; k < data->getNumberOfSamples() + 2; k++) {
        radii[k] += scale*0.025/zoom;
      }

      glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);

      //draw outlines

      glePolyCone_c4f(data->getNumberOfSamples() + 2, points, colors, radii);      

      if (data->getExtremaValues()(i1) < data->getExtremaValues()(i2)) {
        glColor4f(colors[0][0], colors[0][1], colors[0][2], colors[0][3]);
      } else {
        glColor4f(colors[data->getNumberOfSamples()][0], 
            colors[data->getNumberOfSamples()][1], 
            colors[data->getNumberOfSamples()][2],
            colors[data->getNumberOfSamples()][3]);
      }
      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i1), 
          data->getExtremaLayout()(1, i1), 
          data->getExtremaNormalized()(i1)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i1) + scale*0.025/zoom, 50, 50);
      glPopMatrix();

      if (data->getExtremaValues()(i2) < data->getExtremaValues()(i1)) {
        glColor4f(colors[0][0], colors[0][1], colors[0][2], colors[0][3]);
      } else{
        glColor4f(colors[data->getNumberOfSamples()][0], 
            colors[data->getNumberOfSamples()][1], 
            colors[data->getNumberOfSamples()][2],
            colors[data->getNumberOfSamples()][3]);
      }

      glePolyCone_c4f(data->getNumberOfSamples() + 2, points, nullptr, radii);

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i1), 
          data->getExtremaLayout()(1, i1), 
          data->getExtremaNormalized()(i1)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i1)+ scale*0.025/zoom, 50, 50);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i2), 
          data->getExtremaLayout()(1, i2), 
          data->getExtremaNormalized()(i2)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i2)+ scale*0.025/zoom, 50, 50);
      glPopMatrix();

      glClear(GL_STENCIL_BUFFER_BIT);
    } 
  }       

  glEnable(GL_LIGHTING);	
  glDisable(GL_STENCIL_TEST);  
}

template<typename TPrecision>
void DisplayTubes<TPrecision>::renderExtrema() {
  glEnable(GL_STENCIL_TEST);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60);
  for (unsigned int i = 0; i < data->getExtremaLayout().N(); i++) {
    if (selectedPeaks(i) == true) {
      glColor4f(0.8, 0.8, 0.8, 0.1);

      glStencilFunc(GL_ALWAYS, 0x1, 0x1);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
      glDepthMask(GL_FALSE);

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2, 0.1);

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i), 
          data->getExtremaLayout()(1, i), 
          data->getExtremaNormalized()(i)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i), 50, 50);
      glPopMatrix();

      glDisable(GL_POLYGON_OFFSET_FILL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

      glEnable(GL_BLEND);
      glDepthFunc(GL_LEQUAL);

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i), 
          data->getExtremaLayout()(1, i), 
          data->getExtremaNormalized()(i)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i), 50, 50);
      glPopMatrix();

      glDisable(GL_BLEND);
      glDepthFunc(GL_LESS);


      glDepthMask(GL_TRUE);
      glDisable(GL_LIGHTING);
      glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);

      glPushMatrix();
      glTranslatef(
          data->getExtremaLayout()(0, i), 
          data->getExtremaLayout()(1, i), 
          data->getExtremaNormalized()(i)); 
      glutSolidSphere(scale *data->getExtremaWidths()(i) + scale*0.015/zoom, 50, 50);
      glPopMatrix();

      glEnable(GL_LIGHTING);

      glClear(GL_STENCIL_BUFFER_BIT);  
    }
  } 
  glDisable(GL_STENCIL_TEST);      

};


template<typename TPrecision>
void DisplayTubes<TPrecision>::renderMS() {
  //draw color tubes and extremal points
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 2);

  renderTubes();

  //transparent widths
  if (extremaOnly) {
    renderExtrema(); 
  } else {
    renderWidths();
  }


  //Draw selected location
  if (showPosition) {
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);
    std::vector<Precision> color =
      data->colormap.getColor(data->yc[state->selectedCell](state->selectedPoint));
    glColor4f(color[0], color[1], color[2], 0.4); 
    //glColor4f(0, 0, 0, 1);   
    glPushMatrix(); 
    glTranslatef(data->L[state->selectedCell](0, state->selectedPoint), data->L[state->selectedCell](1,
          state->selectedPoint), data->z[state->selectedCell](state->selectedPoint));
    glutSolidSphere(0.04, 50, 50);  
    glPopMatrix();

    gleDouble points[4][3];
    gleDouble radii[4];
    int sIndex = state->selectedPoint-1;
    if (sIndex<0) {
      sIndex = 1;
    }
    Precision l = 0;
    for (int m = 0; m < 2; m++) {
      Precision p =  data->L[state->selectedCell](m, state->selectedPoint);
      Precision dir =  p - data->L[state->selectedCell](m, sIndex);
      l += dir*dir;  
    }
    Precision tmp = data->z[state->selectedCell](state->selectedPoint)- data->z[state->selectedCell](sIndex);
    l += tmp*tmp;
    l = sqrt(l);
    for (int m = 0; m < 2; m++) {
      Precision p =  data->L[state->selectedCell](m, state->selectedPoint);
      Precision dir =  p - data->L[state->selectedCell](m, sIndex); 
      points[0][m] = p + dir/l*0.04f; 
      points[1][m] = p + dir/l*0.02f; 
      points[2][m] = p - dir/l*0.02f; 
      points[3][m] = p - dir/l*0.04f; 
    }      
    Precision p =  data->z[state->selectedCell](state->selectedPoint);
    Precision dir =  p - data->z[state->selectedCell](sIndex); 
    points[0][2] = p + dir/l*0.04f; 
    points[1][2] = p + dir/l*0.02f; 
    points[2][2] = p - dir/l*0.02f; 
    points[3][2] = p - dir/l*0.04f; 
    for (int k = 0; k < 4; k++) {
      radii[k] = data->yw[state->selectedCell](state->selectedPoint);
    }
    glePolyCone_c4f(4, points, nullptr, radii);      
  }
};


template<typename TPrecision>
void DisplayTubes<TPrecision>::setFontSize(int fsize) {
  if (fsize < 5) {
    fsize = 5;
  }
  font.FaceSize(fsize);
};


template<typename TPrecision>
void DisplayTubes<TPrecision>::initState() {
  selectedCTubes.deallocate();
  selectedTubes.deallocate();
  renderMode.deallocate();
  selectedPeaks.deallocate();

  selectedCTubes = FortranLinalg::DenseVector<bool>(data->getEdges().N());
  selectedTubes = FortranLinalg::DenseVector<bool>(data->getEdges().N());
  renderMode = FortranLinalg::DenseVector<int>(data->getEdges().N());
  for (unsigned int i = 0; i < data->getEdges().N(); i++) {
    selectedCTubes(i) = true;
    selectedTubes(i) = false;
    renderMode(i) = RENDER_TUBE;
  }
  selectedPeaks = FortranLinalg::DenseVector<bool>(data->getExtremaLayout().N());
  for (unsigned int i = 0; i < data->getExtremaLayout().N(); i++) {
    selectedPeaks(i) = true;
  }
};

// instantiate necessary types
template class DisplayTubes<Precision>;
