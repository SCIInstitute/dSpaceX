#include "Precision.h"

#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "Linalg.h"
#include "colormapper.h" 

#include "CmdLine.h"

#include <GL/gl.h>	
#include "GL/glut.h"
#include <GL/gle.h>

#include <iostream>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFSIZE 512

//Windows
int ms;

int width, height;

Precision scale = 1;

//number of smaples per cell for rendering
int nSamples = 100;

//Morse-Smale edge information
DenseMatrix<unsigned int> edges; 

//curves
DenseMatrix<Precision> *curves;

//Samples
DenseMatrix<Precision> *samples;
DenseMatrix<Precision> *pSamples;
DenseMatrix<Precision> *fSamples;


DenseVector<unsigned int> E;
DenseVector<Precision> Ef;

//ColorMapper for each cell
ColorMapper<Precision> colormap;

//color/width and transparent width values
DenseVector<Precision> *yc;
DenseVector<Precision> *yw;



DenseMatrix<Precision> Xall;


//
bool *selection;

//mouse 
int last_x1;
int last_y1;
int cur_button1 = -1;

// the camera info
float eye1[3];
float lookat1[3];

float eye2[3];
float lookat2[3];


//_______ Load data

void loadColorValues(std::string type){
  Precision zmax = std::numeric_limits<Precision>::min();
  Precision zmin = std::numeric_limits<Precision>::max();
  for(unsigned int i=0; i<edges.N(); i++){
    std::stringstream ss1;
    ss1 << "ps_cell_" << i << type;
    yc[i].deallocate();
    yc[i] = LinalgIO<Precision>::readVector(ss1.str());
  
    for(unsigned int k=0; k< yc[i].N(); k++){
      if(yc[i](k) < zmin){
        zmin = yc[i]( k);
      }
      if(yc[i](k) > zmax){
        zmax = yc[i](k);
      }
    }
  }

  colormap = ColorMapper<Precision>(zmin, zmax);
  
}


void loadWidthValues(std::string type){
  Precision zmax = std::numeric_limits<Precision>::min();
  Precision zmin = std::numeric_limits<Precision>::max();

  for(unsigned int i=0; i<edges.N(); i++){
    std::stringstream ss1;
    ss1 << "ps_cell_" << i << type;
    yw[i].deallocate();
    yw[i] = LinalgIO<Precision>::readVector(ss1.str());    
   
    for(unsigned int k=0; k< yw[i].N(); k++){
      if(yw[i](k) < zmin){
        zmin = yw[i]( k);
      }
      if(yw[i](k) > zmax){
        zmax = yw[i](k);
      }
    }
  }
  
  for(unsigned int i=0; i<edges.N(); i++){
    Linalg<Precision>::Subtract(yw[i], zmin, yw[i]);
    Linalg<Precision>::Scale(yw[i], 0.2/(zmax-zmin), yw[i]);
    Linalg<Precision>::Add(yw[i], 0.01, yw[i]);
  }
  
}




//_________ Helper Linalg

void normalize(float v[3]){
	float l = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	l = 1 / (float)sqrt(l);

	v[0] *= l;
	v[1] *= l;
	v[2] *= l;
}

void crossproduct(float a[3], float b[3], float res[3]){
	res[0] = (a[1] * b[2] - a[2] * b[1]);
	res[1] = (a[2] * b[0] - a[0] * b[2]);
	res[2] = (a[0] * b[1] - a[1] * b[0]);
}

float length(float v[3]){
	return (float)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}








//________ Rendering

