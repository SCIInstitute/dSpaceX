#ifndef DISPLAYRANGE_H
#define DISPLAYRANGE_H

#include "Display.h"
#include <string>

template<typename TPrecision>
class DisplayRange : public Display{

  private:
    HDVizData *data;
    HDVizState *state;
    FTGLPixmapFont font;
    void setFontSize(int fsize){
      fsize *= scale;
      if(fsize<5){
        fsize = 5;
      }
      font.FaceSize(fsize);
    }

    int width, height;

    TPrecision scale;
    TPrecision tx, ty;

    int last_x;
    int last_y;
    int cur_button;
    int mod;


  public:

    DisplayRange(HDVizData *data, HDVizState *state, std::string fontname) : data(data),
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
      int off = 0;//2.5*font.Advance("x");
      int w = width - off;
      //GLfloat rpos[4];
      for(int i=0; i< M; i++){
        drawRange(off, off + w*0.6, height - i*h, height - (i+1)*h, i);
        drawTangent(off+w*0.6f, width, height - i*h, height - (i+1)*h, i);   


        /*glColor3f(0.3f,0.3f,0.3f);
        glRasterPos2f(off*0.1f, height - i*h - h*0.6f);
        setFontSize((int)(h*0.5f));
        font.Render("x");
        glGetFloatv(GL_CURRENT_RASTER_POSITION, rpos);
        glRasterPos2f(rpos[0]+font.Advance("x"), rpos[1]- h*0.15f);
        std::stringstream ss;
        ss << i;
        setFontSize((int)(h*0.2f));
        font.Render(ss.str().c_str());*/

        glLineWidth(1.f); 
        glColor3f(0.9f,0.9f,0.9f);
        glBegin(GL_LINES);
        glVertex2f(0, i*h);
        glVertex2f(width, i*h);  
        glEnd();   
      }

/*
      glLineWidth(2.f); 
      glColor3f(0.3,0.3,0.3);
      glBegin(GL_LINES);
      glVertex2f(off + w*0.6f, 0);
      glVertex2f(off + w*0.6f, height);   
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

    void drawRange(Precision l, Precision r, Precision t, Precision b, int i){    
      Precision w = r-l;
      Precision h = t-b;
      Precision h4 = h/4.f;
      Precision h3 = h/3.f;
      Precision h25 = h/2.5f;
      Precision h2 = h/2.f;


      Precision w1 = w/2.f;
      Precision offw1 = (w-w1)/2.f;

      Precision glmax = std::max(fabs(data->getGradientMin()(i)), fabs(data->getGradientMax()(i)));
      Precision rlmax = std::max(fabs(data->getRMin()(i)), fabs(data->getRMax()(i)));

      Precision m = data->getReconstruction()[state->selectedCell](i, state->selectedPoint);
      m = (m - data->getRMin()(i))   / ( data->getRMax()(i) - data->getRMin()(i) ) * w1;

      glColor3f(0.7, 0.7, 0.7);
      glLineWidth(2.f); 
      glBegin(GL_LINES);
      glVertex2f(l+offw1, t-h4);
      glVertex2f(l+offw1, b+h4);
      glVertex2f(l+offw1 + w1, t-h4);
      glVertex2f(l+offw1 + w1, b+h4);
      glEnd();

      glLineWidth(1.f); 
      glBegin(GL_LINES);
      glVertex2f(l+offw1, t-h2);
      glVertex2f(l+offw1+w1, t-h2);
      glEnd();


      Precision v = data->getVariance()[state->selectedCell](i, state->selectedPoint);
      v = v / ( data->getRMax()(i) - data->getRMin()(i) ) * w1 ;


      glColor3f(0.4, 0.4, 0.4);
      glBegin(GL_QUADS);
      glVertex2f(l+offw1+m+v, t-h25);
      glVertex2f(l+offw1+m+v, b+h25); 
      glVertex2f(l+offw1+m-v, b+h25); 
      glVertex2f(l+offw1+m-v, t-h25); 
      glEnd();



      Precision gm = data->getGradient()[state->selectedCell](i, state->selectedPoint);
      gm = gm  / ( data->getRMax()(i) - data->getRMin()(i) ) * w1 / (2*std::max(glmax, rlmax));

      std::vector<Precision> color =
        data->colormap.getColor(data->yc[state->selectedCell](state->selectedPoint));
      glColor3f(color[0], color[1], color[2]);   
      glLineWidth(3.f); 
      glBegin(GL_LINES);
      glVertex2f(l+offw1+m, t-h3);
      glVertex2f(l+offw1+m, b+h3);

      glVertex2f(l+offw1+m, t-h2);
      glVertex2f(l+offw1+m+gm, t-h2);
      glEnd();   




      std::stringstream ssm;
      ssm << std::setiosflags(std::ios::fixed) << std::setprecision(2) <<
        data->getReconstruction()[state->selectedCell](i, state->selectedPoint);

      std::stringstream sse;
      sse << std::setiosflags(std::ios::fixed) << std::setprecision(2);
      sse << "(" << data->yc[state->selectedCell](state->selectedPoint) << ")";


      std::stringstream ss1;
      ss1 << std::setiosflags(std::ios::fixed) << std::setprecision(2) << data->getRMin()(i);

      std::stringstream ss2;
      ss2 << std::setiosflags(std::ios::fixed) << std::setprecision(2) << data->getRMax()(i);



      setFontSize((int)(h*0.1f));

      Precision a = font.Advance(" ");
      glRasterPos2f(l+offw1+m+a/2, t - h3 + 0.04f*h);
      font.Render(sse.str().c_str());


      glColor3f(0.3f,0.3f,0.3f);    
      Precision a2 = font.Advance(ssm.str().c_str());
      glRasterPos2f(l+offw1+m-a/2-a2, t - h3 + 0.04f*h);
      font.Render(ssm.str().c_str());

      a = font.Advance(ss1.str().c_str());
      glRasterPos2f(l+offw1-a/2, b + 0.04f*h);
      font.Render(ss1.str().c_str());

      a = font.Advance(ss2.str().c_str());
      glRasterPos2f(l+offw1+w1-a/2, b + 0.04f*h);
      font.Render(ss2.str().c_str());     
      
      //name
      glColor3f(0.1f,0.1f,0.1f);
      setFontSize((int)(h*0.15f));
      a = font.Advance(data->getNames()(i).c_str());
      glRasterPos2f(l+offw1+w1+offw1*0.7f-a/2.f, t - h/6.f);
      font.Render(data->getNames()(i).c_str());  
    };




    void drawTangent(Precision l, Precision r, Precision t, Precision b, int i){  
      Precision w = r-l;
      Precision h = t-b;
      Precision h4 = h/4.f;
      Precision h3 = h/3.f;
      Precision h25 = h/2.5f;

      Precision w2 = w/1.3f;
      Precision offw2 = (w-w2)/2.f;


      Precision m = data->getGradient()[state->selectedCell](i, state->selectedPoint);

      Precision mmax1 = std::max(fabs(data->getRMin()(i)), fabs(data->getRMax()(i)));
      Precision mmax2 = std::max(fabs(data->getGradientMin()(i)), fabs(data->getGradientMax()(i)));
      Precision mmax = std::max(mmax1, mmax2);
      if(mmax != 0){
        m = (m + mmax) / (2*mmax) * w2 ;
      }
      else{
	      m = 0;
      }
      Precision s = (-mmax1 + mmax) / (2*mmax) * w2 ;


      Precision mid = w2/2.f ;

      glColor3f(0.7, 0.7, 0.7);
      glLineWidth(2.f); 
      glBegin(GL_LINES);
      glVertex2f(l+offw2 + s, t-h4);
      glVertex2f(l+offw2 + s, b+h4);
      glVertex2f(l+offw2 + w2-s, t-h4);
      glVertex2f(l+offw2 + w2-s, b+h4);
      glVertex2f(l+offw2 + mid, t-h3);
      glVertex2f(l+offw2 + mid, b+h3);
      glEnd();

      glLineWidth(1.f); 
      glBegin(GL_LINES);
      glVertex2f(l+offw2, t-h/2.f);
      glVertex2f(l+offw2+w2, t-h/2.f);
      glEnd();

      glColor3f(0.2, 0.2, 0.2);
      glLineWidth(3.f); 
      glBegin(GL_LINES);
      glVertex2f(l+offw2+m, t-h25);
      glVertex2f(l+offw2+m, b+h25);
      glEnd();    



      std::stringstream ssm;
      ssm << std::setiosflags(std::ios::fixed) << std::setprecision(2);
      ssm << data->getGradient()[state->selectedCell](i, state->selectedPoint);

      std::stringstream ss1;
      ss1 << std::setiosflags(std::ios::fixed) << std::setprecision(2) << -mmax1;

      std::stringstream ss2;
      ss2 << std::setiosflags(std::ios::fixed) << std::setprecision(2) << mmax1;




      setFontSize((int)(h*0.1f));

      Precision a = font.Advance(ssm.str().c_str());
      glRasterPos2f(l+offw2+m-a/2, t - h3 + 0.04f*h);
      font.Render(ssm.str().c_str());


      glColor3f(0.3f,0.3f,0.3f);

      a = font.Advance(ss1.str().c_str());
      glRasterPos2f(l+offw2+s-a/2, b + 0.04f*h);
      font.Render(ss1.str().c_str());

      a = font.Advance(ss2.str().c_str());
      glRasterPos2f(l+offw2+w2-s-a/2, b + 0.04f*h);
      font.Render(ss2.str().c_str());
    };


};

#endif
