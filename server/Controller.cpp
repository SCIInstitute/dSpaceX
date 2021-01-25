#include <base64/base64.h>
#include <boost/filesystem.hpp>
#include "Controller.h"
#include "dataset/DatasetLoader.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/DenseVector.h"
#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "hdprocess/HDGenericProcessor.h"
#include "hdprocess/LegacyTopologyDataImpl.h"
#include "hdprocess/SimpleHDVizDataImpl.h"
#include "hdprocess/TopologyData.h"
#include <jsoncpp/json/json.h>
#include "dataset/Precision.h"
#include "dataset/ValueIndexPair.h"
#include "serverlib/wst.h"
#include "utils/DenseVectorSample.h"
#include "utils/loaders.h"
#include "utils/utils.h"
#include "pmodels/Model.h"
#include "utils/Data.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <pybind11/embed.h>
#include <pybind11/eigen.h>
namespace py = pybind11;

// This clock corresponds to CLOCK_MONOTONIC at the syscall level.
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using namespace std::literals::chrono_literals;

namespace dspacex {

// Simplify namespace access to _1, _2 for std::bind parameter binding.
using namespace std::placeholders;

// Maximum number of directories from root path to seek config.yaml files
const int MAX_DATASET_DEPTH = 6;


Controller::Controller(const std::string &datapath_) : datapath(datapath_) {
  configureCommandHandlers();
  configureAvailableDatasets(datapath);
}

/* 
 * Sets the given error message in the Json response.
 */
void setError(Json::Value &response, const std::string &str)
{
  response["error"] = true;
  response["error_msg"] = str;
}

/**
 * Construct map of command names to command handlers.
 */
void Controller::configureCommandHandlers() {
  m_commandMap.insert({"fetchDatasetList", std::bind(&Controller::fetchDatasetList, this, _1, _2)});
  m_commandMap.insert({"fetchDataset", std::bind(&Controller::fetchDataset, this, _1, _2)});
  m_commandMap.insert({"fetchKNeighbors", std::bind(&Controller::fetchKNeighbors, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleDecomposition", std::bind(&Controller::fetchMorseSmaleDecomposition, this, _1, _2)});
  m_commandMap.insert({"exportMorseSmaleDecomposition", std::bind(&Controller::exportMorseSmaleDecomposition, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmalePersistenceLevel", std::bind(&Controller::fetchMorseSmalePersistenceLevel, this, _1, _2)});
  m_commandMap.insert({"fetchEmbeddingsList", std::bind(&Controller::fetchEmbeddingsList, this, _1, _2)});
  m_commandMap.insert({"fetchSingleEmbedding", std::bind(&Controller::fetchSingleEmbedding, this, _1, _2)});
  m_commandMap.insert({"fetchNodeColors", std::bind(&Controller::fetchNodeColors, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleRegression", std::bind(&Controller::fetchMorseSmaleRegression, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleExtrema", std::bind(&Controller::fetchMorseSmaleExtrema, this, _1, _2)});
  m_commandMap.insert({"fetchCrystal", std::bind(&Controller::fetchCrystal, this, _1, _2)});
  m_commandMap.insert({"fetchParameter", std::bind(&Controller::fetchParameter, this, _1, _2)});
  m_commandMap.insert({"fetchQoi", std::bind(&Controller::fetchQoi, this, _1, _2)});
  m_commandMap.insert({"fetchThumbnails", std::bind(&Controller::fetchThumbnails, this, _1, _2)});
  m_commandMap.insert({"fetchNImagesForCrystal", std::bind(&Controller::fetchNImagesForCrystal, this, _1, _2)});
  m_commandMap.insert({"fetchCrystalOriginalSampleImages", std::bind(&Controller::fetchCrystalOriginalSampleImages, this, _1, _2)});
}

/**
 * Construct list of available datasets.
 */

void Controller::configureAvailableDatasets(const std::string &path) {
  boost::filesystem::path rootPath(path);
  if (!boost::filesystem::is_directory(rootPath)) {
    std::cout << "Data path " << rootPath.string() << " is not a valid directory." << std::endl;
  }

  auto currentPath = boost::filesystem::current_path();
  std::cout << "Running server from current path: " << currentPath.string() << std::endl;

  try {
    auto canonicalRootPath = boost::filesystem::canonical(rootPath, currentPath);
    std::cout << "Configuring data from root path: " << canonicalRootPath.string() << std::endl;
  } catch (const boost::filesystem::filesystem_error &e) {
    std::cout << e.code().message() << std::endl;
    return;
  }

  std::vector<boost::filesystem::path> configPaths;
  boost::filesystem::recursive_directory_iterator iter{rootPath};
  while (iter != boost::filesystem::recursive_directory_iterator{}) {
    if (iter->path().filename() != "config.yaml") {
      iter++;
      continue;
    }
    if (iter.level() == MAX_DATASET_DEPTH) {
      iter.pop();
    }
    configPaths.push_back(*iter);
    iter++;
  }

  std::cout << "Found the following datasets:" << std::endl;
  for (auto path : configPaths) {
    try {
      std::string name = DatasetLoader::getDatasetName(path.string());
      std::cout << "  " << path.string() << " --> " << name << std::endl;
      m_availableDatasets.push_back({name, path.string()});
    } catch (const std::exception &error) {
      std::cout << "  " << path.string() << " --> " << "[ FAILED TO LOAD ]" << std::endl;
    }
  }
}

// ensure the fieldname exists in this Controller's dataset
bool Controller::verifyFieldname(Fieldtype type, const std::string &name) const
{
  if (type == Fieldtype::QoI) {
    auto qois = m_currentDataset->getQoiNames();
    return std::find(std::begin(qois), std::end(qois), name) != std::end(qois);
  }
  else if (type == Fieldtype::DesignParameter) {
    auto parameters = m_currentDataset->getParameterNames();
    return std::find(std::begin(parameters), std::end(parameters), name) != std::end(parameters);
  }
  return false;
}    

void Controller::handleData(void *wsi, void *data) {
  // TODO:  Implement
}

void Controller::handleText(void *wsi, const std::string &text) {
  Json::Reader reader;
  Json::Value request;

  try {
    reader.parse(text, request);
    int messageId = request["id"].asInt();
    std::string commandName = request["name"].asString();
    //std::cout << "[" << messageId << "] " << commandName << "..." << std::endl;

    Json::Value response(Json::objectValue);
    response["id"] = messageId;

    auto command = m_commandMap[commandName];

    if (command) {
      time_point<Clock> start = Clock::now();
      command(request, response);
      time_point<Clock> end = Clock::now();
      milliseconds diff = duration_cast<milliseconds>(end - start);
      //std::cout << "[" << messageId << "] " << commandName << " completed in "
      //          << static_cast<float>(diff.count())/1000.0f << "s" << std::endl;
    } else {
      std::cout << "Error: Unrecognized Command: " << commandName << std::endl;
    }

    Json::StyledWriter writer;
    std::string text = writer.write(response);
    wst_sendText(wsi, const_cast<char *>(text.c_str()));
  } catch (const std::exception &e) {
    std::cerr << "Command Execution Error: " << e.what() << std::endl;
  }
}

/**
 * Handle the command to fetch list of available datasets.
 */
void Controller::fetchDatasetList(const Json::Value &request, Json::Value &response) {
  response["datasets"] = Json::Value(Json::arrayValue);

  for (size_t i = 0; i < m_availableDatasets.size(); i++) {
    Json::Value object = Json::Value(Json::objectValue);
    object["id"] = static_cast<int>(i);
    object["name"] = m_availableDatasets[i].first;
    response["datasets"].append(object);
  }
}

/**
 * Handle the command to load and fetch details of a dataset.
 */
void Controller::fetchDataset(const Json::Value &request, Json::Value &response) { 
 if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  response["datasetId"] = m_currentDatasetId;
  response["name"] = m_currentDataset->getName();
  response["numberOfSamples"] = m_currentDataset->numberOfSamples();

  // all parameters and qois and their models
  std::vector<std::string> fields;

  // the design parameter and qoi names (todo: consolidate these on the server)
  response["parameterNames"] = Json::Value(Json::arrayValue);
  for (std::string name : m_currentDataset->getParameterNames()) {
    response["parameterNames"].append(name);
    fields.push_back(name);
  }
  response["qoiNames"] = Json::Value(Json::arrayValue);
  for (std::string name : m_currentDataset->getQoiNames()) {
    response["qoiNames"].append(name);
    fields.push_back(name);
  }

  // the set of distance metrics, their sets of fields, and fields' sets of models
  response["distanceMetrics"] = Json::Value(Json::arrayValue);
  for (auto metricname : m_currentDataset->getDistanceMetricNames()) {
    Json::Value metricObject(Json::objectValue);
    metricObject["name"] = metricname;

    // assemble the fields and models for each field
    metricObject["fields"] = Json::Value(Json::arrayValue);
    for (auto fieldname : fields) {
      Json::Value fieldObject(Json::objectValue);
      fieldObject["name"] = fieldname;
      //fieldObject["type"] = <todo>:

      Json::Value modelNames = Json::Value(Json::arrayValue);
      for (auto name : m_currentDataset->getModelNames(metricname, fieldname)) {
        modelNames.append(name);
      }
      fieldObject["models"] = modelNames;

      metricObject["fields"].append(fieldObject);
    }

    response["distanceMetrics"].append(metricObject);
  }
}

/**
 * Handle the command to fetch the k-nearest-neighbor graph of a dataset.
 */
void Controller::fetchKNeighbors(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  // k is the num nearest neighbors to consider when generating M-S complex for a dataset
  int k = request["k"].asInt();
  if (k < 0) return setError(response, "invalid knn");

  auto metric = request["metric"].asString();

  // TODO: cache results of this search if it takes too long to repeat
  int n = m_currentDataset->getDistanceMatrix(metric).N();
  auto KNN = FortranLinalg::DenseMatrix<int>(k, n);
  auto KNND = FortranLinalg::DenseMatrix<Precision>(k, n);
  Distance<Precision>::findKNN(m_currentDataset->getDistanceMatrix(metric), KNN, KNND);

  response["datasetId"] = m_currentDatasetId;
  response["k"] = k;
  response["graph"] = Json::Value(Json::arrayValue);
  for (unsigned i = 0; i < KNN.M(); i++) {
    Json::Value row = Json::Value(Json::arrayValue);
    response["graph"].append(row);
    for (unsigned int j = 0; j < KNN.N(); j++) {
      response["graph"][i].append(KNN(i, j));
    }
  }
}

/**
 * Handle the command to fetch the morse smale decomposition of a dataset.
 * Returns min/max persistences and num crystals for each one.
 */
void Controller::fetchMorseSmaleDecomposition(const Json::Value &request, Json::Value &response)
{
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  //std::cout << "Controller::fetchMorseSmaleDecomposition persistence range: [" << minLevel << "," << maxLevel << "]\n";

  response["datasetId"] = m_currentDatasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["minPersistenceLevel"] = minLevel;
  response["maxPersistenceLevel"] = maxLevel;
  response["complexSizes"] = Json::Value(Json::arrayValue);
  for (int level = minLevel; level <= maxLevel; level++) {
    std::shared_ptr<MorseSmaleComplex> complex = m_currentTopoData->getComplex(level);
    int size = complex->getCrystals().size();
    response["complexSizes"].append(size);
    //std::cout << "Controller::fetchMorseSmaleDecomposition persistence " << level << " num crystals: " << size << std::endl;
  }
}

/**
 * Write the current morse smale decomposition of a dataset.
 */
void Controller::exportMorseSmaleDecomposition(const Json::Value &request, Json::Value &response)
{
  response["field"] = m_currentField;
  response["category"] = m_currentCategory.asString();
  response["neighborhoodSize"] = m_currentKNN;
  response["datasigma"] = m_currentSmoothDataSigma;
  response["curvesigma"] = m_currentSmoothCurveSigma;
  response["crystalCurvepoints"] = m_currentNumCurvepoints;
  response["depth"] = m_currentPersistenceDepth;
  response["noise"] = m_currentAddNoise;
  response["normalize"] = m_currentNormalize;
  response["minPersistence"] = m_currentVizData->getMinPersistenceLevel();

  // array of sample ids and extrema of each crystal in each persistence
  response["persistences"] = Json::Value(Json::arrayValue);
  auto crystals = m_currentVizData->getAllCrystals();
  auto extrema = m_currentVizData->getAllExtrema();
  for (auto p = m_currentVizData->getMinPersistenceLevel(); p < m_currentVizData->getPersistence().N(); ++p) {
    Json::Value persistence = Json::Value(Json::objectValue);
    persistence["persistenceLevel"] = p;
    persistence["crystals"] = Json::Value(Json::arrayValue);
    persistence["extrema"] = Json::Value(Json::arrayValue);
    for (auto c = 0; c < crystals[p].size(); ++c) {
      Json::Value crystal = Json::Value(Json::objectValue);
      crystal["ids"] = Json::Value(Json::arrayValue);
      for (auto id: crystals[p][c]) {
        crystal["ids"].append(id);
      }
      
      Json::Value extremanode = Json::Value(Json::objectValue);
      extremanode["max"] = extrema[p][c].first;
      extremanode["min"] = extrema[p][c].second;
      crystal["extrema"] = extremanode;

      persistence["crystals"].append(crystal);
    }
    response["persistence"].append(persistence);
  }
}

/**
 * Gets the persistence level in this request, setting error in response and returning -1 if invalid.
 */
int Controller::getPersistence(const Json::Value &request, Json::Value &response) {
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistence = request["persistence"].asInt();
  if (persistence < minLevel || persistence > maxLevel) {
    setError(response, "invalid persistence level");
    return -1;
  }
  return persistence;
}

/**
 * Handle the command to fetch the crystal complex composing a morse smale persistence level.
 */
void Controller::fetchMorseSmalePersistenceLevel(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  std::shared_ptr<MorseSmaleComplex> complex = m_currentTopoData->getComplex(persistence);

  response["datasetId"] = m_currentDatasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["k"] = m_currentKNN;
  response["persistence"] = persistence;
  response["complex"] = Json::Value(Json::objectValue);
  response["complex"]["crystals"] = Json::Value(Json::arrayValue);

  for (unsigned int c = 0; c < complex->getCrystals().size(); c++) {
    std::shared_ptr<Crystal> crystal = complex->getCrystals()[c];
    Json::Value crystalObject(Json::objectValue);
    crystalObject["minIndex"] = crystal->getMinSample();
    crystalObject["maxIndex"] = crystal->getMaxSample();
    crystalObject["numberOfSamples"] = static_cast<int>(crystal->getAllSamples().size());
    response["complex"]["crystals"].append(crystalObject);
  }

  // TODO: Add crystal adjacency information
}

/**
 * This fetches the 2d embedding layout for the nodes for a field in a given distance metric.
 * It is composed of node positions and node connectivity.
 * @param request
 * @param response
 */
void Controller::fetchSingleEmbedding(const Json::Value &request, Json::Value &response) {
  int embeddingId = request["embeddingId"].asInt();
  if (embeddingId < 0) return setError(response, "invalid embeddingId");

  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  auto metric = request["metric"].asString();

  if (m_currentDataset->numberOfEmbeddings(metric) > 0) {
    auto embedding = m_currentDataset->getEmbeddingMatrix(metric, embeddingId);
    auto name = m_currentDataset->getEmbeddingNames(metric)[embeddingId];

    // TODO: Factor out a normalizing routine. [see normalize in utils.h]
    float minX = embedding(0, 0);
    float maxX = embedding(0, 0);
    float minY = embedding(0, 1);
    float maxY = embedding(0, 1);
    for (int i = 0; i < embedding.M(); i++) {
      minX = embedding(i, 0) < minX ? embedding(i, 0) : minX;
      maxX = embedding(i, 0) > maxX ? embedding(i, 0) : maxX;
      minY = embedding(i, 1) < minY ? embedding(i, 1) : minY;
      maxY = embedding(i, 1) > maxY ? embedding(i, 1) : maxY;
    }
    for (int i = 0; i < embedding.M(); i++) {
      embedding(i, 0) = (embedding(i, 0) - minX) / (maxX - minX) - 0.5;
      embedding(i, 1) = (embedding(i, 1) - minY) / (maxY - minY) - 0.5;
    }

    response["embedding"] = Json::Value(Json::objectValue);
    response["embedding"]["name"] = name;
    // TODO: Write utility for DenseMatrix to JSON nested array conversion.
    auto layout = Json::Value(Json::arrayValue);
    for (int i = 0; i < embedding.M(); i++) {
      auto row = Json::Value(Json::arrayValue);
      for (int j = 0; j < embedding.N(); j++) {
        row.append(embedding(i, j));
      }
      layout.append(row);
    }
    response["embedding"]["layout"] = layout;

    auto adjacency = Json::Value(Json::arrayValue);
    auto neighbors = m_currentVizData->getNearestNeighbors();
    for (int i = 0; i < neighbors.N(); i++) {
      for (int j = 0; j < neighbors.M(); j++) {
        int neighbor = neighbors(j, i);
        if (i == neighbor)
          continue;
        auto row = Json::Value(Json::arrayValue);
        row.append(i);
        row.append(neighbor);
        adjacency.append(row);
      }

    }
    response["embedding"]["adjacency"] = adjacency;
  }
}

/**
 * This fetches the colors of nodes based on the current field's values.
 */
void Controller::fetchNodeColors(const Json::Value &request, Json::Value &response) {
  int embeddingId = request["embeddingId"].asInt();
  if (embeddingId < 0) return setError(response, "invalid embeddingId");

  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  // get the vector of values for the requested field
  Eigen::Map<Eigen::VectorXf> fieldvals = m_currentDataset->getFieldvalues(m_currentField, m_currentCategory, m_currentNormalize);
  if (!fieldvals.data())
    return setError(response, "Invalid fieldname or empty field");

  // Get colors based on current field
  auto colorMap = m_currentVizData->getColorMap(persistence);
  response["colors"] = Json::Value(Json::arrayValue);
  for(unsigned int i = 0; i < fieldvals.size(); ++i) {
    auto colorRow = Json::Value(Json::arrayValue);
    auto color = colorMap.getColor(fieldvals(i));
    colorRow.append(color[0]);
    colorRow.append(color[1]);
    colorRow.append(color[2]);
    response["colors"].append(colorRow);
  }
}

void Controller::fetchMorseSmaleRegression(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  // Get points for regression line
  auto layoutType = HDVizLayout(request["layout"].asString());
  auto layout = m_currentVizData->getLayout(layoutType, persistence);
  int rows = m_currentVizData->getNumberOfSamples();
  double points[rows][3];
  double colors[rows][3];

  // For each crystal
  response["curves"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < m_currentVizData->getCrystals(persistence).N(); i++) {

    // Get all the points and node colors
    for (unsigned int n = 0; n < layout[i].N(); ++n) {
      auto color = m_currentVizData->getColorMap(persistence).getColor(m_currentVizData->getMean(persistence)[i](n));
      colors[n][0] = color[0];
      colors[n][1] = color[1];
      colors[n][2] = color[2];

      for (unsigned int m = 0; m < layout[i].M(); ++m) {
        points[n][m] = layout[i](m, n);
      }
      points[n][2] = m_currentVizData->getMeanNormalized(persistence)[i](n);
    }

    // Get layout for each crystal
    Json::Value regressionObject(Json::objectValue);
    regressionObject["id"] = i;
    regressionObject["points"] = Json::Value(Json::arrayValue);
    regressionObject["colors"] = Json::Value(Json::arrayValue);
    for (unsigned int n = 0; n < rows; ++n) {
      auto regressionRow = Json::Value(Json::arrayValue);
      auto colorRow = Json::Value(Json::arrayValue);
      for (unsigned int m = 0; m < 3; ++m) {
        regressionRow.append(points[n][m]);
        colorRow.append(colors[n][m]);
      }
      regressionObject["points"].append(regressionRow);
      regressionObject["colors"].append(colorRow);
    }
    response["curves"].append(regressionObject);
  }
}

void Controller::fetchMorseSmaleExtrema(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  auto layoutType = HDVizLayout(request["layout"].asString());
  auto extremaLayout = m_currentVizData->getExtremaLayout(layoutType, persistence);
  auto extremaNormalized = m_currentVizData->getExtremaNormalized(persistence);
  auto extremaValues = m_currentVizData->getExtremaValues(persistence);

  response["extrema"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < extremaLayout.N(); ++i) {
    // Position
    Json::Value extremaObject(Json::objectValue);
    extremaObject["position"] = Json::Value(Json::arrayValue);
    extremaObject["position"].append(extremaLayout(0, i));
    extremaObject["position"].append(extremaLayout(1, i));
    extremaObject["position"].append(extremaNormalized(i));

    // Color
    auto color = m_currentVizData->getColorMap(persistence).getColor(extremaValues(i));
    extremaObject["color"] = Json::Value(Json::arrayValue);
    extremaObject["color"].append(color[0]);
    extremaObject["color"].append(color[1]);
    extremaObject["color"].append(color[2]);

    response["extrema"].append(extremaObject);
  }
}

/*
 * Returns samples belonging to this crystal and ids of its extrema.
 */
void Controller::fetchCrystal(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  // the crystal id to look for in this persistence level
  int crystalID = request["crystalID"].asInt();
  std::cout << "selected crystal " << crystalID << std::endl;
  
  // ALL samples belonging to this crystal, including its extrema
  response["crystalSamples"] = Json::Value(Json::arrayValue);
  for (auto id: m_currentVizData->getAllCrystals()[persistence][crystalID]) {
    response["crystalSamples"].append(id);
  }

  // the sample ids of this crystal's extrema
  auto extrema = m_currentVizData->getAllExtrema()[persistence][crystalID];
  response["crystalExtrema"] = Json::Value(Json::arrayValue);
  response["crystalExtrema"].append(extrema.first);  // max
  response["crystalExtrema"].append(extrema.second); // min
}

void Controller::fetchEmbeddingsList(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  auto metric = request["metric"].asString();
  auto embeddingNames = m_currentDataset->getEmbeddingNames(metric);
  response["embeddings"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < embeddingNames.size(); ++i) {
    Json::Value embeddingObject(Json::objectValue);
    embeddingObject["name"] = embeddingNames[i];
    embeddingObject["id"] = i;
    response["embeddings"].append(embeddingObject);
  }
}

/**
 * Handle the command to fetch an array of a named parameter. (todo: fetchParameter and fetchQoi could easily be consolidated)
 */
void Controller::fetchParameter(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  // get the vector of values for the requested field
  std::string parameterName = request["parameterName"].asString();
  Eigen::Map<Eigen::VectorXf> fieldvals = m_currentDataset->getFieldvalues(parameterName, Fieldtype::DesignParameter, false /*normalized*/);
  if (!fieldvals.data())
    return setError(response, "invalid fieldname");
  
  response["parameterName"] = parameterName;
  response["parameter"] = Json::Value(Json::arrayValue);
  for (int i = 0; i < fieldvals.size(); i++) {
    response["parameter"].append(fieldvals(i));
  }
}

/**
 * Handle the command to fetch an array of a named QoI (todo: fetchParameter and fetchQoi could easily be consolidated)
 */
void Controller::fetchQoi(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  // get the vector of values for the requested field
  std::string qoiName = request["qoiName"].asString();
  Eigen::Map<Eigen::VectorXf> fieldvals = m_currentDataset->getFieldvalues(qoiName, Fieldtype::QoI, false /*normalized*/);
  if (!fieldvals.data())
    return setError(response, "invalid fieldname");
  
  response["qoiName"] = qoiName;
  response["qoi"] = Json::Value(Json::arrayValue);
  for (int i = 0; i < fieldvals.size(); i++) {
    response["qoi"].append(fieldvals(i));
  }
}

/**
 * adds the given image to a respose's "thumbnails" array
 */
void addImageToResponse(Json::Value &response, const Image &image) {
  Json::Value imageObject = Json::Value(Json::objectValue);
  imageObject["width"] = image.getWidth();
  imageObject["height"] = image.getHeight();
  imageObject["rawData"] = base64_encode(image.getPNGData().data(), image.getPNGData().size());
  response["thumbnails"].append(imageObject);
}

/**
 * Handle the command to fetch sample image thumbnails if available.
 */
void Controller::fetchThumbnails(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  auto thumbnails = m_currentDataset->getThumbnails();

  response["thumbnails"] = Json::Value(Json::arrayValue);
  for (auto& image : thumbnails) {
    addImageToResponse(response, image);
  }
}

/**
 * This returns a set of new samples (images) computed with the given ShapeOdds model for
 * the specified crystal at this persistence level using numZ latent space coordinates.
 * The z_coords are created for numZ evenly-spaced values of the field covered by this crystal
 *
 * Parameters: 
 *   datasetId     - should already be loaded
 *   category      - e.g., qoi or param (used to get fieldvalues)
 *   fieldname     - e.g., one of the QoIs
 *   persistence   - persistence level of the M-S for this field of the dataset
 *   metric        - distance matrix used to compute the M-S for this field  // <ctc> needed>?
 *   crystalID     - crystal of the given persistence level
 *   modelname     - model from which to fetch interpolated samples
 *   numSamples    - number of evenly-spaced samples of the field at which to generate new latent images
 *   sigmaScale    - Scale of Gaussian sigma used for generating new z_coord for shape
 *   showOrig      - simply return original samples for this crystal
 *   validate      - generate model-interpolated images using the z_coords it provided
 *   diff_validate - return diffs of the model-interpolated images with the originals
 *   percent       - distance along crystal from which to find/generate sample (when only one is requested)
 */
void Controller::fetchNImagesForCrystal(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  // category of the passed fieldname (design param or qoi)
  Fieldtype category = Fieldtype(request["category"].asString());
  if (!category.valid()) return setError(response, "invalid category");

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  // read model parameters
  auto fieldname = request["fieldname"].asString();
  auto modelname = request["modelname"].asString();
  auto crystalId = request["crystalID"].asInt();
  auto metric = request["metric"].asString();


  time_point<Clock> start = Clock::now();

  // try to find the requested model
  auto modelset(m_currentDataset->getModelset(metric, fieldname, modelname));
  auto persistence_idx = getAdjustedPersistenceLevelIdx(persistence, modelset);
  auto model(modelset ? modelset->getModel(persistence_idx, crystalId) : nullptr);

  milliseconds diff = duration_cast<milliseconds>(Clock::now() - start);
  //std::cout << "model loaded in " << static_cast<float>(diff.count())/1000.0f << "s" << std::endl;


  // if there isn't a model or original images requested just show its original samples' images 
  bool showOrig = request["showOrig"].asBool();
  if (showOrig || !model)
    return fetchCrystalOriginalSampleImages(request, response);

  // if requested, validate model by generating interpolated images using z_coords of its own samples
  bool validate = request["validate"].asBool();
  if (validate) {
    bool diff = request["return_diff"].asBool();
    return regenOriginalImagesForCrystal(*modelset, model, persistence_idx, crystalId, diff, response);
  }

  // interpolate the model for the given samples
  auto numZ = request["numSamples"].asInt();
  auto percent = request["percent"].asFloat();
  /*
  std::cout << "fetchNImagesForCrystal: " << numZ << " samples requested for crystal "<<crystalId<<" of persistence level "<<persistence <<"; datasetId is "<<m_currentDatasetId<<", fieldname is "<<fieldname<<", modelname is " << modelname;
  if (numZ == 1) std::cout << " (percent is " << percent << ")";
  std::cout << std::endl;
  */  

  // get the vector of values for the field
  auto samples(modelset->getCrystalFieldvals(persistence_idx, crystalId));
  Eigen::Map<Eigen::VectorXf> fieldvals(samples.data(), samples.size());
  if (!fieldvals.data())
    return setError(response, "Invalid fieldname or empty field");

  // partition the field into numZ values and evaluate model for each
  float minval = model->minFieldValue();
  float maxval = model->maxFieldValue();
  float delta = (maxval - minval) / static_cast<float>(numZ-1); // (numZ - 1) so it generates samples for crystal min and max

  // if numZ == 1, evaluate at given percent along crystal
  if (numZ == 1) {
    delta = 1.0;
    minval = minval + (maxval - minval) * percent;
  }

  // get sigma to determine width of samples to use
  Precision sigmaScale = static_cast<Precision>(request["sigmaScale"].asInt());
  sigmaScale *= modelset->getCrystalSigma(persistence_idx, crystalId);

  // get dims of image to be created by model from one of the original samples
  const Image& sample = m_currentDataset->getThumbnail(0);
  auto width{sample.getWidth()};
  auto height{sample.getHeight()};
  auto numChannels{sample.numChannels()};

  for (unsigned i = 0; i < numZ; i++)
  {
    // compute new field value and add it to response
    auto fieldval = minval + delta * i;
    response["fieldvals"].append(fieldval);

    // add "sample ids" to response (multiplying them by 10 to make it a little more obvious it's from interpolation)
    response["sampleids"].append((int)i * 10 - 1);

    // get new latent space coordinate for this field_val
    Eigen::RowVectorXf z_coord = model->getNewLatentSpaceValue(fieldvals, model->getZCoords(), fieldval, sigmaScale);

    // evaluate model at this coordinate
    std::shared_ptr<Eigen::MatrixXf> I = model->evaluate(z_coord);

    if (modelset->hasCustomRenderer()) {
      // push to genthumbs list; main thread will call python rendering function
      genthumbs.emplace_back(Thumbgen{I, *modelset, response, width, height});
    }
    else {
      // convert resultant matrix to a 2d image
      Image image(*I, width, height, numChannels, modelset->rotate());
    
      // add result image to response 
      addImageToResponse(response, image);
    }
  }
  
  if (modelset->hasCustomRenderer()) {
    start = Clock::now();

    // wait until all thumbnails have been generated and added to response
    while (!genthumbs.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << "waited for " << numZ << " thumbnails (50ms at a time) for "
              << duration_cast<milliseconds>(Clock::now() - start).count()
              << " ms" << std::endl;
  }

  /*
  // inform caller of number and source of returned images
  response["msg"] =
    std::string("generated " + std::to_string(numZ) +
                " P" + std::to_string(persistence) +
                "-C" + std::to_string(crystalId) + " " +
                modelname + "-predicted images.");
  */
}

/*
 * Calls a custom Python thumbnail generation function with the matrix produced by the model evaluation.
 * - MUST be called from main thread (aka main in server.cpp) if renderer uses OpenGL (most of them do)
*/
void Controller::generateCustomThumbnail(std::shared_ptr<Eigen::MatrixXf> I, MSModelset& modelset,
                                         Json::Value &response, unsigned width, unsigned height) {
  using namespace pybind11::literals;
  auto& ren = modelset.getCustomRenderer();

  time_point<Clock> start = Clock::now();


  // update renderer with new data
  try {
    ren.attr("update")(*I);
  } catch(std::exception) {
    std::cerr << "error updating extern renderer. Ignoring\n";
  }
  
  time_point<Clock> end = Clock::now();
  std::cout << "vertices updated in " << duration_cast<milliseconds>(end - start).count() << " ms" << std::endl;
  start = end;

  // fetch new image
  py::list resolution;
  resolution.append(width);
  resolution.append(height);
  auto npvec = ren.attr("getImage")("resolution"_a = resolution,
                                    "scale"_a = modelset.getImageScale).cast<py::array_t<unsigned char>>();

  end = Clock::now();
  std::cout << "image generated in " << duration_cast<milliseconds>(end - start).count() << " ms" << std::endl;
  start = end;


  // convert numpy array to Image 
  Image image(dspacex::toStdVec(npvec), width, height, 3);

#if 0
  // <ctc> debug by also dumping the generated image
  std::ostringstream os;
  static int genidx{0};
  os << "/tmp/generated-thumbnail-" << std::setfill('0') << std::setw(3) << genidx++ << ".png";
  image.write(os.str());
#endif
  

  // add result image to response 
  addImageToResponse(response, image);
}

/**
 * computes index of the requested (0-based) persistence level in this M-S complex
 * (since there could be more actual persistence levels than those stored in the complex)
 * TODO: make MSModelset function adjust internally, so from outside it just asks for the actualy persistence level (almost there)
 */
int Controller::getAdjustedPersistenceLevelIdx(const unsigned desired_persistence, const std::shared_ptr<MSModelset>& mscomplex) const
{
  int persistence_idx = desired_persistence -
    (m_currentTopoData->getMaxPersistenceLevel() - (mscomplex ? mscomplex->numPersistenceLevels() : 0) + 1);

  return persistence_idx;
}

/**
 * This returns a set of new samples (images) computed with the given ShapeOdds model for the
 * specified crystal at this persistence level using the latent space coordinates for each of
 * the original samples of model/crystal (each sample has a z_coord).
 * Either add generated or difference image to response.
 */
void Controller::regenOriginalImagesForCrystal(MSModelset &modelset, std::shared_ptr<Model> model,
                                               int persistence_idx, int crystalId, bool compute_diff,
                                               Json::Value &response) {
  auto samples(modelset.getPersistenceLevel(persistence_idx).crystals[crystalId].getSamples());
  std::cout << "Testing all latent space variables computed for the "
            << samples.size() << " samples in this crystal.\n";

  for (auto sample: samples)
  {
    // load thumbnail corresponding to this z_idx for comparison of model at same z_idx (they should be close)
    const Image& orig = m_currentDataset->getThumbnail(sample.idx);
    auto width{orig.getWidth()};
    auto height{orig.getHeight()};
    auto numChannels{orig.numChannels()};

    Eigen::MatrixXf z_coord(model->getZCoord(sample.local_idx));
    Eigen::MatrixXf I = *model->evaluate(z_coord);

    Image image(I, width, height, numChannels, modelset.rotate());
    if (compute_diff) {
      image -= orig;
    }
    addImageToResponse(response, image);
    
    // add field value to response
    response["fieldvals"].append(sample.val);
  }

  response["msg"] = std::string("returning interpolated images by model at crystal " + std::to_string(crystalId) + " of persistence_idx" + std::to_string(persistence_idx));
}

/* 
 * Returns vector of global sample ids and their fieldvalue for the samples from which the crystal is comprised.
 * Used when Model == None since there is no Modelset in that case.
 */
std::vector<ValueIndexPair> Controller::getSamples(Fieldtype category, const std::string &fieldname,
                                                   unsigned persistence, unsigned crystalid, bool sort) {
  std::vector<ValueIndexPair> fieldvalues_and_indices;

  // get the vector of values for the field
  Eigen::Map<Eigen::VectorXf> fieldvals = m_currentDataset->getFieldvalues(fieldname, category, false /*normalized*/);
  if (!fieldvals.data()) {
    std::cerr << "Invalid fieldname or empty field\n";
    return fieldvalues_and_indices;
  }

  // create a vector of global sample ids and their fieldvalue for the samples from which this crystal is comprised
  auto crystal = m_currentVizData->getAllCrystals()[persistence][crystalid];
  for (auto i: crystal) {
    ValueIndexPair sample;
    sample.idx = i;
    sample.val = fieldvals(i);
    fieldvalues_and_indices.push_back(sample);
  }

  // sort it by increasing fieldvalue
  if (sort)
    std::sort(fieldvalues_and_indices.begin(), fieldvalues_and_indices.end(), ValueIndexPair::compare);

  return fieldvalues_and_indices;
}

/**
 * Fetches the original sample images for the specified crystal (no MSModelset/crystal/model required)
 */
void Controller::fetchCrystalOriginalSampleImages(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  // category of the passed fieldname (design param or qoi)
  Fieldtype category = Fieldtype(request["category"].asString());
  if (!category.valid()) return setError(response, "invalid category");

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  auto fieldname = request["fieldname"].asString();
  auto crystalid = request["crystalID"].asInt();
  auto samples(getSamples(category, fieldname, persistence, crystalid));

  for (auto sample: samples)
  {
    // load thumbnail corresponding to this z_idx
    const Image& orig = m_currentDataset->getThumbnail(sample.idx);

    // add image to response
    addImageToResponse(response, orig);  // todo: add index to response so drawer can display it

    // add field value to response
    response["fieldvals"].append(sample.val);

    // add sample ids to response
    response["sampleids"].append(sample.idx);
  }

  response["msg"] = std::string("returning original " + std::to_string(samples.size()) + " images for crystal " + std::to_string(crystalid) + " of persistence level " + std::to_string(persistence));
}

/**
 * loadDataset
 *
 * Called by maybeLoadDataset which validates parameters.
 */
bool Controller::loadDataset(int datasetId) {
  if (datasetId == m_currentDatasetId)
    return true;

  // todo: handle errors that can occur when loading the dataset
  std::string configPath = m_availableDatasets[datasetId].second;
  m_currentDataset = DatasetLoader::loadDataset(configPath);
  m_currentDatasetId = datasetId;

  // clear current computation results
  m_currentVizData = nullptr;
  m_currentTopoData = nullptr;

  return true;
}

/**
 * Checks if the requested dataset is loaded.
 * If not, loads the dataset and sets state.
 *
 * Returns true if no need to load or load successful, 
 * false if invalid dataset id or load failed.
 */
bool Controller::maybeLoadDataset(const Json::Value &request, Json::Value &response) {
  auto datasetId = request.isMember("datasetId") ? request["datasetId"].asInt() : m_currentDatasetId;

  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    setError(response, "datasetId does not exist");
    return false;
  }

  if (!loadDataset(datasetId)) {
    setError(response, "dataset failed to load");
    return false;
  }

  return true;
}

/**
 * Check to see if parameters of request are valid to process data
 */
bool Controller::verifyProcessDataParams(Fieldtype category, std::string fieldname, int knn, std::string metric,
                                         int curvepoints, double datasigma, double curvesigma, bool addnoise, int depth,
                                         bool normalize, Json::Value &response) {

  // category of the passed fieldname (design param or qoi)
  if (!category.valid()) {
    setError(response, "invalid category");
    return false;
  }

  // desired fieldname (one of the design params or qois)
  if (!verifyFieldname(category, fieldname)) {
    setError(response, "invalid fieldname");
    return false;
  }

  // metric of distance matrix
  if (!m_currentDataset->hasDistanceMatrix(metric)) {
    setError(response, "invalid distance metric");
    return false;
  }  
  
  // M-S computation params
  if (knn < 0) {
    setError(response, "knn must be >= 0");
    return false;
  } else if (datasigma < 0) {
    setError(response, "datasigma must be >= 0");
    return false;
  } else if (curvesigma < 0) {
    setError(response, "smooth must be >= 0");
    return false;
  } else if (depth <= 0 && depth != -1) {
    setError(response, "must compute at least one (depth > 0) or all (depth = -1) persistence levels");
    return false;
  } else if (curvepoints < 3) {
    setError(response, "regression curves must have at least 3 points");
    return false;
  }

  return true;
}

/**
 * try to process this request to compute M-S
 */
bool Controller::maybeProcessData(const Json::Value &request, Json::Value &response) {

  // read all parameters that can exist in request, defaulting to current value
  auto category    = request.isMember("category")    ? Fieldtype(request["category"].asString()) : m_currentCategory;
  auto fieldname   = request.isMember("fieldname")   ? request["fieldname"].asString()           : m_currentField;
  auto knn         = request.isMember("knn")         ? request["knn"].asInt()                    : m_currentKNN;
  auto metric      = request.isMember("metric")      ? request["metric"].asString()              : m_currentDistanceMetric;
  auto curvepoints = request.isMember("curvepoints") ? request["curvepoints"].asInt()            : m_currentNumCurvepoints;
  auto datasigma   = request.isMember("datasigma")   ? request["datasigma"].asFloat()            : m_currentSmoothDataSigma;
  auto curvesigma  = request.isMember("curvesigma")  ? request["curvesigma"].asFloat()           : m_currentSmoothCurveSigma;
  auto addnoise    = request.isMember("noise")       ? request["noise"].asBool()                 : m_currentAddNoise;
  auto depth       = request.isMember("depth")       ? request["depth"].asInt()                  : m_currentPersistenceDepth;
  auto normalize   = request.isMember("normalize")   ? request["normalize"].asBool()             : m_currentNormalize;

  if (!verifyProcessDataParams(category, fieldname, knn, metric, curvepoints, datasigma,
                               curvesigma, addnoise, depth, normalize, response))
    return false; // response will contain the error

  if (!processData(category, fieldname, knn, metric, curvepoints, datasigma, curvesigma, addnoise, depth, normalize)) {
    setError(response, "failed to process data");
    return false;
  }

  return true;
}

/// Avoid regeneration of data if parameters haven't changed
bool Controller::processDataParamsChanged(Fieldtype category, std::string fieldname, int knn, std::string metric,
                                          int num_samples, double datasigma, double curvesigma, bool add_noise,
                                          int num_persistences, bool normalize) {
  return !(m_currentCategory        == category &&
           m_currentField           == fieldname &&
           m_currentKNN             == knn &&
           m_currentDistanceMetric  == metric &&
           m_currentNumCurvepoints  == num_samples &&
           m_currentSmoothDataSigma == datasigma &&
           m_currentSmoothCurveSigma== curvesigma &&
           m_currentAddNoise        == add_noise &&
           m_currentPersistenceDepth== num_persistences &&
           m_currentNormalize       == normalize);
}

/**
 * Checks if the requested dataset has been processed. If not, processes the data.
 *
 * Called by maybeProcessData which validates parameters.
 *
 * Returns true if no need to process or process successful, 
 * false if processing failed.
 */
bool Controller::processData(Fieldtype category, std::string fieldname, int knn, std::string metric,
                             int num_samples, double datasigma, double curvesigma, bool add_noise,
                             int num_persistences, bool normalize) {
  if (m_currentTopoData &&
      !processDataParamsChanged(category, fieldname, knn, metric, num_samples, datasigma, curvesigma,
                                add_noise, num_persistences, normalize)) {
    return true;
  }

  std::cout << "computing nnmscomplex for fieldname: " << fieldname << "..." << std::endl;
  time_point<Clock> start = Clock::now();

  // get the vector of values for the requested field
  Eigen::Map<Eigen::Matrix<Precision, Eigen::Dynamic, 1>> fieldvals = m_currentDataset->getFieldvalues(fieldname, category, normalize);
  if (!fieldvals.data()) {
    std::cerr << "Controller::processData failed: invalid fieldname or empty field.\n";
    return false;
  }

  // clear current computation results
  m_currentVizData = nullptr;
  m_currentTopoData = nullptr;

  // load or generate the distance matrix 
  if (m_currentDataset->hasDistanceMatrix(metric)) {
    m_currentDistanceMatrix = m_currentDataset->getDistanceMatrix(metric);
  } else if (m_currentDataset->hasGeometryMatrix()) {
    auto geometrysMatrix = m_currentDataset->getGeometryMatrix();
    m_currentDistanceMatrix = HDProcess::computeDistanceMatrix<Precision>(geometrysMatrix);
  } else {
    std::cerr << "processData failed: no distance matrix or geometrysMatrix available\n";
    return false;
  }

  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
  try {
    m_currentVizData.reset(new SimpleHDVizDataImpl(
      genericProcessor.processOnMetric(m_currentDistanceMatrix,
                                       FortranLinalg::DenseVector<Precision>(fieldvals.size(), fieldvals.data()),
                                       knn,              /* k nearest neighbors to consider */
                                       num_samples,      /* points along each crystal regression curve */
                                       num_persistences, /* generate this many at most; -1 generates all of 'em */
                                       add_noise,        /* adds very slight noise to field values */
                                       curvesigma,       /* soften crystal regression curves */
                                       datasigma)));     /* smooth data to compute topology */
    m_currentTopoData.reset(new LegacyTopologyDataImpl(m_currentVizData));
  } catch (const char *err) {
    std::cerr << "Controller::processData: processOnMetric failed: " << err << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Controller::processData: processOnMetric failed with unknown exception." << std::endl;
    return false;
  }

  // save current processing state to avoid unnecessary recomputation
  m_currentCategory = category;
  m_currentField = fieldname;
  m_currentDistanceMetric = metric;
  m_currentKNN = knn;
  m_currentNumCurvepoints = num_samples;
  m_currentSmoothDataSigma = datasigma;
  m_currentSmoothCurveSigma = curvesigma;
  m_currentAddNoise = add_noise;
  m_currentPersistenceDepth = num_persistences;
  m_currentNormalize = normalize;
  
  std::cout << "computation complete (" << duration_cast<milliseconds>(Clock::now() - start).count() << " ms)\n";

  return true;
}

} // dspacex
