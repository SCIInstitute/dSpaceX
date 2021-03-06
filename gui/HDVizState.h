#pragma once

#include "flinalg/LinalgIO.h"
#include "hdprocess/HDVizData.h"
#include "dataset/Precision.h"

class HDVizState {
 public:
  HDVizState(HDVizData *data) {
    selectedCell  = 0;
    selectedPoint = 0;
    currentLevel = data->getMaxPersistenceLevel();
    currentLayout = HDVizLayout::ISOMAP;
  }

  HDVizState(HDVizData *data, FortranLinalg::DenseMatrix<Precision> dists) {    
    selectedCell  = 0;
    selectedPoint = 0;
    currentLevel = data->getMaxPersistenceLevel();
    currentLayout = HDVizLayout::ISOMAP;
    distances = dists;
  }

  FortranLinalg::DenseMatrix<Precision> distances;
  HDVizLayout currentLayout;
  int currentLevel;  
  int selectedCell;
  int selectedPoint;
};
