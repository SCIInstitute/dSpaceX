#pragma once

#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "Linalg.h"
#include "colormapper.h" 
#include "Precision.h"
#include <string>
#include <vector>

enum class HDVizLayout : char {
    ISOMAP  = 0,
    PCA = 1,
    PCA2 = 2,
};

class HDVizData {
  public:
    virtual ~HDVizData(){};
  
    // Morse-Smale edge information.
    virtual FortranLinalg::DenseMatrix<int>& getEdges(int persistenceLevel) = 0;
    virtual FortranLinalg::DenseVector<Precision>& getPersistence() = 0;
    virtual FortranLinalg::DenseVector<std::string>& getNames() = 0;
    virtual std::vector<FortranLinalg::DenseMatrix<Precision>>& getLayout(
        HDVizLayout layout, int persistenceLevel) = 0;

    // Extrema Layouts
    virtual FortranLinalg::DenseVector<Precision>& getExtremaValues(int persistenceLevel) = 0;
    virtual FortranLinalg::DenseVector<Precision>& getExtremaNormalized(int persistenceLevel) = 0;
    virtual FortranLinalg::DenseVector<Precision>& getExtremaWidths(int persistenceLevel) = 0;
    virtual FortranLinalg::DenseVector<Precision>& getExtremaWidthsScaled(int persistenceLevel) = 0;
    virtual FortranLinalg::DenseMatrix<Precision>& getExtremaLayout(
        HDVizLayout layout, int persistenceLevel) = 0;
    
    // Number of samples used for layouts.
    virtual int getNumberOfSamples() = 0;
           
    // Cell reconstruction
    virtual std::vector<FortranLinalg::DenseMatrix<Precision>>& getReconstruction(int persistenceLevel) = 0;
    virtual std::vector<FortranLinalg::DenseMatrix<Precision>>& getVariance(int persistenceLevel) = 0;   // TODO: Maybe rename.
    virtual std::vector<FortranLinalg::DenseMatrix<Precision>>& getGradient(int persistenceLevel) = 0;
    virtual FortranLinalg::DenseVector<Precision>& getRMin() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getRMax() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getRsMin(int persistenceLevel) = 0;
    virtual FortranLinalg::DenseVector<Precision>& getRsMax(int persistenceLevel) = 0;  
    virtual FortranLinalg::DenseVector<Precision>& getGradientMin(int persistenceLevel) = 0;
    virtual FortranLinalg::DenseVector<Precision>& getGradientMax(int persistenceLevel) = 0;
    
    virtual Precision getExtremaMinValue(int persistenceLevel) = 0;
    virtual Precision getExtremaMaxValue(int persistenceLevel) = 0;
    virtual Precision getWidthMin(int persistenceLevel) = 0;
    virtual Precision getWidthMax(int persistenceLevel) = 0;

    // color/width and transparent width values
    virtual std::vector<FortranLinalg::DenseVector<Precision>>& getMean(
        int persistenceLevel) = 0;
    virtual std::vector<FortranLinalg::DenseVector<Precision>>& getMeanNormalized(
        int persistenceLevel) = 0;
    virtual std::vector<FortranLinalg::DenseVector<Precision>>& getWidth(
        int persistenceLevel) = 0; 
    virtual std::vector<FortranLinalg::DenseVector<Precision>>& getDensity(
        int persistenceLevel) = 0;

    // ColorMapper for each cell
    virtual ColorMapper<Precision>& getColorMap(int persistenceLevel) = 0;
    virtual ColorMapper<Precision>& getDColorMap(int persistenceLevel) = 0;

    virtual int getMinPersistenceLevel() = 0;
    virtual int getMaxPersistenceLevel() = 0;
};
