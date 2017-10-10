#ifndef DISPLAYCURVES_H
#define DISPLAYCURVES_H

#include "Display.h"
#include <string>

template<typename TPrecision>
class DisplayCurves : public Display{

  private:
    HDVizData *data;
    HDVizState *state;
    FTGLPixmapFont font;
    void setFontSize(double fsize){
      fsize *= scale;
      if(fsize<5){
        fsize = 5;
      }
      font.FaceSize((int)fsize);
    }

    int width, height;


    TPrecision scale;
    TPrecision tx, ty;

    int last_x;
    int last_y;
    int cur_button;
    int mod;



  public:

    DisplayCurves(HDVizData *data, HDVizState *state, std::string fontname) : data(data),
            state(state), font(fontname.c_str()){      
      scale = 1;
      cur_button = -1;
      ty = 0;
    };

    std::string title(){
      return "Mean, Std and Gradient";
    };



    void init(){  
      glClearColor(1, 1, 1, 0);
      glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      glEnable(GL_LINE_SMOOTH);       
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    }

    void reshape(int w, int h)
    {
      width = w;
      height = h;
      glViewport(0, 0, w, h);      
      glMatrixMode(GL_PROJECTION);  
      glLoadIdentity();
      gluOrtho2D(0, w, ty, ty+h/scale);
    };



    void display(){  
      glMatrixMode(GL_MODELVIEW); 	
      glClear(GL_COLOR_BUFFER_BIT);

      glLoadIdentity();
      int M = data->getReconstruction()[state->selectedCell].M();
      Precision h = (Precision) height / M;

      setFontSize((int)(h*0.5f));
      //int off = 2.5*font.Advance("x");
      //GLfloat rpos[4];
      for(int i=0; i< M; i++){
        drawCurve(0, width, height - i*h, height - (i+1)*h, i);

/*
        glColor3f(0.3f,0.3f,0.3f);
        glRasterPos2f(off*0.1f, height - i*h - h*0.6f);
        setFontSize((int)(h*0.5f));
        font.Render("x");
        glGetFloatv(GL_CURRENT_RASTER_POSITION, rpos);
        glRasterPos2f(rpos[0]+font.Advance("x"), rpos[1]- h*0.15f);
        std::stringstream ss;
        ss << i;
        setFontSize((int)(h*0.2f));
        font.Render(ss.str().c_str());
*/

        glLineWidth(1.f); 
        glColor3f(0.9f,0.9f,0.9f);
        glBegin(GL_LINES);
        glVertex2f(0, i*h);
        glVertex2f(width, i*h);  
        glEnd();   

      }

/*
      glLineWidth(4.f); 
      glColor3f(0,0,0);
      glBegin(GL_LINES);
      glVertex2f(off, 0);
      glVertex2f(off, height);   
      glEnd();
*/


      glutSwapBuffers();

    };

    void printHelp(){      
      std::cout <<"Mouse controls:\n\n";
      std::cout <<"- Navigation\n";
      std::cout <<"    Press and drag mouse in drawing area  use\n";
      std::cout <<"    o Left mouse button for scrooling.\n";
      std::cout <<"    o Middle mouse button for zoom.\n\n\n";
    };

    void keyboard(unsigned char key, int x, int y){
	switch(key){
	  case 'r':
	  case 'R':
	    scale = 1;
	    tx =ty = 0;
	    reshape(width, height);
            glutPostRedisplay();
            break;
         default:
           break;
	}
    };

    void mouse(int button, int state, int x, int y){      
      last_x = x;
      last_y = y;
      mod = glutGetModifiers();
      if (state == GLUT_DOWN){
        cur_button = button;
   	if(button == GLUT_RIGHT_BUTTON){
           ty += height/scale - y/scale; 
           scale *= 5;
	   ty -= height/(scale*2.f);
           reshape(width, height);
 	   glutPostRedisplay();		
	}
      }
      else if (button == cur_button){
        cur_button = -1;
      }

    };

    void motion(int x, int y){ 
      int dx = x-last_x;
      int dy = y-last_y;



      switch(cur_button){
        case GLUT_LEFT_BUTTON:
          // translate
          tx -= dx*0.1;
          ty += dy*0.1;  
          
          reshape(width, height);
          break;

        case GLUT_MIDDLE_BUTTON:

          scale += dy*0.01;
          reshape(width, height);
          break;

        case GLUT_RIGHT_BUTTON:

          break;
      }
      last_x = x;
      last_y = y;
      glutPostRedisplay();
    };




  private:

