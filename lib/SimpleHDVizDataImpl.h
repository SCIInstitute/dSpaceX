#pragma once

#include "HDVizData.h"
#include "HDProcessResult.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "Linalg.h"
#include "colormapper.h" 
#include "Precision.h"
#include <string>

class SimpleHDVizDataImpl : public HDVizData {
  public:
    SimpleHDVizDataImpl(HDProcessResult *result);
    Precision getSelectedCoordinate(
        int persistenceLevel, int selectedCell, int selectedPoint, int index);
    Precision getSelectedVariance(
        int persistenceLevel, int selectedCell, int selectedPoint, int index);

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
    FortranLinalg::DenseMatrix<Precision>& getExtremaLayout(
        HDVizLayout layout, int persistenceLevel);
    
    // Number of samples used for rendering.
    int getNumberOfSamples();   
    
    // Set which persistence level and layout to use.
    void loadData(int level);
    void setLayout(HDVizLayout layout, int level);
       
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
    Precision getZMin(int persistenceLevel);
    Precision getZMax(int persistenceLevel);

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
    HDProcessResult *m_data;
    
    // Computed visualization helper data
    std::vector<FortranLinalg::DenseVector<Precision>> extremaNormalized;
    FortranLinalg::DenseVector<Precision> Rmin;               // TODO: geom min, rename?
    FortranLinalg::DenseVector<Precision> Rmax;               // TODO: geom max, rename?
    std::vector<FortranLinalg::DenseVector<Precision>> Rsmin; // Reconstruction mins
    std::vector<FortranLinalg::DenseVector<Precision>> Rsmax; // Reconstruction maxs
    std::vector<FortranLinalg::DenseVector<Precision>> gRmin; // Reconstruction Gradient mins
    std::vector<FortranLinalg::DenseVector<Precision>> gRmax; // Reconstruction Gradient maxs
    std::vector<Precision> efmin;                             // Extrema Value Mins
    std::vector<Precision> efmax;                             // Extrema Value Maxs
    std::vector<Precision> zmin;                              // Normalized Extrema Value Mins
    std::vector<Precision> zmax;                              // Normalized Extrema Value Maxs
    std::vector<std::vector<FortranLinalg::DenseVector<Precision>>> meanNormalized;

    // ColorMappers
    std::vector<ColorMapper<Precision>> colormap;
    std::vector<ColorMapper<Precision>> dcolormap;
};

