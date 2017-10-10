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
    Precision getSelectedCoordinate(int selectedCell, int selectedPoint, int index);
    Precision getSelectedVariance(int selectedCell, int selectedPoint, int index);

    // Morse-Smale edge information.
    FortranLinalg::DenseMatrix<int>& getEdges(int persistenceLevel);    
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
    FortranLinalg::DenseVector<Precision>* getValueColor(/* int persistenceLevel */);
    FortranLinalg::DenseVector<Precision>* getZ();    // What is z??
    FortranLinalg::DenseVector<Precision>* getWidth(); 
    FortranLinalg::DenseVector<Precision>* getDensity(); 

    // ColorMapper for each cell
    ColorMapper<Precision>& getColorMap();
    ColorMapper<Precision>& getDColorMap();

    int getMinPersistenceLevel(); 
    int getMaxPersistenceLevel();
        
  private:
    HDProcessResult *m_data;
};