GLUquadricObj *qobj = gluNewQuadric();
void renderMS(){
 

  //draw color tubes and extremal points
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 2);
  
  gleDouble points[nSamples][3];
  float colors[nSamples][4];
  gleDouble radii[nSamples];

  ColorMapper<Precision> cmap(0, edges.N()-1);    
  Precision coords[3];
  coords[2] = 0;

  //draw samples
  for(unsigned int i=0; i<edges.N(); i++){

    for(unsigned int k=0; k< samples[i].N(); k++){     
      std::vector<Precision> color = colormap.getColor(fSamples[i](0,k));
      glColor4f(color[0], color[1], color[2], 1);        
      glPushMatrix();
      for(unsigned int m=0; m< samples[i].M(); m++){
        coords[m] = samples[i](m, k); 
      }
      glTranslatef(coords[0], coords[1], coords[2]);
      gluQuadricNormals(qobj, GLU_SMOOTH);
      gluSphere(qobj, scale*0.05, 30, 30);
      glPopMatrix();
    }
  }  
  
  for(unsigned int i=0; i<E.N(); i++){
    //Draw extremal point 
    glPushMatrix();
    std::vector<Precision> color = colormap.getColor(Ef(i));
    glColor4f(color[0], color[1], color[2], 1);
    for(unsigned int m=0; m< Xall.M(); m++){
      coords[m] = Xall(m, E(i)); 
    }
    glTranslatef(coords[0], coords[1], coords[2]);
    gluQuadricNormals(qobj, GLU_SMOOTH);
    gluSphere(qobj, scale * 0.2, 30, 30);
    glPopMatrix();
  }


  //draw tubes 
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);

  //draw projection lines  
  for(unsigned int i=0; i<edges.N(); i++){
    glColor4f(0, 0, 0, 0.15);     
    glBegin(GL_LINES);
    for(unsigned int k=0; k< samples[i].N(); k++){    
      for(unsigned int m=0; m< samples[i].M(); m++){
        coords[m] = pSamples[i](m, k); 
      }
      glVertex3f(coords[0], coords[1], coords[2]);     
      for(unsigned int m=0; m< samples[i].M(); m++){
        coords[m] = samples[i](m, k); 
      }
      glVertex3f(coords[0], coords[1], coords[2]); 
    }
    glEnd();
  }


  for(unsigned int i=0; i<edges.N(); i++){

    for(unsigned int k=0; k<curves[i].N(); k++){
      std::vector<Precision> color = colormap.getColor(yc[i](k));
      colors[k][0] = color[0];
      colors[k][1] = color[1];
      colors[k][2] = color[2];
      colors[k][3] = 0.5;
      
      points[k][2] = 0;
      for(unsigned int m=0; m<curves[i].M(); m++){
        points[k][m] = curves[i](m, k);
      }
      radii[k] = scale*yw[i](k);
    }
    glPushName(i);
    glePolyCone_c4f(nSamples, points, colors, radii);
    glPopName();    
  }
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);

}



void display1(void){

  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW); 	

	glLoadIdentity();
	GLfloat view_light_pos[] = {100, -100, -50, 1};
	glLightfv(GL_LIGHT0, GL_POSITION, view_light_pos);
	gluLookAt(eye1[0], eye1[1], eye1[2], lookat1[0], lookat1[1], lookat1[2], 0, 1, 0);
 
  renderMS();
  
  glutSwapBuffers();
}





//______ Picking

void doPick(){

  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);
  GLuint selectBuf[BUFSIZE];
  glSelectBuffer(BUFSIZE, selectBuf);
  glRenderMode(GL_SELECT);
  glInitNames();

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(last_x1, vp[3]-last_y1, 2, 2, vp);
  gluPerspective(80, width/(float)height, 1, 100);
  

  display1();
  GLint hits = glRenderMode(GL_RENDER);
  if(hits>0){
    int index = selectBuf[3];
    selection[index] = !selection[index];
  }  
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
	glutPostRedisplay();
};







//_______ Mouse

void mouse1(int button, int state, int x, int y){
	last_x1 = x;
	last_y1 = y;
  if (state == GLUT_DOWN){
    int mod = glutGetModifiers();
    if(mod == GLUT_ACTIVE_CTRL){
      doPick();
    }
    else if(mod ==  GLUT_ACTIVE_ALT){
    
    }
    else{
		  cur_button1 = button;
    }
  }
  else if (button == cur_button1){
		cur_button1 = -1;
	}

}




