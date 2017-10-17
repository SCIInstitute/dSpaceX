#pragma once

#include "HDVizData.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "Linalg.h"
#include "colormapper.h" 
#include "Precision.h"
#include <string>

class FileCachedHDVizDataImpl : public HDVizData {
  public:
    FileCachedHDVizDataImpl(std::string path);
    
    // Morse-Smale edge information.
    FortranLinalg::DenseMatrix<int>& getEdges(int persistenceLevel);    
    FortranLinalg::DenseVector<Precision>& getPersistence();
    FortranLinalg::DenseVector<std::string>& getNames();
    std::vector<FortranLinalg::DenseMatrix<Precision>>& getLayout(
        HDVizLayout layout, int persistenceLevel);

    // Extrema Layouts
    FortranLinalg::DenseVector<Precision>& getExtremaValues(int persistenceLevel);
    FortranLinalg::DenseVector<Precision>& getExtremaNormalized(int persistenceLevel);
    FortranLinalg::DenseVector<Precision>& getExtremaWidths(int persistenceLevel);
    FortranLinalg::DenseVector<Precision>& getExtremaWidthsScaled(int persistenceLevel);
    FortranLinalg::DenseMatrix<Precision>& getExtremaLayout(
        HDVizLayout layout, int persistenceLevel);
    
    // Number of samples used for layouts.
    int getNumberOfSamples();   
       
    // Cell reconstruction
    std::vector<FortranLinalg::DenseMatrix<Precision>>& getReconstruction(int persistenceLevel);
    std::vector<FortranLinalg::DenseMatrix<Precision>>& getVariance(int persistenceLevel);
    std::vector<FortranLinalg::DenseMatrix<Precision>>& getGradient(int persistenceLevel);
    FortranLinalg::DenseVector<Precision>& getRMin();
    FortranLinalg::DenseVector<Precision>& getRMax(); 
    FortranLinalg::DenseVector<Precision>& getRsMin(int persistenceLevel);
    FortranLinalg::DenseVector<Precision>& getRsMax(int persistenceLevel);
    FortranLinalg::DenseVector<Precision>& getGradientMin(int persistenceLevel);
    FortranLinalg::DenseVector<Precision>& getGradientMax(int persistenceLevel);
    
    Precision getExtremaMinValue(int persistenceLevel);
    Precision getExtremaMaxValue(int persistenceLevel);
    Precision getWidthMin(int persistenceLevel);
    Precision getWidthMax(int persistenceLevel);

    // color/width and transparent width values
    std::vector<FortranLinalg::DenseVector<Precision>>& getMean(
        int persistenceLevel);
    std::vector<FortranLinalg::DenseVector<Precision>>& getMeanNormalized(
        int persistenceLevel);
    std::vector<FortranLinalg::DenseVector<Precision>>& getWidth(
        int persistenceLevel);
    std::vector<FortranLinalg::DenseVector<Precision>>& getDensity(
        int persistenceLevel);

    // ColorMapper for each cell
    ColorMapper<Precision>& getColorMap(int persistenceLevel);
    ColorMapper<Precision>& getDColorMap(int persistenceLevel);

    int getMinPersistenceLevel(); 
    int getMaxPersistenceLevel();
        
  private:        
    void loadData(int level);
    void setLayout(HDVizLayout layout, int level);
    void loadLayout(std::string type, std::string extFile, 
        std::string minFile, std::string maxFile, int level);    
    void loadColorValues(std::string type, int level);
    void loadWidthValues(std::string type, int level);
    void loadDensityValues(std::string type, int level);
    void loadReconstructions(int level);
    void maybeSwapLevelCache(int level);
    void maybeSwapLayoutCache(HDVizLayout layout);
    
    FortranLinalg::DenseVector<Precision> pSorted;  //  Persistence.data.hdr
    // Morse-Smale edge information.
    FortranLinalg::DenseMatrix<int> edges;          //  Crystals_<level>.data.hdr
    // Cell layouts
    std::vector<FortranLinalg::DenseMatrix<Precision>> L;   // Point Positions

    // Number of smaples per cell for rendering.
    int nSamples;

    FortranLinalg::DenseVector<std::string> m_names;
    int minLevel;
    int maxLevel;
    

    // Extrema Layouts
    FortranLinalg::DenseVector<Precision> ef;   // Extrema Values
    FortranLinalg::DenseVector<Precision> ez;   // Extrema Values normalized [0,1]
    FortranLinalg::DenseVector<Precision> ew;   // Extrema Widths
    FortranLinalg::DenseMatrix<Precision> eL;   // Extrema layout 

    // filenames
    std::string m_path;

    // Cell Reconstruction
    std::vector<FortranLinalg::DenseMatrix<Precision>> R;      // mean
    std::vector<FortranLinalg::DenseMatrix<Precision>> Rvar;   // std-dev
    std::vector<FortranLinalg::DenseMatrix<Precision>> gradR;  // gradient
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
    Precision widthMin, widthMax;

    // color/width and transparent width values
    std::vector<FortranLinalg::DenseVector<Precision>> yc;   // mean-value
    std::vector<FortranLinalg::DenseVector<Precision>> z;    // normalized mean-value
    std::vector<FortranLinalg::DenseVector<Precision>> yw;   // width / radii (std)
    std::vector<FortranLinalg::DenseVector<Precision>> yd;   // density

    // ColorMapper for each cell
    ColorMapper<Precision> colormap;
    ColorMapper<Precision> dcolormap;

    HDVizLayout m_currentLayout;
    unsigned int m_currentLevel;
};
