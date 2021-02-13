#ifndef DISPLAYMOLECULE_H
#define DISPLAYMOLECULE_H

#include "Display.h"
#include "flinalg/DenseVector.h"
#include "flinalg/DenseMatrix.h"
#include "FTGL/FTGLPixmapFont.h"
#include "hdprocess/HDVizData.h"
#include "kernelstats/KernelInterpolator.h"

#include <cstdlib>
#include <string>


template<typename TPrecision>
class DisplayMolecule : public Display {
  private:
#define BUFSIZE 512

    int width, height;
    Precision dw, dh;

    TPrecision scale ;

    //naviagtion
    TPrecision zoom ;
    TPrecision tx ;
    TPrecision ty ;
    TPrecision rotation[3];
    int rotationAxis ;


    KernelInterpolator<TPrecision> *ki;
    KernelInterpolator<TPrecision> *kiW;
    FortranLinalg::DenseMatrix<TPrecision> X;
    FortranLinalg::DenseVector<TPrecision> xi;
    FortranLinalg::DenseMatrix<TPrecision> W;
    FortranLinalg::DenseVector<TPrecision> wi;


    //mouse 
    int last_x;
    int last_y;
    int cur_button;
    int mod;


    HDVizData *data;
    HDVizState *state;

    FTGLPixmapFont font;
    void setFontSize(int fsize){
      if(fsize<5){
        fsize = 5;
      }
      font.FaceSize(fsize);
    };



  public:

    DisplayMolecule(HDVizData *data, HDVizState *state, std::string fontname) : data(data), 
        state(state), font(fontname.c_str()){ 

      mod = 0;
      cur_button = -1;

      scale = 1;

      //naviagtion
      double ax = FortranLinalg::Linalg<Precision>::Max(data->getRMax());
      double in = FortranLinalg::Linalg<Precision>::Min(data->getRMin());
      zoom = 1;
      tx = 0;
      ty = 0;
      rotation[0] = -90;
      rotation[1] = 0;
      rotation[2] = 0;
      rotationAxis = 0;

      int n = data->getRMax().N()/3;
      FortranLinalg::DenseVector<TPrecision> tmp(n);
      for(int i=0; i<n; i++){
        tmp(i)= i;
      };
      ki = new KernelInterpolator<TPrecision>(tmp, 3);
      kiW = new KernelInterpolator<TPrecision>(tmp, 1);
      X=FortranLinalg::DenseMatrix<TPrecision>(3, n);
      W=FortranLinalg::DenseMatrix<TPrecision>(1, n);
      xi=FortranLinalg::DenseVector<TPrecision>(3);
      wi=FortranLinalg::DenseVector<TPrecision>(1);

    };



    std::string title(){
      return "Molecule View";
    };


    void reshape(int w, int h){
      width = w;
      height = h;
      glViewport(0, 0, w, h);       
      glMatrixMode(GL_PROJECTION);  
      glLoadIdentity();
      setupOrtho(w, h);
    };




    void init(){  

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_STENCIL_TEST);

      glEnable(GL_LIGHTING); 	
      //GLfloat mat_spec[]={1, 1, 1, 1};
      //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 

      glEnable(GL_COLOR_MATERIAL);
      glEnable(GL_DEPTH_TEST);

