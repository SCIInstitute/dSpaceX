#ifndef DISPLAYIMAGEPCA_H
#define DISPLAYIMAGEPCA_H

#include "MyConfig.h"
#include "ImageVectorConverter.h"
#include "ImageIO.h"
#include "PCA.h"
#include <string>

template<typename TImage, typename TPrecision>
class DisplayImagePCA : public Display{

  private:

    typedef TImage Image;
    typedef typename Image::Pointer ImagePointer;
    typedef ImageVectorConverter<Image> Converter;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;

    ImagePointer image;
    ImagePointer imageStd;
    ImagePointer imageG;
    ImagePointer mask;
    Converter *converter;
    ImageRegion bounds;
    Precision Imin, Imax;
    Precision gImin, gImax;
    PCA<TPrecision> *pca;
    int oldSelectedCell;
    FortranLinalg::DenseMatrix<TPrecision> pcaR;
    FortranLinalg::DenseMatrix<TPrecision> pcaRstd;
    FortranLinalg::DenseMatrix<TPrecision> pcagR;

    unsigned int slice[3];
    int sliceIndex;
    int width, height;

    HDVizData *data;
    
    FTGLPixmapFont font;
    void setFontSize(int fsize){
      if(fsize<5){
        fsize = 5;
      }
      font.FaceSize(fsize);
    }

  public:

    DisplayImagePCA(HDVizData *d, std::string fontname):data(d),
     font(fontname.c_str()){
       slice[0] = 0;
       slice[1] = 0;
       slice[2] = 0;
       oldSelectedCell = -1;    
       sliceIndex = DIMENSION-1;        
    };

    void printHelp(){
           std::cout <<"Mouse controls:\n\n";


      std::cout <<"Keyboard controls:\n\n";
      std::cout <<"  h - help\n";
#if DIMENSION == 3
      std::cout <<"  > - next slice\n";
      std::cout <<"  < - previous\n";
      std::cout <<"  x - view x slice \n";
      std::cout <<"  y - view y slice \n";
      std::cout <<"  z - view z slice \n";
#endif
    };


    std::string title(){
      return "Range Image";
    };

    void init(){  
      glClearColor(1, 1, 1, 0); 
    };

    void reshape(int w, int h){
      width = w;
      height = h;
      glViewport(0, 0, w, h);      
      glMatrixMode(GL_PROJECTION);  
      glLoadIdentity();
      gluOrtho2D(0, w, 0, h);
    };




