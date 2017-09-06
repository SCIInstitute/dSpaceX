#ifndef HDVIZDATA_H
#define HDVIZDATA_H

#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "Linalg.h"
#include "colormapper.h" 
#include "Display.h"
#include "Precision.h"
#include <string>

enum class HDVizLayout : char {
    ISOMAP  = 0,
    PCA = 1,
    PCA2 = 2,
};

class HDVizData{
  public:
    HDVizData(std::string path);
    Precision getSelectedCoordinate(int index);
    Precision getSelectedVariance(int index);
    
    void increasePersistanceLevel();        
    void decreasePersistanceLevel();
    int getPersistanceLevel();
    void setPersistenceLevel(int pl, bool update = true);
    void setLayout(HDVizLayout layout);
  

    FortranLinalg::DenseVector<std::string> names;

    // Number of smaples per cell for rendering.
    int nSamples;

    // Morse-Smale edge information.
    FortranLinalg::DenseMatrix<int> edges; 

    FortranLinalg::DenseVector<Precision> pSorted;

    unsigned nAll;

    // Extrema Layouts
    FortranLinalg::DenseVector<Precision> ef;
    FortranLinalg::DenseVector<Precision> ez;
    FortranLinalg::DenseVector<Precision> ew;
    FortranLinalg::DenseMatrix<Precision> eL;

    // Cell layouts
    FortranLinalg::DenseMatrix<Precision> *L;

    // Cell reconstruction
    FortranLinalg::DenseMatrix<Precision> *R;
    FortranLinalg::DenseMatrix<Precision> *Rvar;
    FortranLinalg::DenseMatrix<Precision> *gradR;
    FortranLinalg::DenseVector<Precision> Rmin; 
    FortranLinalg::DenseVector<Precision> Rmax;
    FortranLinalg::DenseVector<Precision> Rsmin; 
    FortranLinalg::DenseVector<Precision> Rsmax;
    FortranLinalg::DenseVector<Precision> Rvmin;
    FortranLinalg::DenseVector<Precision> Rvmax;
    Precision vmax;
    FortranLinalg::DenseVector<Precision> gRmin; 
    FortranLinalg::DenseVector<Precision> gRmax; 

    FortranLinalg::DenseVector<Precision> Lmin, Lmax;
    Precision efmin, efmax;

    Precision zmax, zmin;

    // filenames
    std::string m_path;

    // color/width and transparent width values
    FortranLinalg::DenseVector<Precision> *yc;
    FortranLinalg::DenseVector<Precision> *z;
    FortranLinalg::DenseVector<Precision> *yw;
    FortranLinalg::DenseVector<Precision> *yd;

    // ColorMapper for each cell
    ColorMapper<Precision> colormap;
    ColorMapper<Precision> dcolormap;

    int selectedCell;
    int selectedPoint;
        
  private:
    void loadLayout(std::string type, std::string extFile);
    void loadData();
    void loadColorValues(std::string type);
    void loadWidthValues(std::string type);
    void loadDensityValues(std::string type);
    void loadReconstructions();

    int minLevel;
    int currentLevel;
    HDVizLayout layout;
};

#endif