    void drawCurve(Precision l, Precision r, Precision t, Precision b, int i){    
      Precision w = r-l;
      Precision h = t-b;
      Precision h5 = h/5.f;
      Precision hd = h-2*h5;


      Precision offw1 = w/8.f;
      Precision w1 = w-2*offw1;



   
      glColor3f(0.7, 0.7, 0.7);
      glLineWidth(4.f); 
      glBegin(GL_LINES);
      glVertex2f(l+offw1, t-h5);
      glVertex2f(l+offw1, b+h5);
      glEnd();

      glLineWidth(4.f); 
      glBegin(GL_LINES);
      glVertex2f(l+offw1, b+h5);
      glVertex2f(l+offw1+w1, b+h5);
      glEnd();


      glLineWidth(6.f); 
      glBegin(GL_LINE_STRIP);
      glColor3f(0.1, 0.1, 0.1);
      for (int j = 0; j < data->getNumberOfSamples(); j++){
	      Precision m = data->getReconstruction()[state->selectedCell](i, j);
      	m = (m - data->getRsMin()(i))   / ( data->getRsMax()(i) - data->getRsMin()(i) ) * hd;
      	glVertex2f(l+offw1+data->z[state->selectedCell](j)*w1, b + h5 + m);
      }
      glEnd();        
      
      glLineWidth(2.f); 
      glBegin(GL_LINE_STRIP);
      glColor3f(0.6, 0.6, 0.6);
      for (int j = 0; j < data->getNumberOfSamples(); j++){
        Precision v = data->getVariance()[state->selectedCell](i, j);
        v = v / ( data->getRsMax()(i) - data->getRsMin()(i) ) * hd ;
	      Precision m = data->getReconstruction()[state->selectedCell](i, j);
      	m = (m - data->getRsMin()(i))   / ( data->getRsMax()(i) - data->getRsMin()(i) ) * hd;
      	glVertex2f(l+offw1+data->z[state->selectedCell](j)*w1, b + h5 + m + v);
      }
      glEnd();       
      
      glBegin(GL_LINE_STRIP);
      for (int j = 0; j < data->getNumberOfSamples(); j++){
        Precision v = data->getVariance()[state->selectedCell](i, j);
        v = v / ( data->getRsMax()(i) - data->getRsMin()(i) ) * hd ;
	      Precision m = data->getReconstruction()[state->selectedCell](i, j);
      	m = (m - data->getRsMin()(i))   / ( data->getRsMax()(i) - data->getRsMin()(i) ) * hd;
      	glVertex2f(l+offw1+data->z[state->selectedCell](j)*w1, b + h5 + m - v);
      }
      glEnd();    


     //labels 
     setFontSize(h*0.1f);
     glColor3f(0.3f,0.3f,0.3f);  
          
     
     std::stringstream ss1;
     ss1 << std::setiosflags(std::ios::fixed) << std::setprecision(2) << data->getExtremaMinValue();
     Precision a = font.Advance(ss1.str().c_str());
     glRasterPos2f(l+offw1-a/2, b + h5 - 0.12f*h);
     font.Render(ss1.str().c_str());     
     
     std::stringstream ss2;
     ss2 << std::setiosflags(std::ios::fixed) << std::setprecision(2) << data->getExtremaMaxValue();
     a = font.Advance(ss2.str().c_str());
     glRasterPos2f(l+offw1 +w1 - a/2, b + h5 - 0.12f*h);
     font.Render(ss2.str().c_str());
      
     std::stringstream ssa1;
     ssa1 << std::setiosflags(std::ios::fixed) << std::setprecision(2) << data->getRsMin()(i);
     a = font.Advance(ssa1.str().c_str());
     glRasterPos2f(l+offw1-1.2*a, b + h5);
     font.Render(ssa1.str().c_str());   

     std::stringstream ssa2;
     ssa2 << std::setiosflags(std::ios::fixed) << std::setprecision(2) << data->getRsMax()(i);
     a = font.Advance(ssa2.str().c_str());
     glRasterPos2f(l+offw1-1.2*a, t - h5);
     font.Render(ssa2.str().c_str());   

     //selected point location      
     std::stringstream ssm;
     ssm << std::setiosflags(std::ios::fixed) << std::setprecision(2) <<
       data->getReconstruction()[state->selectedCell](i, state->selectedPoint) << " ";

     std::stringstream sse;
     sse << std::setiosflags(std::ios::fixed) << std::setprecision(2);
     sse << "(" << data->yc[state->selectedCell](state->selectedPoint) << ")";

     Precision v = data->getVariance()[state->selectedCell](i, state->selectedPoint);
     v = v / ( data->getRsMax()(i) - data->getRsMin()(i) ) * hd ;
	   Precision m = data->getReconstruction()[state->selectedCell](i, state->selectedPoint);
     m = (m - data->getRsMin()(i))   / ( data->getRsMax()(i) - data->getRsMin()(i) ) * hd;
     Precision wz = data->z[state->selectedCell](state->selectedPoint)*w1;

     a = font.Advance(ssm.str().c_str());
     glRasterPos2f(l+offw1+wz-a, b + h5+m+v+0.004f*h);
     font.Render(ssm.str().c_str());


     std::vector<Precision> color =
        data->colormap.getColor(data->yc[state->selectedCell](state->selectedPoint));
     glColor3f(color[0], color[1], color[2]);   
     
     glRasterPos2f(l+offw1+wz, b + h5+m+v+0.004f*h);
     font.Render(sse.str().c_str());


     glLineWidth(3.f); 
     glBegin(GL_LINES);
     glVertex2f(l+offw1+wz,
         b+h5+m-v);
     glVertex2f(l+offw1+wz,
         b+h5+m+v);
     glEnd();

     //name
     glColor3f(0.1f,0.1f,0.1f);
     setFontSize(h*0.25f);
     a = font.Advance(data->getNames()(i).c_str());
     glRasterPos2f(l+offw1+w1-a+offw1/2, t - h5);
     font.Render(data->getNames()(i).c_str());  



    };




};

#endif
