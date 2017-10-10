#ifndef HDVIZDATA_H
#define HDVIZDATA_H

#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "Linalg.h"
#include "colormapper.h" 
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
    Precision getSelectedCoordinate(int selectedCell, int selectedPoint, int index);
    Precision getSelectedVariance(int selectedCell, int selectedPoint, int index);

    // Morse-Smale edge information.
    FortranLinalg::DenseMatrix<int>& getEdges();    
    FortranLinalg::DenseVector<Precision>& getPersistence();
    FortranLinalg::DenseVector<std::string>& getNames();
    FortranLinalg::DenseMatrix<Precision>* getLayout();

    // Extrema Layouts
    FortranLinalg::DenseVector<Precision>& getExtremaValues();
    FortranLinalg::DenseVector<Precision>& getExtremaNormalized();
    FortranLinalg::DenseVector<Precision>& getExtremaWidths();
    FortranLinalg::DenseMatrix<Precision>& getExtremaLayout();
    
    // Number of samples used for rendering.
    int getNumberOfSamples();   
    
    // Set which persistence level and layout to use.
    void loadData(int level);
    void setLayout(HDVizLayout layout, int level);
       
    // Cell reconstruction
    FortranLinalg::DenseMatrix<Precision>* getReconstruction(/* int persistenceLevel */);    
    FortranLinalg::DenseMatrix<Precision>* getVariance(/* int persistenceLevel */);
    FortranLinalg::DenseMatrix<Precision>* getGradient(/* int persistenceLevel */);
    FortranLinalg::DenseVector<Precision>& getRMin();
    FortranLinalg::DenseVector<Precision>& getRMax(); 
    FortranLinalg::DenseVector<Precision>& getRsMin(); 
    FortranLinalg::DenseVector<Precision>& getRsMax();      
    FortranLinalg::DenseVector<Precision>& getGradientMin();
    FortranLinalg::DenseVector<Precision>& getGradientMax();
    
    Precision getExtremaMinValue();
    Precision getExtremaMaxValue();
    Precision getZMin();
    Precision getZMax();

    // color/width and transparent width values
    FortranLinalg::DenseVector<Precision> *yc;   // value / color
    FortranLinalg::DenseVector<Precision> *z;    // point positions
    FortranLinalg::DenseVector<Precision> *yw;   // width / radii (std)
    FortranLinalg::DenseVector<Precision> *yd;   // density

    // ColorMapper for each cell
    ColorMapper<Precision> colormap;
    ColorMapper<Precision> dcolormap;

    int getMinPersistenceLevel() { return minLevel; }
    int getMaxPersistenceLevel() { return maxLevel; }
        
  private:
    void loadLayout(std::string type, std::string extFile, int level);    
    void loadColorValues(std::string type, int level);
    void loadWidthValues(std::string type, int level);
    void loadDensityValues(std::string type, int level);
    void loadReconstructions(int level);
    
    FortranLinalg::DenseVector<Precision> pSorted;  //  Persistence.data.hdr
    // Morse-Smale edge information.
    FortranLinalg::DenseMatrix<int> edges;          //  Crystals_<level>.data.hdr
    // Cell layouts
    FortranLinalg::DenseMatrix<Precision> *L;   // Point Positions

    // Number of smaples per cell for rendering.
    int nSamples;

    FortranLinalg::DenseVector<std::string> m_names;
    int minLevel;
    int maxLevel;
    HDVizLayout layout;

    // Extrema Layouts
    FortranLinalg::DenseVector<Precision> ef;   // Extrema Values
    FortranLinalg::DenseVector<Precision> ez;   // Extrema Values normalized [0,1]
    FortranLinalg::DenseVector<Precision> ew;   // Extrema Widths
    FortranLinalg::DenseMatrix<Precision> eL;   // Extrema layout 

    // filenames
    std::string m_path;

    // Cell Reconstruction
    FortranLinalg::DenseMatrix<Precision> *R;      // mean
    FortranLinalg::DenseMatrix<Precision> *Rvar;   // std-dev
    FortranLinalg::DenseMatrix<Precision> *gradR;  // gradient
    FortranLinalg::DenseVector<Precision> Rmin; 
    FortranLinalg::DenseVector<Precision> Rmax;
    FortranLinalg::DenseVector<Precision> Rsmin; 
    FortranLinalg::DenseVector<Precision> Rsmax;
    FortranLinalg::DenseVector<Precision> Rvmin;
    FortranLinalg::DenseVector<Precision> Rvmax;
    FortranLinalg::DenseVector<Precision> gRmin; 
    FortranLinalg::DenseVector<Precision> gRmax; 
    Precision vmax;

    FortranLinalg::DenseVector<Precision> Lmin, Lmax;

    Precision efmin, efmax;
    Precision zmin, zmax;
};

#endif