// catch mouse move events
void motion1(int x, int y){
	int dx = x-last_x1;
	int dy = y-last_y1;
		

		float scale, len, theta;
		float neye1[3], neye12[3];
		float f[3], r[3], u[3];

		switch(cur_button1){
		case GLUT_LEFT_BUTTON:
			// translate
			f[0] = lookat1[0] - eye1[0];
			f[1] = lookat1[1] - eye1[1];
			f[2] = lookat1[2] - eye1[2];
			u[0] = 0;
			u[1] = 1;
			u[2] = 0;

			// scale the change by how far away we are
			scale = sqrt(length(f)) * 0.007;

			crossproduct(f, u, r);
			crossproduct(r, f, u);
			normalize(r);
			normalize(u);

			eye1[0] += -r[0]*dx*scale + u[0]*dy*scale;
			eye1[1] += -r[1]*dx*scale + u[1]*dy*scale;
			eye1[2] += -r[2]*dx*scale + u[2]*dy*scale;
			
      lookat1[0] += -r[0]*dx*scale + u[0]*dy*scale;
			lookat1[1] += -r[1]*dx*scale + u[1]*dy*scale;
			lookat1[2] += -r[2]*dx*scale + u[2]*dy*scale;

			break;

		case GLUT_MIDDLE_BUTTON:
			// zoom
			f[0] = lookat1[0] - eye1[0];
			f[1] = lookat1[1] - eye1[1];
			f[2] = lookat1[2] - eye1[2];

			len = length(f);
			normalize(f);

			// scale the change by how far away we are
			len -= sqrt(len)*dx*0.03;

			eye1[0] = lookat1[0] - len*f[0];
			eye1[1] = lookat1[1] - len*f[1];
			eye1[2] = lookat1[2] - len*f[2];

			// make sure the eye1 and lookat1 points are sufficiently far away
			// push the lookat1std::vector<Precision> color = colormap.getColor(yc[i](k)); point forward if it is too close
			if (len < 1){
				lookat1[0] = eye1[0] + f[0];
				lookat1[1] = eye1[1] + f[1];
				lookat1[2] = eye1[2] + f[2];
			}

			break;

		case GLUT_RIGHT_BUTTON:
			// rotate

			neye1[0] = eye1[0] - lookat1[0];
			neye1[1] = eye1[1] - lookat1[1];
			neye1[2] = eye1[2] - lookat1[2];

			// first rotate in the x/z plane
			theta = -dx * 0.007;
			neye12[0] = (float)cos(theta)*neye1[0] + (float)sin(theta)*neye1[2];
			neye12[1] = neye1[1];
			neye12[2] =-(float)sin(theta)*neye1[0] + (float)cos(theta)*neye1[2];


			// now rotate vertically
			theta = -dy * 0.007;

			f[0] = -neye12[0];
			f[1] = -neye12[1];
			f[2] = -neye12[2];
			u[0] = 0;
			u[1] = 1;
			u[2] = 0;
			crossproduct(f, u, r);
			crossproduct(r, f, u);
			len = length(f);
			normalize(f);
			normalize(u);

			neye1[0] = len * ((float)cos(theta)*f[0] + (float)sin(theta)*u[0]);
			neye1[1] = len * ((float)cos(theta)*f[1] + (float)sin(theta)*u[1]);
			neye1[2] = len * ((float)cos(theta)*f[2] + (float)sin(theta)*u[2]);

			eye1[0] = lookat1[0] - neye1[0];
			eye1[1] = lookat1[1] - neye1[1];
			eye1[2] = lookat1[2] - neye1[2];

			break;
		}
	//}
	last_x1 = x;
	last_y1 = y;
	glutPostRedisplay();
}








//________ help

void printHelp(){
	std::cout <<"Mouse controls:\n\n";
	std::cout <<"- Navigation around scene\n";
	std::cout <<"    Press and drag mouse in drawing area  use\n";
	std::cout <<"    o Left mouse button for translation.\n";
	std::cout <<"    o Middle mouse button for zoom.\n";
	std::cout <<"    o Right mouse button for rotation.\n";
 	std::cout <<"- Ctrl click to emphasize/deamphasize tube\n\n\n";
  
  std::cout <<"Keyboard controls:\n\n";
	std::cout <<"  h - help\n";
  std::cout <<"  q - quit\n";
  std::cout <<"  m - color by function value means\n";
  std::cout <<"  g - width by function variances\n";
  std::cout <<"  + - increase scale (thickness of tubes etc)\n";
  std::cout <<"  - - decrease scale\n";
}






