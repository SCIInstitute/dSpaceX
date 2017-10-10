#pragma once

#include "HDVizData.h"

class HDVizState {
 public:
  HDVizState(HDVizData *data) {
    selectedCell  = 0;
    selectedPoint = 0;
    currentLevel = data->getMaxPersistenceLevel();
    HDVizLayout currentLayout = HDVizLayout::ISOMAP;
  }

  HDVizLayout currentLayout;
  int currentLevel;  
  int selectedCell;
  int selectedPoint;
};