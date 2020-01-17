#pragma once

#include "Dataset.h"
#include "hdprocess/HDProcessResult.h"
#include "hdprocess/HDVizData.h"
#include "hdprocess/TopologyData.h"
#include <jsoncpp/json/json.h>

#include <map>
#include <functional>


class Controller {
 public:
  Controller(const std::string &datapath_);
  void handleData(void *wsi, void *data);
  void handleText(void *wsi, const std::string &text);

 private:
  void configureCommandHandlers();
  void configureAvailableDatasets(const std::string &rootPath);

  void maybeLoadDataset(int datasetId);
  void maybeProcessData(int k);

  // Command Handlers
  void fetchDatasetList(const Json::Value &request, Json::Value &response);
  void fetchDataset(const Json::Value &request, Json::Value &response);
  void fetchKNeighbors(const Json::Value &request, Json::Value &response);
  void fetchMorseSmalePersistence(const Json::Value &request, Json::Value &response);
  void fetchMorseSmalePersistenceLevel(const Json::Value &request, Json::Value &response);
  void fetchMorseSmaleCrystal(const Json::Value &request, Json::Value &response);
  void fetchMorseSmaleDecomposition(const Json::Value &request, Json::Value &response);
  void fetchSingleEmbedding(const Json::Value &request, Json::Value &response);
  void fetchMorseSmaleRegression(const Json::Value &request, Json::Value &response);
  void fetchMorseSmaleExtrema(const Json::Value &request, Json::Value &response);
  void fetchCrystalPartition(const Json::Value &request, Json::Value &response);
  void fetchEmbeddingsList(const Json::Value &request, Json::Value &response);
  void fetchParameter(const Json::Value &request, Json::Value &response);
  void fetchQoi(const Json::Value &request, Json::Value &response);
  void fetchThumbnails(const Json::Value &request, Json::Value &response);

  // SharedGP
  void fetchAllForLatentSpaceUsingSharedGP(const Json::Value &request, Json::Value &response);

  // ShapeOdds
  void fetchImageForLatentSpaceCoord_Shapeodds(const Json::Value &request, Json::Value &response);
  void fetchNImagesForCrystal_Shapeodds(const Json::Value &request, Json::Value &response);
  void fetchAllImagesForCrystal_Shapeodds(const Json::Value &request, Json::Value &response);

  typedef std::function<void(const Json::Value&, Json::Value&)> RequestHandler;
  std::map<std::string, RequestHandler> m_commandMap;

  std::vector<std::pair<std::string, std::string>> m_availableDatasets;
  Dataset *m_currentDataset = nullptr;
  int m_currentDatasetId = -1;
  FortranLinalg::DenseMatrix<Precision> m_currentDistanceMatrix;
  int m_currentK = -1;
  HDProcessResult *m_currentProcessResult = nullptr;
  HDVizData *m_currentVizData = nullptr;
  TopologyData *m_currentTopoData = nullptr;
  std::string datapath;
};
