#include <base64/base64.h>
#include <boost/filesystem.hpp>
#include "Controller.h"
#include "dspacex/DatasetLoader.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/DenseVector.h"
#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "hdprocess/HDGenericProcessor.h"
#include "hdprocess/LegacyTopologyDataImpl.h"
#include "hdprocess/SimpleHDVizDataImpl.h"
#include "hdprocess/TopologyData.h"
#include <jsoncpp/json/json.h>
#include "dspacex/Precision.h"
#include "serverlib/wst.h"
#include "utils/DenseVectorSample.h"
#include "utils/loaders.h"
#include "utils/utils.h"
#include "pmodels/Models.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <fstream>
#include <string>

using namespace dspacex;

// Simplify namespace access to _1, _2 for std::bind parameter binding.
using namespace std::placeholders;

// Maximum number of directories from root path to seek config.yaml files
const int MAX_DATASET_DEPTH = 3;


Controller::Controller(const std::string &datapath_) : datapath(datapath_) {
  configureCommandHandlers();
  configureAvailableDatasets(datapath);
}

// setError
// Sets the given error message in the Json response.
void Controller::setError(Json::Value &response, const std::string &str)
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
  m_commandMap.insert({ "writeMorseSmaleDecomposition", std::bind(&Controller::writeMorseSmaleDecomposition, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmalePersistenceLevel", std::bind(&Controller::fetchMorseSmalePersistenceLevel, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleCrystal", std::bind(&Controller::fetchMorseSmaleCrystal, this, _1, _2)});
  m_commandMap.insert({"fetchSingleEmbedding", std::bind(&Controller::fetchSingleEmbedding, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleRegression", std::bind(&Controller::fetchMorseSmaleRegression, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleExtrema", std::bind(&Controller::fetchMorseSmaleExtrema, this, _1, _2)});
  m_commandMap.insert({"fetchCrystalPartition", std::bind(&Controller::fetchCrystalPartition, this, _1, _2)});
  m_commandMap.insert({"fetchModelsList", std::bind(&Controller::fetchModelsList, this, _1, _2)});
  m_commandMap.insert({"fetchEmbeddingsList", std::bind(&Controller::fetchEmbeddingsList, this, _1, _2)});
  m_commandMap.insert({"fetchParameter", std::bind(&Controller::fetchParameter, this, _1, _2)});
  m_commandMap.insert({"fetchQoi", std::bind(&Controller::fetchQoi, this, _1, _2)});
  m_commandMap.insert({"fetchThumbnails", std::bind(&Controller::fetchThumbnails, this, _1, _2)});
  m_commandMap.insert({"fetchImageForLatentSpaceCoord_Shapeodds", std::bind(&Controller::fetchImageForLatentSpaceCoord_Shapeodds, this, _1, _2)});
  m_commandMap.insert({"fetchNImagesForCrystal_Shapeodds", std::bind(&Controller::fetchNImagesForCrystal_Shapeodds, this, _1, _2)});
  m_commandMap.insert({"fetchAllImagesForCrystal_Shapeodds", std::bind(&Controller::fetchAllImagesForCrystal_Shapeodds, this, _1, _2)});

  m_commandMap.insert({"fetchAllForLatentSpaceUsingSharedGP", std::bind(&Controller::fetchAllForLatentSpaceUsingSharedGP, this, _1, _2)});
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
    if (iter.level() == MAX_DATASET_DEPTH) {
      iter.pop();
    }
    if (iter->path().filename() != "config.yaml") {
      iter++;
      continue;
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

// verifyFieldname
// ensure the fieldname exists in this Controller's dataset
bool Controller::verifyFieldname(Fieldtype type, const std::string &name)
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
    std::cout << "[" << messageId << "] " << commandName << std::endl;

    Json::Value response(Json::objectValue);
    response["id"] = messageId;

    auto command = m_commandMap[commandName];
    if (command) {
      command(request, response);
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
  response["parameterNames"] = Json::Value(Json::arrayValue);
  for (std::string parameterName : m_currentDataset->getParameterNames()) {
    response["parameterNames"].append(parameterName);
  }
  response["qoiNames"] = Json::Value(Json::arrayValue);
  for (std::string qoiName : m_currentDataset->getQoiNames()) {
    response["qoiNames"].append(qoiName);
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

  // TODO: Don't reprocess data if previous commands already did so.
  int n = m_currentDataset->getDistanceMatrix().N();
  auto KNN = FortranLinalg::DenseMatrix<int>(k, n);
  auto KNND = FortranLinalg::DenseMatrix<Precision>(k, n);
  Distance<Precision>::findKNN(m_currentDataset->getDistanceMatrix(), KNN, KNND);

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
  std::cout << "Controller::fetchMorseSmaleDecomposition persistence range: [" << minLevel << "," << maxLevel << "]\n";

  response["datasetId"] = m_currentDatasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["minPersistenceLevel"] = minLevel;
  response["maxPersistenceLevel"] = maxLevel;
  response["complexSizes"] = Json::Value(Json::arrayValue);
  for (int level = minLevel; level <= maxLevel; level++) {
    std::shared_ptr<MorseSmaleComplex> complex = m_currentTopoData->getComplex(level);
    int size = complex->getCrystals().size();
    response["complexSizes"].append(size);
    std::cout << "Controller::fetchMorseSmaleDecomposition persistence " << level << " num crystals: " << size << std::endl;
  }
}

/**
 * Write the current morse smale decomposition of a dataset.
 */
void Controller::writeMorseSmaleDecomposition(const Json::Value &request, Json::Value &response)
{
    response["neighborhoodSize"] = m_currentKNN;
    response["sigma"] = m_currentSigma;
    response["smoothing"] = m_currentSmoothing;
    response["crystalCurvepoints"] = m_currentNumCurvepoints;

    auto crystal_partitions = m_currentVizData->getAllCrystalPartitions();
    response["crystalPartitions"] = Json::Value(Json::arrayValue);
    for (unsigned i = 0; i < m_currentVizData->getPersistence().N(); ++i) {
        Json::Value row = Json::Value(Json::arrayValue);
        response["crystalPartitions"].append(row);
        for (unsigned j = 0; j < crystal_partitions[i].N(); ++j) {
            response["crystalPartitions"][i].append(crystal_partitions[i](j));
        }
    }
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
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  std::shared_ptr<MorseSmaleComplex> complex = m_currentTopoData->getComplex(persistenceLevel);

  response["datasetId"] = m_currentDatasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["k"] = m_currentKNN;
  response["persistenceLevel"] = persistenceLevel;
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
 * Handle the command to fetch the details of a single crystal in a persistence level.
 */
// TODO: NOT USED BY ANYTHING!
void Controller::fetchMorseSmaleCrystal(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  std::shared_ptr<MorseSmaleComplex> complex = m_currentTopoData->getComplex(persistenceLevel);

  int crystalId = request["crystalId"].asInt();
  if (crystalId < 0 || crystalId >= complex->getCrystals().size())
    return setError(response, "invalid crystal id");

  std::shared_ptr<Crystal> crystal = complex->getCrystals()[crystalId];

  response["datasetId"] = m_currentDatasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["persistenceLevel"] = persistenceLevel;
  response["crystalId"] = crystalId;
  response["crystal"] = Json::Value(Json::objectValue);
  response["crystal"]["minIndex"] = crystal->getMinSample();
  response["crystal"]["maxIndex"] = crystal->getMaxSample();
  response["crystal"]["sampleIndexes"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < crystal->getAllSamples().size(); i++) {
    unsigned int index = crystal->getAllSamples()[i];
    response["crystal"]["sampleIndexes"].append(index);
  }

  // TODO: Add crystal adjacency information
}

/**
 * This fetches the graph embedding layout for a given persistence level
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

  // get requested persistence level
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  if (m_currentDataset->numberOfEmbeddings() > 0) {
    auto embedding = m_currentDataset->getEmbeddingMatrix(embeddingId);
    auto name = m_currentDataset->getEmbeddingNames()[embeddingId];

    // TODO: Factor out a normalizing routine.
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

  // get the vector of values for the requested field
  Eigen::Map<Eigen::VectorXd> fieldvals = getFieldvalues(m_currentCategory, m_currentField);
  if (!fieldvals.data())
    return setError(response, "Invalid fieldname or empty field");

  // Get colors based on current field
  auto colorMap = m_currentVizData->getColorMap(persistenceLevel);
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
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  // Get points for regression line
  auto layout = m_currentVizData->getLayout(HDVizLayout::ISOMAP, persistenceLevel);
  int rows = m_currentVizData->getNumberOfSamples();
  double points[rows][3];
  double colors[rows][3];

  // For each crystal
  response["curves"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < m_currentVizData->getCrystals(persistenceLevel).N(); i++) {

    // Get all the points and node colors
    for (unsigned int n = 0; n < layout[i].N(); ++n) {
      auto color = m_currentVizData->getColorMap(persistenceLevel).getColor(m_currentVizData->getMean(persistenceLevel)[i](n));
      colors[n][0] = color[0];
      colors[n][1] = color[1];
      colors[n][2] = color[2];

      for (unsigned int m = 0; m < layout[i].M(); ++m) {
        points[n][m] = layout[i](m, n);
      }
      points[n][2] = m_currentVizData->getMeanNormalized(persistenceLevel)[i](n);
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
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  auto extremaLayout = m_currentVizData->getExtremaLayout(HDVizLayout::ISOMAP, persistenceLevel);
  auto extremaNormalized = m_currentVizData->getExtremaNormalized(persistenceLevel);
  auto extremaValues = m_currentVizData->getExtremaValues(persistenceLevel);

  response["extrema"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < extremaLayout.N(); ++i) {
    // Position
    Json::Value extremaObject(Json::objectValue);
    extremaObject["position"] = Json::Value(Json::arrayValue);
    extremaObject["position"].append(extremaLayout(0, i));
    extremaObject["position"].append(extremaLayout(1, i));
    extremaObject["position"].append(extremaNormalized(i));

    // Color
    auto color = m_currentVizData->getColorMap(persistenceLevel).getColor(extremaValues(i));
    extremaObject["color"] = Json::Value(Json::arrayValue);
    extremaObject["color"].append(color[0]);
    extremaObject["color"].append(color[1]);
    extremaObject["color"].append(color[2]);

    response["extrema"].append(extremaObject);
  }
}

void Controller::fetchCrystalPartition(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  // the crystal id to look for in this persistence level
  int crystalID = request["crystalID"].asInt();

  auto crystal_partition = m_currentVizData->getCrystalPartitions(persistenceLevel);

  response["crystalSamples"] = Json::Value(Json::arrayValue);
  for(unsigned int i = 0; i < crystal_partition.N(); ++i) {
    if(crystal_partition(i) == crystalID) {
      response["crystalSamples"].append(i);
    }
  }
}

void Controller::fetchModelsList(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  auto modelTypes = m_currentDataset->getModelTypes();
  response["models"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < modelTypes.size(); ++i) {
    Json::Value modelObject(Json::objectValue);
    modelObject["name"] = modelTypes[i];
    modelObject["id"] = i;
    response["models"].append(modelObject);
  }
}

void Controller::fetchEmbeddingsList(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  auto embeddingNames = m_currentDataset->getEmbeddingNames();
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
  Eigen::Map<Eigen::VectorXd> fieldvals = getFieldvalues(Fieldtype::DesignParameter, parameterName);
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
  Eigen::Map<Eigen::VectorXd> fieldvals = getFieldvalues(Fieldtype::QoI, qoiName);
  if (!fieldvals.data())
    return setError(response, "invalid fieldname");
  
  response["qoiName"] = qoiName;
  response["qoi"] = Json::Value(Json::arrayValue);
  for (int i = 0; i < fieldvals.size(); i++) {
    response["qoi"].append(fieldvals(i));
  }
}

/**
 * Handle the command to fetch sample image thumbnails if available.
 */
void Controller::fetchThumbnails(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  auto thumbnails = m_currentDataset->getThumbnails();

  response["thumbnails"] = Json::Value(Json::arrayValue);
  for (auto image : thumbnails) {
    Json::Value imageObject = Json::Value(Json::objectValue);
    imageObject["width"] = image.getWidth();
    imageObject["height"] = image.getHeight();
    // imageObject["data"] = base64_encode(  // <ctc> not needed by anything, so why are we sending this?
    //                                     reinterpret_cast<const unsigned char *>(image.getData()),
    //                                     4 * image.getWidth() * image.getHeight());
    imageObject["rawData"] = base64_encode(reinterpret_cast<const unsigned char *>(&image.getRawData()[0]),
                                           image.getRawData().size());
    response["thumbnails"].append(imageObject);
    //response["fieldvals"].append(sample.val);  //<ctc> I don't think this function associates fieldvals with samples, and fetching this is awkward right now, so just putting this in as a placeholder in case it's desired 
  }
}

/**
 * This computes and returns a new sample (image) for the given latent space coordinate using
 * the ShapeOdds model for the specified crystal at this persistence level
 * TODO <ctc>: can't this just be done by calling fetchNImage with N=1?
 *
 * Parameters: 
 *   datasetId     - should already be loaded
 *   fieldname     - (e.g., one of the QoIs)
 *   persistenceId - persistence level of the M-S for this field of the dataset
 *   crystalId     - crystal of the given persistence level
 *   z_coord       - latent space coordinate (<ctc> todo: add fetchNewLatentSpaceCoordForShapeOddsModel function to compute z)
 */
void Controller::fetchImageForLatentSpaceCoord_Shapeodds(const Json::Value &request, Json::Value &response) {
#if 0
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");
  // maybeLoadModel(persistence,crystal); //<ctc> if for speed we decide not to load models till their evaluation is requested

  int persistence = 15; //request["persistence"].asInt();
  int crystalid = 6; //request["crystalid"].asInt();
  //int zidx = request["zidx"].asInt();  // <ctc> really, we want to pass a latent space variable z, which we'll also generate serverside
  std::cout << "fetchImageForLatentSpaceCoord_Shapeodds: datasetId is "<<datasetId<<", persistence is "<<persistence<<", crystalid is "<<crystalid<<std::endl;

  //create images using the elements of this model's Z
  const dspacex::Model &model(m_currentDataset->getMSModels()[0].getModel(persistence, crystalid).second);

  Eigen::MatrixXd new_sample =  ShapeOdds::evaluateModel(model, z_coord);

  ImageBase64 image = MatrixToImage(new_sample);

  addImageToResponse(response, image); // <ctc> factor out common functionality as getThumbnails uses
#endif
  
  response["msg"] = std::string("here's your new sample computing using ShapeOdds(tm). Have a nice day!");
}

/**
 * This returns a set of new samples (images) computed with the given ShapeOdds model for
 * the specified crystal at this persistence level using numZ latent space coordinates.
 * The z_coords are created for numZ evenly-spaced values of the field covered by this crystal
 *
 * Parameters: 
 *   datasetId     - should already be loaded
 *   fieldname     - (e.g., one of the QoIs)
 *   persistenceId - persistence level of the M-S for this field of the dataset
 *   crystalId     - crystal of the given persistence level
 *   numZ          - number of evenly-spaced levels of this model's field at which to generate new latent space coordinates
 */
void Controller::fetchNImagesForCrystal_Shapeodds(const Json::Value &request, Json::Value &response)
{
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");
  // maybeLoadModel(persistence,crystal); //<ctc> TODO: for speed, do not load models till their evaluation is requested

  // category of the passed fieldname (design param or qoi)
  Fieldtype category = Fieldtype(request["category"].asString());
  if (!category.valid()) return setError(response, "invalid category");

  // get requested persistence level
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  std::string fieldname = request["fieldname"].asString();
  int crystalid = request["crystalID"].asInt();
  
  int mscomplex_idx = m_currentDataset->getMSComplexIdxForFieldname(fieldname);

  // if there isn't a model associated with the crystal at this plvl, just show its associated samples' images 
  if (mscomplex_idx < 0)
    return fetchCrystalOriginalSampleImages(request, response);

  dspacex::MSComplex &mscomplex = m_currentDataset->getMSComplex(fieldname);
  int persistenceLevel_idx = getPersistenceLevelIdx(persistenceLevel, mscomplex);

  // if there isn't a model for the selected persistence level, just show its associated samples' images
  if (persistenceLevel_idx < 0)
    return fetchCrystalOriginalSampleImages(request, response); // fixme: plvl access should simply be by actual plvl, not this idx confusion (class consolidation should resolve this)

  // original images requested
  bool showOrig = request["showOrig"].asBool();
  if (showOrig)
    return fetchCrystalOriginalSampleImages(request, response);

  // model-interpolated images for original samples requested
  bool interpolateOrig = request["interpolateOrig"].asBool();
  if (interpolateOrig)
    return fetchAllImagesForCrystal_Shapeodds(request, response);

  int numZ = request["numSamples"].asInt();
  double percent = request["percent"].asDouble();
  std::cout << "fetchNImagesForCrystal_Shapeodds: " << numZ << " samples requested for crystal "<<crystalid<<" of persistence level "<<persistenceLevel<<" (datasetId is "<<m_currentDatasetId<<", fieldname is "<<fieldname<<", percent is "<<percent<<")\n";
  
  dspacex::Model &model(mscomplex.getModel(persistenceLevel_idx, crystalid).second);  

  // <ctc> we need to connect the dataset and its values more closely when reading a model, as the crystal's model should already know its fieldvalues
  // get the vector of values for the field
  Eigen::Map<Eigen::VectorXd> fieldvals = getFieldvalues(category, fieldname);
  if (!fieldvals.data())
    return setError(response, "Invalid fieldname or empty field");
  model.setFieldValues(fieldvals);

  const Image& sample_image = m_currentDataset->getThumbnail(0);  // just using this to get dims of image created by model prediction

  // partition the crystal's model's field (QoI) into numZ values and evaluate model for each of them
  double minval = model.minFieldValue();
  double maxval = model.maxFieldValue();
  double delta = (maxval - minval) / static_cast<double>(numZ-1);  // / (numZ - 1) so it will generate samples for the crystal min and max
  double sigma = 0.15; // ~15% of fieldrange // TODO: this should be user-specifiable; it's not the same as M-S computation

  // if numZ == 1, evaluate at given percent along crystal
  if (numZ == 1) {
    delta = 1.0;
    minval = minval + (maxval - minval) * percent;  // only getting the single value at this percent along crystal
    sigma = 0.05; // 5% of fieldrange, because continuous sampling should be smaller? Maybe that's true, but still should probably be user-specifiable (TODO)
  }
  
  for (unsigned i = 0; i < numZ; i++)
  {
    double fieldval = minval + delta * i;

    // get new latent space coordinate for this field_val
    Eigen::RowVectorXd z_coord = model.getNewLatentSpaceValue(fieldval, sigma);  //<ctc> this will get the sigma from Model's form control item (see notes)

    // evaluate model at this coordinate
    Eigen::MatrixXd I = dspacex::ShapeOdds::evaluateModel(model, z_coord, false /*writeToDisk*/);
    
    // add result image to response
    Image image = Image::convertToImage(I, sample_image.getWidth(), sample_image.getHeight());
    // <ctc> our preprocessing pca models are sending images in row-major order, so try swapping width/height,
    // but Shireen's PCA models aren't. Pick one and be happy or specify it in the config.yaml
    //Image image = Image::convertToImage(I, sample_image.getHeight(), sample_image.getWidth());
    Json::Value imageObject = Json::Value(Json::objectValue);
    imageObject["width"] = image.getWidth();
    imageObject["height"] = image.getHeight();
    imageObject["rawData"] = base64_encode(reinterpret_cast<const unsigned char *>(&image.getConstRawData()[0]), image.getConstRawData().size());
    response["thumbnails"].append(imageObject);
    response["fieldvals"].append(fieldval);
  }

  response["msg"] = std::string("returning " + std::to_string(numZ) + " requested images predicted by model at crystal " + std::to_string(crystalid) + " of persistence level " + std::to_string(persistenceLevel));
}

// computes index of the requested (0-based) persistence level in this M-S complex
// (since there could be more actual persistence levels than those stored in the complex)
int Controller::getPersistenceLevelIdx(const unsigned desired_persistenceLevel, const dspacex::MSComplex &mscomplex) const
{
  int persistenceLevel_idx = desired_persistenceLevel -
    (m_currentTopoData->getMaxPersistenceLevel() - mscomplex.numPersistenceLevels() + 1);

  return persistenceLevel_idx;
}

/**
 * This returns a set of new samples (images) computed with the given ShapeOdds model for the
 * specified crystal at this persistence level using the latent space coordinates for each of
 * the original samples of model/crystal (each sample has a z_coord).
 *
 * Parameters: 
 *   datasetId - should already be loaded
 *   fieldname     - (e.g., one of the QoIs)
 *   persistenceId - persistence level of the M-S for this field of the dataset
 *   crystalId     - crystal of the given persistence level
 */
void Controller::fetchAllImagesForCrystal_Shapeodds(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");
  // maybeLoadModel(persistence,crystal); //<ctc> TODO: for speed, do not load models till their evaluation is requested

  // category of the passed fieldname (design param or qoi)
  Fieldtype category = Fieldtype(request["category"].asString());
  if (!category.valid()) return setError(response, "invalid category");

  // get requested persistence level
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  std::string fieldname = request["fieldname"].asString();
  int crystalid = request["crystalID"].asInt();
  std::cout << "fetchAllImagesForCrystal_Shapeodds: datasetId is "<<m_currentDatasetId<<", persistence is "<<persistenceLevel<<", crystalid is "<<crystalid<<std::endl;

  dspacex::MSComplex &mscomplex = m_currentDataset->getMSComplex(fieldname);
  int persistenceLevel_idx = getPersistenceLevelIdx(persistenceLevel, mscomplex);
  dspacex::Model &model(mscomplex.getModel(persistenceLevel_idx, crystalid).second);

  // TODO: cut and paste from above! ugh:
  // <ctc> we need to connect the dataset and its values more closely when reading a model, as the crystal's model should already know its fieldvalues
  // get the vector of values for the field
  Eigen::Map<Eigen::VectorXd> fieldvals = getFieldvalues(category, fieldname);
  if (!fieldvals.data())
    return setError(response, "Invalid fieldname or empty field");
  model.setFieldValues(fieldvals);

  //create images using the elements of this model's Z
  auto sample_indices(model.getSampleIndices());
  std::cout << "Testing all latent space variables computed for the " << sample_indices.size() << " samples in this model.\n";

  //z coords are sorted by fieldvalue in Model::setFieldValues
  for (auto sample: sample_indices)
  {
    // load thumbnail corresponding to this z_idx for comparison to evaluated model at same z_idx (they should be close)
    extern Controller *controller;
    if (!controller || !controller->m_currentDataset)
      throw std::runtime_error("ERROR: tried to access controller's current dataset, but it's NULL.");

    const Image& sample_image = controller->m_currentDataset->getThumbnail(sample.idx);
    unsigned sampleWidth = sample_image.getWidth(), sampleHeight = sample_image.getHeight();

    std::string outputBasepath(datapath + "/CantileverBeam_wclust_wraw/outimages"); //<ctc> todo: dataset_name or /debug/datasetname/outimages
    std::string outpath(outputBasepath + "/p" + std::to_string(persistenceLevel) + "-c" + std::to_string(crystalid) +
                        "-z" + std::to_string(sample.idx) + ".png");
    Eigen::MatrixXd I = dspacex::ShapeOdds::evaluateModel(model, model.getZCoord(sample.idx), false /*writeToDisk*/,
                                                            outpath, sample_image.getWidth(), sample_image.getHeight());


    //<ctc> simplify this to use the images passed in rather than re-generating (rename it to compareImages or something)
    float quality = dspacex::ShapeOdds::testEvaluateModel(model, model.getZCoord(sample.idx), persistenceLevel, crystalid, sample.idx, sample_image,
                                                          false /*writeToDisk*/, outputBasepath);

    // todo: is "quality" the correct term for comparison of generated image vs original?
    std::cout << "Quality of generation of image for model at persistence level "
              << persistenceLevel << ", crystalid " << crystalid << ": " << quality << std::endl;

    // add image to response
    Image image = Image::convertToImage(I, sample_image.getWidth(), sample_image.getHeight());
    // <ctc> our preprocessing pca models are sending images in row-major order, so try swapping width/height,
    // but Shireen's PCA models aren't. Pick one and be happy or specify it in the config.yaml
    //Image image = Image::convertToImage(I, sample_image.getHeight(), sample_image.getWidth());
    Json::Value imageObject = Json::Value(Json::objectValue);
    imageObject["width"] = image.getWidth();
    imageObject["height"] = image.getHeight();
    //imageObject["data"] = base64_encode(image.getConstData(), 4 * image.getWidth() * image.getHeight());  //<ctc> not used by client
    imageObject["rawData"] = base64_encode(reinterpret_cast<const unsigned char *>(&image.getConstRawData()[0]), image.getConstRawData().size());
    response["thumbnails"].append(imageObject);
    response["fieldvals"].append(sample.val);
  }

  response["msg"] = std::string("returning " + std::to_string(sample_indices.size()) + " images for model at crystal " + std::to_string(crystalid) + " of persistence level " + std::to_string(persistenceLevel));
}

// fetches the original sample images for the specified crystal (no MSComplex/crystal/model required)
void Controller::fetchCrystalOriginalSampleImages(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  // category of the passed fieldname (design param or qoi)
  Fieldtype category = Fieldtype(request["category"].asString());
  if (!category.valid()) return setError(response, "invalid category");

  // get requested persistence level
  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel)
    return setError(response, "invalid persistence level");

  std::string fieldname = request["fieldname"].asString();
  int crystalid = request["crystalID"].asInt();
  std::cout << "fetchCrystalOriginalSampleImages: datasetId is "<<m_currentDatasetId<<", persistence is "<<persistenceLevel<<", crystalid is "<<crystalid<<std::endl;

  // get the vector of values for the field
  Eigen::Map<Eigen::VectorXd> fieldvals = getFieldvalues(category, fieldname);
  if (!fieldvals.data())
    return setError(response, "Invalid fieldname or empty field");

  FortranLinalg::DenseVector<int> &crystal_partition(m_currentVizData->getCrystalPartitions(persistenceLevel));
  Eigen::Map<Eigen::VectorXi> partitions(crystal_partition.data(), crystal_partition.N());
  std::vector<dspacex::Model::ValueIndexPair> fieldvalues_and_indices;
  for (unsigned i = 0; i < partitions.size(); i++)
  {
    if (partitions(i) == crystalid)
    {
      dspacex::Model::ValueIndexPair sample;
      sample.idx = i;
      sample.val = fieldvals(i);
      fieldvalues_and_indices.push_back(sample);
    }
  }

  // sort by increasing fieldvalue
  std::sort(fieldvalues_and_indices.begin(), fieldvalues_and_indices.end(), dspacex::Model::ValueIndexPair::compare);
  std::cout << "Returning images for the " << fieldvalues_and_indices.size() << " samples in this crystal.\n";

  for (auto sample: fieldvalues_and_indices)
  {
    // load thumbnail corresponding to this z_idx
    const Image& sample_image = m_currentDataset->getThumbnail(sample.idx);
    unsigned sampleWidth = sample_image.getWidth(), sampleHeight = sample_image.getHeight();

    // add image to response
    Json::Value imageObject = Json::Value(Json::objectValue);
    imageObject["width"] = sample_image.getWidth();
    imageObject["height"] = sample_image.getHeight();
    imageObject["rawData"] = base64_encode(reinterpret_cast<const unsigned char *>(&sample_image.getConstRawData()[0]), sample_image.getConstRawData().size());
    response["thumbnails"].append(imageObject);
    response["fieldvals"].append(sample.val);
  }

  response["msg"] = std::string("returning " + std::to_string(fieldvalues_and_indices.size()) + " images for crystal " + std::to_string(crystalid) + " of persistence level " + std::to_string(persistenceLevel));
}

/**
 * This fetches the QoI, design params, and image for a given latent space produced by the SharedGP library.
 * TODO <ctc>: delete this next as it will not be used (just trying to keep commits separated)
 */
void Controller::fetchAllForLatentSpaceUsingSharedGP(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size())
    return setError(response, "invalid datasetId");
  int qoi = request["qoi"].asInt();
  std::cout << "fetchAllForLatentSpaceUsingSharedGP: datasetId is "<<datasetId<<", qoi is "<<qoi<<std::endl;
  if (qoi < 0)
    return setError(response, "invalid parameter");

  std::cout << "TODO: return something :-)\n";
  response["msg"] = std::string("need to return desired QoI, DPs, and an image for the given shared_gp latent space");
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

// getFieldvalues
// 
// returns Eigen::Map wrapping the vector of values for a given field
const Eigen::Map<Eigen::VectorXd> Controller::getFieldvalues(Fieldtype type, const std::string &name)
{
  if (type == Fieldtype::DesignParameter)
  {
    auto parameters = m_currentDataset->getParameterNames();
    auto result = std::find(std::begin(parameters), std::end(parameters), name);
    if (result == std::end(parameters)) 
      return Eigen::Map<Eigen::VectorXd>(NULL, 0);

    int index = std::distance(parameters.begin(), result);
    FortranLinalg::DenseVector<Precision> values = m_currentDataset->getParameterVector(index);
    return Eigen::Map<Eigen::VectorXd>(values.data(), values.N());
  }
  else if (type == Fieldtype::QoI)
  {
    auto qois = m_currentDataset->getQoiNames();
    auto result = std::find(std::begin(qois), std::end(qois), name);
    if (result == std::end(qois)) 
      return Eigen::Map<Eigen::VectorXd>(NULL, 0);

    int index = std::distance(qois.begin(), result);
    FortranLinalg::DenseVector<Precision> values = m_currentDataset->getQoiVector(index);
    return Eigen::Map<Eigen::VectorXd>(values.data(), values.N());
  }
  return Eigen::Map<Eigen::VectorXd>(NULL, 0);
}

/**
 * Check to see if parameters of request are valid to process data
 */
bool Controller::verifyProcessDataParams(Fieldtype category, std::string fieldname, int knn, int curvepoints,
                                         double sigma, double smoothing, bool addnoise, int depth,
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
  
  // M-S computation params
  if (knn < 0) {
    setError(response, "knn must be >= 0");
    return false;
  } else if (sigma < 0) {
    setError(response, "sigma must be >= 0");
    return false;
  } else if (smoothing < 0) {
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
  auto curvepoints = request.isMember("curvepoints") ? request["curvepoints"].asInt()            : m_currentNumCurvepoints;
  auto sigma       = request.isMember("sigma")       ? request["sigma"].asFloat()                : m_currentSigma;
  auto smoothing   = request.isMember("smooth")      ? request["smooth"].asFloat()               : m_currentSmoothing;
  auto addnoise    = request.isMember("noise")       ? request["noise"].asBool()                 : m_currentAddNoise;
  auto depth       = request.isMember("depth")       ? request["depth"].asInt()                  : m_currentPersistenceDepth;
  auto normalize   = request.isMember("normalize")   ? request["normalize"].asInt()              : m_currentNormalize;  

  if (!verifyProcessDataParams(category, fieldname, knn, curvepoints, sigma, smoothing, addnoise, depth, normalize, response))
    return false; // response will contain the error

  if (!processData(category, fieldname, knn, curvepoints, sigma, smoothing, addnoise, depth, normalize)) {
    setError(response, "failed to process data");
    return false;
  }

  return true;
}

/// Avoid regeneration of data if parameters haven't changed
bool Controller::processDataParamsChanged(Fieldtype category, std::string fieldname, int knn, int num_samples,
                                          double sigma, double smoothing, bool add_noise,
                                          int num_persistences, bool normalize) {
  return !(m_currentCategory        == category &&
           m_currentField           == fieldname &&
           m_currentKNN             == knn &&
           m_currentNumCurvepoints == num_samples &&
           m_currentSigma           == sigma &&
           m_currentSmoothing       == smoothing &&
           m_currentAddNoise        == add_noise &&
           m_currentPersistenceDepth == num_persistences &&
           m_currentNormalize       == normalize);
}

///display the min, max, avg, var, sdv for this array
template<typename T>
void displayFieldStats(const T& arr) {
  auto minval(arr.minCoeff());
  auto maxval(arr.maxCoeff());
  auto meanval(arr.mean());
  std::cout << "\tmin: " << minval << std::endl;
  std::cout << "\tmax: " << maxval << std::endl;
  std::cout << "\tavg: " << meanval << std::endl;

  //compute variance
  {
    Eigen::Matrix<Precision, Eigen::Dynamic, 1> copyvals(arr);
    copyvals.array() -= meanval;
    copyvals.array() = copyvals.array().square();
    auto variance(copyvals.sum() / (copyvals.size()-1));
    std::cout << "\tvar: " << variance << std::endl;
    std::cout << "\tsdv: " << sqrt(variance) << std::endl;
  }
}

/**
 * Checks if the requested dataset has been processed. If not, processes the data.
 *
 * Called by maybeProcessData which validates parameters.
 *
 * Returns true if no need to process or process successful, 
 * false if processing failed.
 */
bool Controller::processData(Fieldtype category, std::string fieldname, int knn, int num_samples,
                             double sigma, double smoothing, bool add_noise, int num_persistences,
                             bool normalize) {
  if (m_currentTopoData && !processDataParamsChanged(category, fieldname, knn, num_samples, sigma, smoothing,
                                                     add_noise, num_persistences, normalize))
    return true;

  std::cout << "computing nnmscomplex for fieldname: " << fieldname << "..." << std::endl;

  // get the vector of values for the requested field
  Eigen::Map<Eigen::Matrix<Precision, Eigen::Dynamic, 1>> fieldvals = getFieldvalues(category, fieldname);
  if (!fieldvals.data()) {
    std::cerr << "Controller::processData failed: invalid fieldname or empty field.\n";
    return false;
  }

  // normalize fieldvals if requested
  if (normalize) {
    auto showStats(true);
    if (showStats) {
      std::cout << "Raw field stats:\n";
      displayFieldStats(fieldvals);
    }

    // Scale normalize so that all values are in range [0,1]: for each member X: X = (X - min) / (max - min).
    auto minval(fieldvals.minCoeff());
    auto maxval(fieldvals.maxCoeff());
    fieldvals.array() -= minval;
    fieldvals.array() /= (maxval - minval);

    if (showStats) {
      std::cout << "Scale-normalized field stats:\n";
      displayFieldStats(fieldvals);
    }
  }

  // clear current computation results
  m_currentVizData = nullptr;
  m_currentTopoData = nullptr;

  // load or generate the distance matrix 
  if (m_currentDataset->hasDistanceMatrix()) {
    m_currentDistanceMatrix = m_currentDataset->getDistanceMatrix();
  } else if (m_currentDataset->hasSamplesMatrix()) {
    auto samplesMatrix = m_currentDataset->getSamplesMatrix();
    m_currentDistanceMatrix = HDProcess::computeDistanceMatrix(samplesMatrix);
  } else {
    std::cerr << "processData failed: no distance matrix or samplesMatrix available\n";
    return false;
  }

  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
  try {
    std::shared_ptr<HDProcessResult> result(
      genericProcessor.processOnMetric(m_currentDistanceMatrix,
                                       FortranLinalg::DenseVector<Precision>(fieldvals.size(), fieldvals.data()),
                                       knn,              /* k nearest neighbors to consider */
                                       num_samples,      /* points along each crystal regression curve */
                                       num_persistences, /* -1 generates all of 'em */
                                       add_noise,        /* adds very slight noise to field values */
                                       sigma,            /* soften crystal regression curves */
                                       smoothing));      /* smooth topology */
    m_currentVizData.reset(new SimpleHDVizDataImpl(result));
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
  m_currentKNN = knn;
        m_currentNumCurvepoints = num_samples;
  m_currentSigma = sigma;
  m_currentSmoothing = smoothing;
  m_currentAddNoise = add_noise;
        m_currentPersistenceDepth = num_persistences;
  m_currentNormalize = normalize;
  
  return true;
}
