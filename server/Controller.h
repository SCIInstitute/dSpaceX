#pragma once

#include "dspacex/Dataset.h"
#include "hdprocess/HDProcessResult.h"
#include "hdprocess/HDVizData.h"
#include "hdprocess/TopologyData.h"
#include "dspacex/Fieldtype.h"
#include "utils/DataExport.h"

#include <jsoncpp/json/json.h>
#include <map>
#include <functional>

namespace dspacex {

void setError(Json::Value &response, const std::string &str = "server error");

class Controller {
 public:
  Controller(const std::string &datapath_);
  void handleData(void *wsi, void *data);
  void handleText(void *wsi, const std::string &text);

 private:
  bool verifyFieldname(Fieldtype type, const std::string &name);

  void configureCommandHandlers();
  void configureAvailableDatasets(const std::string &rootPath);

  bool maybeLoadDataset(const Json::Value &request, Json::Value &response);
  bool loadDataset(int datasetId);
  bool verifyProcessDataParams(Fieldtype category, std::string fieldname, int knn, int curvepoints,
                               double sigma, double smoothing, bool addnoise, int depth,
                               bool normalize, Json::Value &response);
  bool processDataParamsChanged(Fieldtype category, std::string fieldname, int knn, int num_samples,
                                double sigma, double smoothing, bool add_noise,
                                int num_persistences, bool normalize);
  bool maybeProcessData(const Json::Value &request, Json::Value &response);
  bool processData(Fieldtype category, std::string fieldname, int knn,
                   int num_samples = 55, double sigma = 0.25, double smoothing = 15.0,
                   bool add_noise = true /* duplicate values risk erroroneous M-S */,
                   int num_persistences = -1 /* generates all persistence levels */,
                   bool normalize = true /* scale normalize field values */);
  int getPersistence(const Json::Value &request, Json::Value &response);

  // Command Handlers
  void fetchDatasetList(const Json::Value &request, Json::Value &response);
  void fetchDataset(const Json::Value &request, Json::Value &response);
  void fetchKNeighbors(const Json::Value &request, Json::Value &response);
  void fetchMorseSmalePersistenceLevel(const Json::Value &request, Json::Value &response);
  void fetchMorseSmaleCrystal(const Json::Value &request, Json::Value &response);
  void fetchMorseSmaleDecomposition(const Json::Value &request, Json::Value &response);
  void exportMorseSmaleDecomposition(const Json::Value &request, Json::Value &response);
  void fetchSingleEmbedding(const Json::Value &request, Json::Value &response);
  void fetchMorseSmaleRegression(const Json::Value &request, Json::Value &response);
  void fetchMorseSmaleExtrema(const Json::Value &request, Json::Value &response);
  void fetchCrystalPartition(const Json::Value &request, Json::Value &response);
  //  void fetchModelsList(const Json::Value &request, Json::Value &response);
  void fetchEmbeddingsList(const Json::Value &request, Json::Value &response);
  void fetchParameter(const Json::Value &request, Json::Value &response);
  void fetchQoi(const Json::Value &request, Json::Value &response);
  void fetchThumbnails(const Json::Value &request, Json::Value &response);
  void fetchNImagesForCrystal(const Json::Value &request, Json::Value &response);
  void regenOriginalImagesForCrystal(MSModelset &modelset, std::shared_ptr<Model> model, int persistence, int crystalId, Json::Value &response);
  void fetchCrystalOriginalSampleImages(const Json::Value &request, Json::Value &response);

  std::vector<ValueIndexPair> getSamples(Fieldtype category, const std::string &fieldname,
                                         unsigned persistenceLevel, unsigned crystalid, bool sort = true);

  int getAdjustedPersistenceLevelIdx(const unsigned desired_persistence, const std::shared_ptr<MSModelset>& mscomplex) const;

  typedef std::function<void(const Json::Value&, Json::Value&)> RequestHandler;
  std::map<std::string, RequestHandler> m_commandMap;
  std::vector<std::pair<std::string, std::string>> m_availableDatasets;
  FortranLinalg::DenseMatrix<Precision> m_currentDistanceMatrix;
  std::shared_ptr<HDVizData> m_currentVizData;
  std::unique_ptr<TopologyData> m_currentTopoData;
  std::string datapath;

  // current loaded dataset
  std::unique_ptr<dspacex::Dataset> m_currentDataset;
  int m_currentDatasetId{-1};

  // current processing state
  std::string m_currentField;
  Fieldtype m_currentCategory{Fieldtype::Unknown};
  int m_currentKNN{15};
  int m_currentNumCurvepoints{50};
  double m_currentSigma{0.25};
  double m_currentSmoothing{15.0};
  bool m_currentAddNoise{true};
  int m_currentPersistenceDepth{20};
  bool m_currentNormalize{true};

};

} // dspacex