//_________ Keyboard

void keyboard(unsigned char key, int x, int y){
  switch(key)
	{
    
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
  case 'm':
	case 'M':
    loadColorValues("_fmean.data.hdr");    
    break;  
  case 'g':
	case 'G':
    loadWidthValues("_fvar.data.hdr");    
    break;   
  case '+':
    scale += 0.1;  
    break;
  case '-':
    scale -= 0.1;  
    break; 
	}


  glutPostWindowRedisplay(ms);
}







void reshape1(int w, int h)
{
  width = w;
  height = h;
  glViewport(0, 0, w, h);       /* Establish viewing area to cover entire window. */
  glMatrixMode(GL_PROJECTION);  /* Start modifying the projection matrix. */
  glLoadIdentity();
	gluPerspective(80, w/(float)h, 1, 100);
}




void init1(){  
  
  // initialize the camera
	eye1[0] = 0;
	eye1[1] = 4;
	eye1[2] = 10;
	lookat1[0] = 0;
	lookat1[1] = 0;
	lookat1[2] = 0;

  
	glEnable(GL_LIGHTING); 	
  GLfloat mat_spec[]={1, 1, 1, 1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 
 
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);

	//Global ambient light
	GLfloat light_ambient[] = {0.8, 0.8, 0.8, 1};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
	
  //Local Viewer
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	
	//Light 45 degrees up/right to viewer 
	GLfloat white_light[] = {1, 1, 1, 1};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightf(GL_LIGHT0, GL_SPECULAR, 2);
	//glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.1f);
	//glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.000001f);
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
  
  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
}




int main(int argc, char **argv){	

  
  //Command line parsing
  TCLAP::CmdLine cmd("HDViz", ' ', "1");
  
  try{
	  cmd.parse( argc, argv );
	} 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }


  printHelp();

  //Read information

  //edges 
  edges = LinalgIO<unsigned int>::readMatrix("Edges.data.hdr");
  //read layout information matrices
  samples = new DenseMatrix<Precision>[edges.N()];
  fSamples = new DenseMatrix<Precision>[edges.N()];
  pSamples = new DenseMatrix<Precision>[edges.N()];
  curves = new DenseMatrix<Precision>[edges.N()];
  for(unsigned int i=0; i< edges.N(); i++){
    std::stringstream ss1;
    ss1 << "ps_cell_" << i << "_Y.data.hdr";
    samples[i] = LinalgIO<Precision>::readMatrix(ss1.str());
    
    std::stringstream ss2;
    ss2 << "ps_cell_" << i << "_Yp.data.hdr";
    pSamples[i] = LinalgIO<Precision>::readMatrix(ss2.str());

    
    std::stringstream ss3;
    ss3 << "ps_cell_" << i << "_f.data.hdr";
    fSamples[i] = LinalgIO<Precision>::readMatrix(ss3.str());

    
    std::stringstream ss4;
    ss4 << "ps_cell_" << i << "_Rs.data.hdr";
    curves[i] = LinalgIO<Precision>::readMatrix(ss4.str());

  }
  //Load color and width informations
  yc = new DenseVector<Precision>[edges.N()];
  yw = new DenseVector<Precision>[edges.N()];

  loadColorValues("_fmean.data.hdr");
  loadWidthValues("_pk.data.hdr");

  selection= new bool[edges.N()];
  for(unsigned int i=0; i<edges.N(); i++){
    selection[i] = true;
  }

  E = LinalgIO<unsigned int>::readVector("Extrema.data.hdr");
  Xall = LinalgIO<Precision>::readMatrix("Geom.data.hdr");
  Ef = LinalgIO<Precision>::readVector("ExtremaValues.data.hdr");

  //GL stuff
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
  
  
  ms = glutCreateWindow("Morse-Smale Complex");
  glutDisplayFunc(display1);
  glutReshapeFunc(reshape1);
  glutMouseFunc(mouse1);
	glutMotionFunc(motion1);
  glutKeyboardFunc(keyboard);
  init1();

  glutMainLoop();

  return 0;
}
