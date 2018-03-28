#pragma once

#include "Dataset.h"
#include "HDProcessResult.h"
#include "HDVizData.h"
#include "TopologyData.h"
#include <jsoncpp/json.h>


class Controller {
 public:
  void configureAvailableDatasets();
  void handleData(void *wsi, void *data);
  void handleText(void *wsi, const std::string &text);

 private:
  void maybeLoadDataset(int datasetId);
  void maybeProcessData(int k);

  Dataset* loadConcreteDataset();
  Dataset* loadCrimesDataset();
  Dataset* loadGaussianDataset();
  Dataset* loadColoradoDataset();

  // Command Handlers
  void fetchDatasetList(void *wsi, int messageId, const Json::Value &request);
  void fetchDataset(void *wsi, int messageId, const Json::Value &request);
  void fetchKNeighbors(void *wsi, int messageId, const Json::Value &request);
  void fetchMorseSmalePersistence(void *wsi, int messageId, const Json::Value &request);
  void fetchMorseSmalePersistenceLevel(void *wsi, int messageId, const Json::Value &request);
  void fetchMorseSmaleCrystal(void *wsi, int messageId, const Json::Value &request);
  void fetchMorseSmaleDecomposition(void *wsi, int messageId, const Json::Value &request);

  FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x);

  std::vector<std::pair<std::string, std::function<Dataset*()>>> m_availableDatasets;
  Dataset *m_currentDataset = nullptr;
  int m_currentDatasetId = -1;
  int m_currentK = -1;
  HDProcessResult *m_currentProcessResult = nullptr;
  HDVizData *m_currentVizData = nullptr;
  TopologyData *m_currentTopoData = nullptr;
};