      //Global ambient light
      GLfloat light_ambient[] = {0.5, 0.5, 0.5, 1};
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);

      //Local Viewer
      glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
      glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);


      glEnable(GL_LIGHT0);

      // Create light components
      float ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
      float diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
      float specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };

      // Assign created components to GL_LIGHT0
      glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
      glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

      glClearColor(1, 1, 1, 0);
      glClearStencil(0x0);
      glShadeModel(GL_SMOOTH);
      glEnable(GL_POLYGON_SMOOTH);

      gleSetJoinStyle (TUBE_NORM_PATH_EDGE | TUBE_JN_ROUND); 
      gleSetNumSides(50);

      glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
      glHint(GL_FOG_HINT, GL_NICEST);
      glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

      glEnable(GL_RESCALE_NORMAL);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 


    };




    void printHelp(){
      std::cout <<"Mouse controls:\n\n";
      std::cout <<"- Navigation around scene\n";
      std::cout <<"    Press and drag mouse in drawing area  use\n";
      std::cout <<"    o Left mouse button for translation.\n";
      std::cout <<"    o Middle mouse button for zoom.\n";
      std::cout <<"    o Right mouse button for rotation.\n";
      std::cout <<"- Ctrl left click to emphasize/demphasize colored tube and select peak\n\n\n";
      std::cout <<"- Ctrl right mosue button down to move heat plot location along curve\n\n\n";

    };




    void display(void){

      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

      glMatrixMode(GL_MODELVIEW); 	
      glLoadIdentity();


      GLfloat view_light_pos[] = {3, 3, -3, 1};
      glLightfv(GL_LIGHT0, GL_POSITION, view_light_pos);




      glScalef(zoom, zoom, zoom);
      glTranslatef(tx,ty, 5); 
      glRotatef(rotation[0], 1, 0, 0); 
      glRotatef(rotation[1], 0, 1, 0); 
      glRotatef(rotation[2], 0, 0, 1); 
      glTranslatef(0,0, 0.5); 

      int index=0;
      for(int i=2; i<data->getRMin().N(); i+=3, index++ ){
         double x = data->getReconstruction(state->currentLevel)[state->selectedCell](i-2, state->selectedPoint);
         double y = data->getReconstruction(state->currentLevel)[state->selectedCell](i-1, state->selectedPoint);
         double z = data->getReconstruction(state->currentLevel)[state->selectedCell](i, state->selectedPoint);
         X(0, index) =x;
         X(1, index) =y;
         X(2, index) =z;
      }


      ki->setData(X);
      int nSamples=500;
      gleDouble points[nSamples+2][3];
      gleDouble radii[nSamples+2];
      gleDouble radiiW[nSamples+2];

      //int cell = state->selectedCell;
      //int point = state->selectedPoint;
      int nAtoms = data->getRMin().N()/3;
 
      for(int i=0; i<nAtoms; i++){
        int ind = i*3;
        double x = data->getVariance(state->currentLevel)[state->selectedCell](ind, state->selectedPoint);
        double y = data->getVariance(state->currentLevel)[state->selectedCell](ind+1, state->selectedPoint);
        double z = data->getVariance(state->currentLevel)[state->selectedCell](ind+2, state->selectedPoint);
        W(0, i) = sqrt( x*x + y*y + z*z );

      }
      kiW->setData(W);

        for(unsigned int k=0; k<nSamples; k++){
          double y =  ( X.N()-1.0) / (nSamples-1.0) * k;
          ki->evaluate(y, xi);    
          for(unsigned int m=0; m<xi.N(); m++){
            points[k+1][m] = xi(m);
          }
          radii[k+1] = 0.5;
          kiW->evaluate(y, wi); 
          radiiW[k+1] = 0.6 + wi(0);
        }    

        radii[0] = radii[1];
        radiiW[0] = radiiW[1];
        radii[nSamples+1] = radii[nSamples];
        radiiW[nSamples+1] = radiiW[nSamples];

        for(unsigned int m=0; m<3; m++){
          points[0][m] = points[1][m]+ points[2][m] - points[1][m];
          points[nSamples+1][m] = points[nSamples][m] + points[nSamples][m] - points[nSamples-1][m];
        }

        //draw atoms   
        glColor3f(0.5, 0.5, 0.5);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100);
        
        //draw tubes
        //glePolyCone_c4f(nSamples+2, points, NULL, radii);
        
        //draw sphers only
        for(int i=0; i<X.N(); i++ ){
         glTranslatef(X(0,i), X(1,i),  X(2,i));
         glutSolidSphere(0.5, 50, 50);
         glTranslatef(-X(0,i), -X(1,i),  -X(2,i));
        }


        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 1);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60);
        
        glColor4f(0.8, 0.8, 0.8, 0.5);

        //Draw outer tube depths only
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_FILL);
        
        glPolygonOffset(2, 0.1);
        glePolyCone_c4f(nSamples+2, points, NULL, radiiW);
        
        glDisable(GL_POLYGON_OFFSET_FILL);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


        //draw transparent outer tube but only front faces 
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);

        //glePolyCone_c4f(nSamples+2, points, NULL, radiiW);
        for(int i=0; i<nAtoms; i++){
          int ind = i*3;
          double x = data->getVariance(state->currentLevel)[state->selectedCell](ind, state->selectedPoint);
          double y = data->getVariance(state->currentLevel)[state->selectedCell](ind+1, state->selectedPoint);
          double z = data->getVariance(state->currentLevel)[state->selectedCell](ind+2, state->selectedPoint);
          glPushMatrix();
          glTranslatef(X(0,i), X(1,i),  X(2,i));
	  glScaled(0.6+x, 0.6+y, 0.6+z);
          glutSolidSphere(1, 50, 50);
          glPopMatrix();
        }

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    

        glClear(GL_DEPTH_BUFFER_BIT |GL_STENCIL_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);  
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);  
        glePolyCone_c4f(nSamples+2, points, NULL, radii);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);    
        glEnable(GL_STENCIL_TEST);  

        glDisable(GL_LIGHTING);

        glColor4f(1, 1, 1, 0);

        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        //glePolyCone_c4f(nSamples+2, points, NULL, radiiW);
        for(int i=0; i<nAtoms; i++){
          int ind = i*3;
          double x = data->getVariance(state->currentLevel)[state->selectedCell](ind, state->selectedPoint);
          double y = data->getVariance(state->currentLevel)[state->selectedCell](ind+1, state->selectedPoint);
          double z = data->getVariance(state->currentLevel)[state->selectedCell](ind+2, state->selectedPoint);
          glPushMatrix();
          glTranslatef(X(0,i), X(1,i),  X(2,i));
          glScaled(0.6+x, 0.6+y, 0.6+z);
          glutSolidSphere(1, 50, 50);
          glPopMatrix();
        }
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);


        for (int k = 0; k < data->getNumberOfLayoutSamples()+2; k++){
          radiiW[k] += scale*0.1/zoom;
        }

        glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
        
      
        //glePolyCone_c4f(nSamples+2, points, NULL, radiiW); 
        for(int i=0; i<nAtoms; i++){
          int ind = i*3;
          double x = data->getVariance(state->currentLevel)[state->selectedCell](ind, state->selectedPoint) + scale*0.1/zoom;
          double y = data->getVariance(state->currentLevel)[state->selectedCell](ind+1, state->selectedPoint)+ scale*0.1/zoom;
          double z = data->getVariance(state->currentLevel)[state->selectedCell](ind+2, state->selectedPoint)+ scale*0.1/zoom;
          glPushMatrix();
          glTranslatef(X(0,i), X(1,i),  X(2,i));
          glScaled(0.6+x, 0.6+y, 0.6+z);
          glutSolidSphere(1, 50, 50);
          glPopMatrix();
        }
        
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_LIGHTING);	
        glDisable(GL_STENCIL_TEST);  

        glutSwapBuffers();

    };






    void keyboard(unsigned char key, int x, int y){
      switch(key)
      {
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
      }
    };



    void mouse(int button, int state, int x, int y){
      last_x = x;
      last_y = y;
      mod = glutGetModifiers();
      if (state == GLUT_DOWN){
        cur_button = button;
      }
      else if (button == cur_button){
        cur_button = -1;
      }

    };



    // catch mouse move events
    void motion(int x, int y){
      int dx = x-last_x;
      int dy = y-last_y;



      switch(cur_button){
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

        case GLUT_RIGHT_BUTTON:
          // rotate
          Precision rot = rotation[rotationAxis] ;
          rot += dx;
          int r = (int)(rot/360);
          rotation[rotationAxis]= rot - r*360;

          break;
      }
      //}
      last_x = x;
      last_y = y;
      glutPostRedisplay();
};


private:


void setupOrtho(int w, int h){  
  Precision sx=1;
  Precision sy=1;
  if(w>h){
    sx = (Precision)w/h;
  }
  else{
    sy = (Precision)h/w;
  }
  glOrtho(-4*sx, 4*sx, -4*sy, 4*sy, 16, -16);
  dh = 8*sy;
  dw = 8*sx;
}




};

#endif
