#pragma once

#include "Dataset.h"
#include "hdprocess/HDProcessResult.h"
#include "hdprocess/HDVizData.h"
#include "hdprocess/TopologyData.h"
#include <jsoncpp/json.h>

#include <map>


class Controller {
 public:
  Controller();  
  void handleData(void *wsi, void *data);
  void handleText(void *wsi, const std::string &text);

 private:
  void configureCommandHandlers();
  void configureAvailableDatasets();
  
  void maybeLoadDataset(int datasetId);
  void maybeProcessData(int k);

  // Command Handlers
  void fetchDatasetList(void *wsi, int messageId, const Json::Value &request);
  void fetchDataset(void *wsi, int messageId, const Json::Value &request);
  void fetchKNeighbors(void *wsi, int messageId, const Json::Value &request);
  void fetchMorseSmalePersistence(void *wsi, int messageId, const Json::Value &request);
  void fetchMorseSmalePersistenceLevel(void *wsi, int messageId, const Json::Value &request);
  void fetchMorseSmaleCrystal(void *wsi, int messageId, const Json::Value &request);
  void fetchMorseSmaleDecomposition(void *wsi, int messageId, const Json::Value &request);
  void fetchLayoutForPersistenceLevel(void *wsi, int messageId, const Json::Value &request);
  void fetchQoi(void *wsi, int messageId, const Json::Value &request);

  std::map<std::string, std::function<void(void*, int, const Json::Value&)>> m_commandMap;

  std::vector<std::pair<std::string, std::string>> m_availableDatasets;
  Dataset *m_currentDataset = nullptr;
  int m_currentDatasetId = -1;
  FortranLinalg::DenseMatrix<Precision> m_currentDistanceMatrix;
  int m_currentK = -1;
  HDProcessResult *m_currentProcessResult = nullptr;
  HDVizData *m_currentVizData = nullptr;
  TopologyData *m_currentTopoData = nullptr;
};