#pragma once

#include "dspacex/Dataset.h"
#include "hdprocess/HDProcessResult.h"
#include "hdprocess/HDVizData.h"
#include "hdprocess/TopologyData.h"
#include "dspacex/Fieldtype.h"

#include <jsoncpp/json/json.h>
#include <map>
#include <functional>


class Controller {
 public:
  Controller(const std::string &datapath_);
  void handleData(void *wsi, void *data);
  void handleText(void *wsi, const std::string &text);

 private:
  void sendError(Json::Value &response, std::string str = "server error");
  bool verifyFieldname(Fieldtype type, const std::string &name);

  void configureCommandHandlers();
  void configureAvailableDatasets(const std::string &rootPath);

  void maybeLoadDataset(int datasetId);
  void maybeProcessData(int k, Fieldtype category, std::string fieldname);

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
  void fetchAllForLatentSpaceUsingSharedGP(const Json::Value &request, Json::Value &response);
  void fetchImageForLatentSpaceCoord_Shapeodds(const Json::Value &request, Json::Value &response);
  void fetchNImagesForCrystal_Shapeodds(const Json::Value &request, Json::Value &response);
  void fetchAllImagesForCrystal_Shapeodds(const Json::Value &request, Json::Value &response);
  void fetchCrystalOriginalSampleImages(const Json::Value &request, Json::Value &response);

  const Eigen::Map<Eigen::VectorXd> getFieldvalues(Fieldtype type, const std::string &name);

  // todo: user shouldn't need this: a plvl is a plvl, so bury the details
  int getPersistenceLevelIdx(const unsigned desired_persistence, const dspacex::MSComplex &mscomplex) const;

  typedef std::function<void(const Json::Value&, Json::Value&)> RequestHandler;
  std::map<std::string, RequestHandler> m_commandMap;
  std::vector<std::pair<std::string, std::string>> m_availableDatasets;
  std::unique_ptr<Dataset> m_currentDataset;
  int m_currentDatasetId = -1;
  std::string m_currentField;
  FortranLinalg::DenseMatrix<Precision> m_currentDistanceMatrix;
  int m_currentK = -1; // num nearest neighbors to consider when generating M-S complex for a dataset
  HDVizData *m_currentVizData = nullptr;
  TopologyData *m_currentTopoData = nullptr;
  std::string datapath;
};
