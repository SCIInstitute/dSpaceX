#pragma once

#include "HDVizData.h"

class HDVizState {
 public:
  HDVizState(HDVizData *data) {
    selectedCell  = 0;
    selectedPoint = 0;
    currentLevel = data->getMaxPersistenceLevel();
  }

  int currentLevel;
  int selectedCell;
  int selectedPoint;
};