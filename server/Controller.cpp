#include <base64/base64.h>
#include <boost/filesystem.hpp>
#include "Controller.h"
#include "DatasetLoader.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/DenseVector.h"
#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "hdprocess/HDGenericProcessor.h"
#include "hdprocess/LegacyTopologyDataImpl.h"
#include "hdprocess/SimpleHDVizDataImpl.h"
#include "hdprocess/TopologyData.h"
#include <jsoncpp/json/json.h>
#include "precision/Precision.h"
#include "serverlib/wst.h"
#include "util/DenseVectorSample.h"
#include "util/csv/loaders.h"
#include "util/utils.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <fstream>
#include <string>

// Simplify namespace access to _1, _2 for std::bind parameter binding.
using namespace std::placeholders;

// Maximum number of directories from root path to seek config.yaml files
const int MAX_DATASET_DEPTH = 3;


Controller::Controller() {
  configureCommandHandlers();
  configureAvailableDatasets();
}

/**
 * Construct map of command names to command handlers.
 */
void Controller::configureCommandHandlers() {
  m_commandMap.insert({"fetchDatasetList", std::bind(&Controller::fetchDatasetList, this, _1, _2)});
  m_commandMap.insert({"fetchDataset", std::bind(&Controller::fetchDataset, this, _1, _2)});
  m_commandMap.insert({"fetchKNeighbors", std::bind(&Controller::fetchKNeighbors, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleDecomposition", std::bind(&Controller::fetchMorseSmaleDecomposition, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmalePersistence", std::bind(&Controller::fetchMorseSmalePersistence, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmalePersistenceLevel", std::bind(&Controller::fetchMorseSmalePersistenceLevel, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleCrystal", std::bind(&Controller::fetchMorseSmaleCrystal, this, _1, _2)});
  m_commandMap.insert({"fetchLayoutForPersistenceLevel", std::bind(&Controller::fetchLayoutForPersistenceLevel, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleLayoutForPersistenceLevel", std::bind(&Controller::fetchMorseSmaleLayoutForPersistenceLevel, this, _1, _2)});
  m_commandMap.insert({"fetchParameter", std::bind(&Controller::fetchParameter, this, _1, _2)});
  m_commandMap.insert({"fetchQoi", std::bind(&Controller::fetchQoi, this, _1, _2)});
  m_commandMap.insert({"fetchThumbnails", std::bind(&Controller::fetchThumbnails, this, _1, _2)});
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
  } catch(const boost::filesystem::filesystem_error& e) {
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
      std::string name  = DatasetLoader::getDatasetName(path.string());
      std::cout << "  " << path.string() << " --> " << name << std::endl;
      m_availableDatasets.push_back({name, path.string()});
    } catch(const std::exception &error) {
      std::cout << "  " << path.string() << " --> " << "[ FAILED TO LOAD ]" << std::endl;
    }
  }
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
    wst_sendText(wsi, const_cast<char*>(text.c_str()));
  } catch (const std::exception &e) {
    std::cerr << "Command Execution Error: " << e.what() << std::endl;
    // TODO: Send back an error message.
  }
}

/**
 * Handle the command to fetch list of available datasets.
 */
void Controller::fetchDatasetList(const Json::Value &request, Json::Value &response) {
  response["datasets"] = Json::Value(Json::arrayValue);

  for (size_t i=0; i < m_availableDatasets.size(); i++) {
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
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  maybeLoadDataset(datasetId);

  response["datasetId"] = datasetId;
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
void Controller::fetchKNeighbors(
    const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  // TODO: Don't reprocess data if previous commands already did so.
  int n = m_currentDataset->getDistanceMatrix().N();
  auto KNN = FortranLinalg::DenseMatrix<int>(k, n);
  auto KNND = FortranLinalg::DenseMatrix<Precision>(k, n);
  Distance<Precision>::findKNN(m_currentDataset->getDistanceMatrix(), KNN, KNND);

  response["datasetId"] = datasetId;
  response["k"] = k;
  response["graph"] = Json::Value(Json::arrayValue);
  for (unsigned i = 0; i < KNN.M(); i++) {
    Json::Value row = Json::Value(Json::arrayValue);
    response["graph"].append(row);
    for (unsigned int j = 0; j < KNN.N(); j++) {
      response["graph"][i].append(KNN(i,j));
    }
  }
}


/**
 * Handle the command to fetch the morse smale persistence levels of a dataset.
 */
void Controller::fetchMorseSmalePersistence(
    const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }

  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();

  response["datasetId"] = datasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["minPersistenceLevel"] = minLevel;
  response["maxPersistenceLevel"] = maxLevel;
  response["complexSizes"] = Json::Value(Json::arrayValue);
  for (int level = minLevel; level <= maxLevel; level++) {
    MorseSmaleComplex *complex = m_currentTopoData->getComplex(level);
    int size = complex->getCrystals().size();
    response["complexSizes"].append(size);
  }
}

/**
 * Handle the command to fetch the crystal complex composing a morse smale persistence level.
 */
void Controller::fetchMorseSmalePersistenceLevel(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();

  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  MorseSmaleComplex *complex = m_currentTopoData->getComplex(persistenceLevel);

  response["datasetId"] = datasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["k"] = k;
  response["persistenceLevel"] = persistenceLevel;
  response["complex"] = Json::Value(Json::objectValue);
  response["complex"]["crystals"] = Json::Value(Json::arrayValue);

  for (unsigned int c = 0; c < complex->getCrystals().size(); c++) {
    Crystal *crystal = complex->getCrystals()[c];
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
void Controller::fetchMorseSmaleCrystal(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();

  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  MorseSmaleComplex *complex = m_currentTopoData->getComplex(persistenceLevel);

  int crystalId = request["crystalId"].asInt();
  if (crystalId < 0 || crystalId >= complex->getCrystals().size()) {
    // TODO: Send back an error message. Invalid CrystalId.
  }

  Crystal *crystal = complex->getCrystals()[crystalId];

  response["datasetId"] = datasetId;
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
 * Handle the command to fetch the full morse smale decomposition of a dataset.
 */
void Controller::fetchMorseSmaleDecomposition(
    const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }


  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();

  response["datasetId"] = datasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["minPersistenceLevel"] = minLevel;
  response["maxPersistenceLevel"] = maxLevel;
  response["complexes"] = Json::Value(Json::arrayValue);
  for (unsigned int level = minLevel; level <= maxLevel; level++) {
    MorseSmaleComplex *complex = m_currentTopoData->getComplex(level);
    Json::Value complexObject(Json::objectValue);
    complexObject["crystals"] = Json::Value(Json::arrayValue);
    for (unsigned int c = 0; c < complex->getCrystals().size(); c++) {
      Crystal *crystal = complex->getCrystals()[c];
      Json::Value crystalObject(Json::objectValue);
      crystalObject["minIndex"] = crystal->getMinSample();
      crystalObject["maxIndex"] = crystal->getMaxSample();
      crystalObject["sampleIndexes"] = Json::Value(Json::arrayValue);
      for (unsigned int i = 0; i < crystal->getAllSamples().size(); i++) {
        unsigned int index = crystal->getAllSamples()[i];
        crystalObject["sampleIndexes"].append(index);
      }
      complexObject["crystals"].append(crystalObject);
    }
    // TODO: Add adjacency to the complex json object.
    response["complexes"].append(complexObject);
  }
}

/**
 * This fetches the graph embedding layout for a given persistence level
 * @param request
 * @param response
 */
void Controller::fetchLayoutForPersistenceLevel(
    const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();

  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  // TODO:  Modify logic to return layout based on chosen layout type.
  //        For now, only send back embeddings provided with the dataset.
  if (m_currentDataset->numberOfEmbeddings() > 0) {
    auto embedding = m_currentDataset->getEmbeddingMatrix(0);
    auto name = m_currentDataset->getEmbeddingNames()[0];

    // TODO: Factor out a normalizing routine.
    float minX = embedding(0,0);
    float maxX = embedding(0,0);
    float minY = embedding(0,1);
    float maxY = embedding(0,1);
    for (int i=0; i < embedding.M(); i++) {
      minX = embedding(i, 0) < minX ? embedding(i, 0) : minX;
      maxX = embedding(i, 0) > maxX ? embedding(i, 0) : maxX;
      minY = embedding(i, 1) < minY ? embedding(i, 1) : minY;
      maxY = embedding(i, 1) > maxY ? embedding(i, 1) : maxY;
    }
    for (int i=0; i < embedding.M(); i++) {
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

void Controller::fetchMorseSmaleLayoutForPersistenceLevel(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();

  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  // Get points for regression line
  std::vector<FortranLinalg::DenseMatrix<Precision>> layout = m_currentVizData->getLayout(HDVizLayout::ISOMAP, persistenceLevel);
//  double points[m_currentVizData->getNumberOfSamples()+2][3];

  // For each crystal
  response["crystals"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < m_currentVizData->getCrystals(persistenceLevel).N(); i++) {
    // Get layout for each crystal
    Json::Value crystalObject(Json::objectValue);
    crystalObject["id"] = i;
    crystalObject["regressionPoints"] = Json::Value(Json::arrayValue);
    for (unsigned int n = 0; n < layout[i].N(); ++n) {
      auto row = Json::Value(Json::arrayValue);
      for(unsigned int m = 0; m < layout[i].M(); ++m) {
        row.append(layout[i](m, n));
      }
      row.append(m_currentVizData->getMeanNormalized(persistenceLevel)[i](n));
      crystalObject["regressionPoints"].append(row);
    }


    // TODO figure out what this is for - copied from DisplayTubes.cpp I am not sure we need it
//    auto row = Json::Value(Json::arrayValue);
//    for (unsigned int j = 0; j < 3; ++j) {
//      points[0][j] = points[1][j] + points[2][j] - points[1][j];
//      points[m_currentVizData->getNumberOfSamples() + 1][j] =
//          points[m_currentVizData->getNumberOfSamples()][j] +
//          points[m_currentVizData->getNumberOfSamples()][j] -
//          points[m_currentVizData->getNumberOfSamples() - 1][j];
//    }

    response["crystals"].append(crystalObject);
  }
}
/**
 * Handle the command to fetch an array of a named parameter.
 */
void Controller::fetchParameter(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  maybeLoadDataset(datasetId);

  std::string parameterName = request["parameterName"].asString();
  auto parameters = m_currentDataset->getParameterNames();
  auto result = std::find(std::begin(parameters), std::end(parameters), parameterName);
  if (result == std::end(parameters)) {
    // TODO: Send back an error message. Invalid Qoi Name.
  }

  int parameterIndex = result - std::begin(parameters);
  auto parameterVector = m_currentDataset->getParameterVector(parameterIndex);

  response["parameterName"] = parameterName;
  response["parameter"] = Json::Value(Json::arrayValue);
  for (int i = 0; i < parameterVector.N(); i++) {
    response["parameter"].append(parameterVector(i));
  }
}

/**
 * Handle the command to fetch an array of a named QoI
 */
void Controller::fetchQoi(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  maybeLoadDataset(datasetId);

  std::string qoiName = request["qoiName"].asString();
  auto qois = m_currentDataset->getQoiNames();
  auto result = std::find(std::begin(qois), std::end(qois), qoiName);
  if (result == std::end(qois)) {
    // TODO: Send back an error message. Invalid Qoi Name.
  }

  int qoiIndex = result - std::begin(qois);
  auto qoiVector = m_currentDataset->getQoiVector(qoiIndex);

  response["qoiName"] = qoiName;
  response["qoi"] = Json::Value(Json::arrayValue);
  for (int i = 0; i < qoiVector.N(); i++) {
    response["qoi"].append(qoiVector(i));
  }
}


/**
 * Handle the command to fetch sample image thumbnails if available.
 */
void Controller::fetchThumbnails(
    const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  maybeLoadDataset(datasetId);

  auto thumbnails = m_currentDataset->getThumbnails();

  response["thumbnails"] = Json::Value(Json::arrayValue);
  for (auto image : thumbnails) {
    Json::Value imageObject = Json::Value(Json::objectValue);
    imageObject["width"] = image.getWidth();
    imageObject["height"] = image.getHeight();
    imageObject["data"] = base64_encode(
            reinterpret_cast<const unsigned char*>(image.getData()),
            4*image.getWidth()*image.getHeight());
    imageObject["rawData"] = base64_encode(
            reinterpret_cast<const unsigned char*>(&image.getRawData()[0]),
            image.getRawData().size());
    response["thumbnails"].append(imageObject);
  }
}

/**
 * Checks if the requested dataset is loaded.
 * If not, loads the dataset and sets state.
 */
void Controller::maybeLoadDataset(int datasetId) {
  if (datasetId == m_currentDatasetId) {
    return;
  }

  if (m_currentDataset) {
    delete m_currentDataset;
    m_currentDataset = nullptr;
  }
  if (m_currentVizData) {
    delete m_currentVizData;
    m_currentVizData = nullptr;
  }
  if (m_currentTopoData) {
    delete m_currentTopoData;
    m_currentTopoData = nullptr;
  }
  m_currentK = -1;

  std::string configPath = m_availableDatasets[datasetId].second;
  m_currentDataset = DatasetLoader::loadDataset(configPath);
  std::cout << m_currentDataset->getName() << " dataset loaded." << std::endl;

  m_currentDatasetId = datasetId;
}

/**
 * Checks if the requested dataset has been processed
 * with the chosen k value. If not, processes the data.
 */
void Controller::maybeProcessData(int k) {
  if (k == m_currentK) {
    return;
  }

  if (m_currentProcessResult) {
    delete m_currentProcessResult;
    m_currentProcessResult = nullptr;
  }
  if (m_currentVizData) {
    delete m_currentVizData;
    m_currentVizData = nullptr;
  }
  if (m_currentTopoData) {
    delete m_currentTopoData;
    m_currentTopoData = nullptr;
  }

  if (m_currentDataset->hasDistanceMatrix()) {
    m_currentDistanceMatrix = m_currentDataset->getDistanceMatrix();
  } else if (m_currentDataset->hasSamplesMatrix()) {
    auto samplesMatrix = m_currentDataset->getSamplesMatrix();
    m_currentDistanceMatrix = HDProcess::computeDistanceMatrix(samplesMatrix);
  } else {
    std::runtime_error("No distance matrix or samplesMatrix available.");
  }



  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
  m_currentK = k;

  // TODO: Provide mechanism to select QoI used.
  // TODO: Expose processing parameters to function interface.
  try {
    m_currentProcessResult = genericProcessor.processOnMetric(
        m_currentDistanceMatrix,
        m_currentDataset->getQoiVector(0),
        m_currentK  /* knn */,
        25        /* samples */,
        20        /* persistence */,
        true      /* random */,
        0.25      /* sigma */,
        0         /* smooth */);
    m_currentVizData = new SimpleHDVizDataImpl(m_currentProcessResult);
    m_currentTopoData = new LegacyTopologyDataImpl(m_currentVizData);
  } catch (const char *err) {
    std::cerr << err << std::endl;
    // TODO: Return Error Message.
  }
}