    //_________ range plotering
    void display(){ 
     
      using namespace FortranLinalg;
      if(oldSelectedCell != data->selectedCell){
        oldSelectedCell = data->selectedCell;
        pcaR.deallocate();
        pcaR = pca->unproject(data->R[data->selectedCell]);
        
        //DenseVector<Precision> tmp = Linalg<TPrecision>::RowMax(pcaR);
        Imax = Linalg<TPrecision>::MaxAll(pcaR);
        //tmp.deallocate();
        //tmp = Linalg<TPrecision>::RowMin(pcaR);
        Imin =Linalg<TPrecision>::MinAll(pcaR);

        pcaRstd.deallocate();
        pcaRstd = pca->unproject(data->Rvar[data->selectedCell], false);

        pcagR.deallocate();
        pcagR = pca->unproject(data->gradR[data->selectedCell], false);
        
        //tmp = Linalg<TPrecision>::Max(pcagR);
        gImax = Linalg<TPrecision>::MaxAll(pcagR);
        //tmp = Linalg<TPrecision>::Min(pcagR);
        gImin = Linalg<TPrecision>::MinAll(pcagR);

        gImax = std::max(fabs(gImin), gImax);
        gImin = -gImax;

        //tmp.deallocate();
      }

      
      converter->fillImage(pcaR, data->selectedPoint, image);
      converter->fillImage(pcaRstd, data->selectedPoint, imageStd);
      converter->fillImage(pcagR, data->selectedPoint, imageG);

      ImageIndex start = bounds.GetIndex(); 
      start[sliceIndex]+=slice[sliceIndex];
      ImageIndex current;
      int index1 = 0;
      int index2 = 1;
      if(sliceIndex == 0){
        index1 = 2;
      }
      else if(sliceIndex ==1){
        index2 = 2;
      }

      Precision nw = bounds.GetSize(index1);
      Precision nh = bounds.GetSize(index2);
      Precision sw = width/nw;
      Precision h2 = height/2.f;
      Precision sh = h2/nh;

      glMatrixMode(GL_MODELVIEW); 	
      glClear(GL_COLOR_BUFFER_BIT);

      glBegin(GL_QUADS);
      for(int i=0; i<nw; i++){
        current = start;
        current[index1] += i;
        Precision x = i*sw;
        for(int j=0; j<nh; j++){
          Precision y = j*sh;
          Precision cI = (image->GetPixel(current)-Imin)/(Imax-Imin);
          Precision cR = fabs(imageStd->GetPixel(current))/(Imax-Imin);
          Precision cI2 = cI / (1.f+cR);
          glColor3f(cI, cI2, cI2);
          glVertex2f(x    , y);
          glVertex2f(x+sw , y);
          glVertex2f(x+sw , y+sh);
          glVertex2f(x    , y+sh);

          Precision gI = imageG->GetPixel(current);
          if(gI<0){
            glColor3f(0, gI/gImin, 0);
          }
          else{
            glColor3f(gI/gImax, 0, 0);
          }
          glVertex2f(x    , h2+y);
          glVertex2f(x+sw , h2+y);
          glVertex2f(x+sw , h2+y+sh);
          glVertex2f(x    , h2+y+sh);


          current[index2]++;
        }
      }
      glEnd();

      std::vector<Precision> color =
        data->colormap.getColor(data->yc[data->selectedCell](data->selectedPoint));
      glColor3f(color[0], color[1], color[2]);   

      std::stringstream sse;
      sse << std::setiosflags(std::ios::fixed) << std::setprecision(2);
      sse <<  data->yc[data->selectedCell](data->selectedPoint) ;  

      setFontSize(40);
      glRasterPos2f(10, 10);
      font.Render(sse.str().c_str());

      glutSwapBuffers();

    }

    void keyboard(unsigned char key, int x, int y){  
      switch(key)
      {

#if DIMENSION == 3  
        case 'x':
        case 'X':
          sliceIndex = 0;
          break;
        case 'y':
        case 'Y':
          sliceIndex = 1;
          break;

        case 'z':
        case 'Z':
          sliceIndex = 2;
          break;
        case '>':
          slice[sliceIndex] += 1;
          if(slice[sliceIndex] >= bounds.GetSize(sliceIndex) ){
            slice[sliceIndex] = bounds.GetSize(sliceIndex)-1;
          }
          break;
        case '<':
          slice[sliceIndex] -= 1;
          if(slice[sliceIndex] < 0){
            slice[sliceIndex] = 0;
          }
          break;
#endif   
      }
      glutPostRedisplay();
    }

    void mouse(int button, int state, int x, int y){
    }

    void motion(int x, int y){
    }

    bool loadAdditionalData(){  
      using namespace FortranLinalg;
      try{
        mask = ImageIO<Image>::readImage("mask.mhd");
        converter = new Converter(mask);
        bounds = converter->getBounds();
        image = converter->createImage();
        imageG = converter->createImage();
        imageStd = converter->createImage();
        Imin = Linalg<Precision>::Min(data->Rmin);
        Imax = Linalg<Precision>::Min(data->Rmax);
        std::ifstream file;
        file.open("pcafiles.txt");
        std::string evecs;
        getline(file, evecs);
        std::string evals;
        getline(file, evals);
        std::string mean;
        getline(file, mean);
        file.close();

        std::cout << evecs << std::endl;

        DenseMatrix<TPrecision> evec = LinalgIO<Precision>::readMatrix(evecs);
        DenseVector<TPrecision> e = LinalgIO<Precision>::readVector(evals);
        std::cout << mean << std::endl;
        ImagePointer meanImage = ImageIO<Image>::readImage(mean);
        DenseVector<TPrecision> m = converter->extractVector(meanImage);
        std::cout << evals << std::endl;
        pca = new PCA<TPrecision>(evec, e, m);


      }
      catch(char *c){
        std::cout << c << std::endl;
        return false;
      }
//catch(...){
//        std::cout << "Failed to read mask image required for image display" << std::endl;
//        return false;
//      }
      return true;
    };
};

#endif
