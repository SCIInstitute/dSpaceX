#pragma once

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

class HDVizData {
  public:
    virtual ~HDVizData(){};
    virtual Precision getSelectedCoordinate(int selectedCell, int selectedPoint, int index) = 0;
    virtual Precision getSelectedVariance(int selectedCell, int selectedPoint, int index) = 0;

    // Morse-Smale edge information.
    virtual FortranLinalg::DenseMatrix<int>& getEdges() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getPersistence() = 0;
    virtual FortranLinalg::DenseVector<std::string>& getNames() = 0;
    virtual FortranLinalg::DenseMatrix<Precision>* getLayout() = 0;

    // Extrema Layouts
    virtual FortranLinalg::DenseVector<Precision>& getExtremaValues() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getExtremaNormalized() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getExtremaWidths() = 0;
    virtual FortranLinalg::DenseMatrix<Precision>& getExtremaLayout() = 0;
    
    // Number of samples used for rendering.
    virtual int getNumberOfSamples() = 0;
    
    // Set which persistence level and layout to use.
    virtual void loadData(int level) = 0;
    virtual void setLayout(HDVizLayout layout, int level) = 0;
       
    // Cell reconstruction
    virtual FortranLinalg::DenseMatrix<Precision>* getReconstruction(/* int persistenceLevel */) = 0;
    virtual FortranLinalg::DenseMatrix<Precision>* getVariance(/* int persistenceLevel */) = 0;
    virtual FortranLinalg::DenseMatrix<Precision>* getGradient(/* int persistenceLevel */) = 0;
    virtual FortranLinalg::DenseVector<Precision>& getRMin() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getRMax() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getRsMin() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getRsMax() = 0;  
    virtual FortranLinalg::DenseVector<Precision>& getGradientMin() = 0;
    virtual FortranLinalg::DenseVector<Precision>& getGradientMax() = 0;
    
    virtual Precision getExtremaMinValue() = 0;
    virtual Precision getExtremaMaxValue() = 0;
    virtual Precision getZMin() = 0;
    virtual Precision getZMax() = 0;

    // color/width and transparent width values
    virtual FortranLinalg::DenseVector<Precision>* getValueColor(/* int persistenceLevel */) = 0;
    virtual FortranLinalg::DenseVector<Precision>* getZ() = 0;    // What is z??
    virtual FortranLinalg::DenseVector<Precision>* getWidth() = 0; 
    virtual FortranLinalg::DenseVector<Precision>* getDensity() = 0;

    // ColorMapper for each cell
    virtual ColorMapper<Precision>& getColorMap() = 0;
    virtual ColorMapper<Precision>& getDColorMap() = 0;

    virtual int getMinPersistenceLevel() = 0;
    virtual int getMaxPersistenceLevel() = 0;
};
