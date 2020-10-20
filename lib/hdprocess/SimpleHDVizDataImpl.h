#pragma once

#include "colormapper.h" 
#include "flinalg/LinalgIO.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/Linalg.h"
#include "HDVizData.h"
#include "HDProcessResult.h"
#include "dataset/Precision.h"

#include <string>

class SimpleHDVizDataImpl : public HDVizData {
  public:
    SimpleHDVizDataImpl(std::shared_ptr<HDProcessResult> result);

    // Morse-Smale edge information.
    FortranLinalg::DenseMatrix<Precision>& getX();
    FortranLinalg::DenseVector<Precision>& getY();
    FortranLinalg::DenseMatrix<int>& getNearestNeighbors();
    FortranLinalg::DenseMatrix<int>& getCrystals(int persistenceLevel);    
    FortranLinalg::DenseVector<int>& getCrystalPartitions(int persistenceLevel);
    std::vector<FortranLinalg::DenseVector<int>> getAllCrystalPartitions();
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
    std::vector<FortranLinalg::DenseVector<Precision>>& getWidthScaled(
        int persistenceLevel);
    std::vector<FortranLinalg::DenseVector<Precision>>& getDensity(
        int persistenceLevel);

    // ColorMapper for each cell
    ColorMapper<Precision>& getColorMap(int persistenceLevel);
    ColorMapper<Precision>& getDColorMap(int persistenceLevel);

    int getMinPersistenceLevel() const override; 
    int getMaxPersistenceLevel() const override;
        
  private:
    std::shared_ptr<HDProcessResult> m_data;
    
    int m_numberOfSamples;

    // Computed visualization helper data
    std::vector<FortranLinalg::DenseVector<Precision>> extremaNormalized;
    std::vector<FortranLinalg::DenseVector<Precision>> extremaWidthScaled;
    FortranLinalg::DenseVector<Precision> Rmin;               // TODO: geom min, rename?
    FortranLinalg::DenseVector<Precision> Rmax;               // TODO: geom max, rename?
    std::vector<FortranLinalg::DenseVector<Precision>> Rsmin; // Reconstruction mins
    std::vector<FortranLinalg::DenseVector<Precision>> Rsmax; // Reconstruction maxs
    std::vector<FortranLinalg::DenseVector<Precision>> gRmin; // Reconstruction Gradient mins
    std::vector<FortranLinalg::DenseVector<Precision>> gRmax; // Reconstruction Gradient maxs
    std::vector<Precision> efmin;                             // Extrema Value Mins
    std::vector<Precision> efmax;                             // Extrema Value Maxs
    std::vector<Precision> widthMin;                          // Width Min
    std::vector<Precision> widthMax;                          // Width Max
    std::vector<std::vector<FortranLinalg::DenseVector<Precision>>> meanNormalized;
    std::vector<std::vector<FortranLinalg::DenseVector<Precision>>> widthScaled;

    // ColorMappers
    std::vector<ColorMapper<Precision>> colormap;
    std::vector<ColorMapper<Precision>> dcolormap;

    void computeScaledLayouts();
    std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> scaledIsoLayout; 
    std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> scaledPCALayout;
    std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> scaledPCA2Layout;
    std::vector<FortranLinalg::DenseMatrix<Precision>> scaledIsoExtremaLayout;
    std::vector<FortranLinalg::DenseMatrix<Precision>> scaledPCAExtremaLayout;
    std::vector<FortranLinalg::DenseMatrix<Precision>> scaledPCA2ExtremaLayout;


};

