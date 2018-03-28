#include "Controller.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "HDGenericProcessor.h"
#include <jsoncpp/json.h>
#include "LegacyTopologyDataImpl.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "Precision.h"
#include "SimpleHDVizDataImpl.h"
#include "TopologyData.h"
#include "util/DenseVectorSample.h"
#include "util/csv/loaders.h"
#include "wst.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <string>
#include <thread>

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#define strtok_r strtok_s
#endif

using namespace std::placeholders;

Controller::Controller() {
  m_commandMap.insert({"fetchDatasetList", std::bind(&Controller::fetchDatasetList, this, _1, _2, _3)});
  m_commandMap.insert({"fetchDataset", std::bind(&Controller::fetchDataset, this, _1, _2, _3)});
  m_commandMap.insert({"fetchKNeighbors", std::bind(&Controller::fetchKNeighbors, this, _1, _2, _3)});
  m_commandMap.insert({"fetchMorseSmaleDecomposition", std::bind(&Controller::fetchMorseSmaleDecomposition, this, _1, _2, _3)});
  m_commandMap.insert({"fetchMorseSmalePersistence", std::bind(&Controller::fetchMorseSmalePersistence, this, _1, _2, _3)});
  m_commandMap.insert({"fetchMorseSmalePersistenceLevel", std::bind(&Controller::fetchMorseSmalePersistenceLevel, this, _1, _2, _3)});
  m_commandMap.insert({"fetchMorseSmaleCrystal", std::bind(&Controller::fetchMorseSmaleCrystal, this, _1, _2, _3)});
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

    auto command = m_commandMap[commandName];
    if (command) {
      command(wsi, messageId, request);
    } else {
      std::cout << "Error: Unrecognized Command: " << commandName << std::endl;
    }

  } catch (const std::exception &e) {
    std::cerr << "Command Parsing Error." << e.what() << std::endl;
    // TODO: Send back an error message.
  }
}

/**
 * Handle the command to fetch list of available datasets.
 */
void Controller::fetchDatasetList(void *wsi, int messageId, const Json::Value &request) {
  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasets"] = Json::Value(Json::arrayValue);

  for (size_t i=0; i < m_availableDatasets.size(); i++) {
    Json::Value object = Json::Value(Json::objectValue);
    object["id"] = static_cast<int>(i);
    object["name"] = m_availableDatasets[i].first;
    response["datasets"].append(object);
  }

  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
}

/**
 * Handle the command to load and fetch details of a dataset.
 */
void Controller::fetchDataset(void *wsi, int messageId, const Json::Value &request) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  maybeLoadDataset(datasetId);

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasetId"] = datasetId;
  response["name"] = m_currentDataset->getName();
  response["numberOfSamples"] = m_currentDataset->numberOfSamples();
  response["qoiNames"] = Json::Value(Json::arrayValue);

  for (std::string qoiName : m_currentDataset->getQoiNames()) {
    response["qoiNames"].append(qoiName);
  }

  response["attributeNames"] = Json::Value(Json::arrayValue);
  for (std::string attributeName : m_currentDataset->getAttributeNames()) {
    response["attributeNames"].append(attributeName);
  }


  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
}

/**
 * Handle the command to fetch the k-nearest-neighbor graph of a dataset.
 */
void Controller::fetchKNeighbors(void *wsi, int messageId, const Json::Value &request) {
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
  Distance<Precision>::findKNN(
      m_currentDataset->getDistanceMatrix(), KNN, KNND);


  Json::Value response(Json::objectValue);
  response["id"] = messageId;
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

  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
}


/**
 * Handle the command to fetch the morse smale persistence levels of a dataset.
 */
void Controller::fetchMorseSmalePersistence(void *wsi, int messageId, const Json::Value &request) {
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

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
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

  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
}

/**
 * Handle the command to fetch the crystal complex composing a morse smale persistence level.
 */
void Controller::fetchMorseSmalePersistenceLevel(void *wsi, int messageId, const Json::Value &request) {
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

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasetId"] = datasetId;
  response["decompositionMode"] = "Morse-Smale";
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

  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
}

/**
 * Handle the command to fetch the details of a single crystal in a persistence level.
 */
void Controller::fetchMorseSmaleCrystal(void *wsi, int messageId, const Json::Value &request) {
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

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
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

  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
}

/**
 * Handle the command to fetch the full morse smale decomposition of a dataset.
 */
void Controller::fetchMorseSmaleDecomposition(void *wsi, int messageId, const Json::Value &request) {
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

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
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

  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
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

  auto loadDataset = m_availableDatasets[datasetId].second;
  m_currentDataset = loadDataset();
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

  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
  m_currentK = k;

  // TODO: Provide mechanism to select QoI used.
  // TODO: Expose processing parameters to function interface.
  try {
    m_currentProcessResult = genericProcessor.processOnMetric(
        m_currentDataset->getDistanceMatrix(),
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


/**
 * Construct list of available datasets.
 */
void Controller::configureAvailableDatasets() {
  m_availableDatasets.push_back({"Concrete", std::bind(&Controller::loadConcreteDataset, this)});
  m_availableDatasets.push_back({ "Crimes",  std::bind(&Controller::loadCrimesDataset,   this)});
  m_availableDatasets.push_back({"Gaussian", std::bind(&Controller::loadGaussianDataset, this)});
  m_availableDatasets.push_back({"Colorado", std::bind(&Controller::loadColoradoDataset, this)});
}


Dataset* Controller::loadConcreteDataset() {
  std::string datasetName = "Concrete";
  std::string path = "../../examples/concrete/";
  std::string geometryFile = "Geom.data.hdr";
  std::string functionFile = "Function.data.hdr";
  std::string namesFile = "names.txt";

  auto x = FortranLinalg::LinalgIO<Precision>::readMatrix(path + geometryFile);
  auto y = FortranLinalg::LinalgIO<Precision>::readVector(path + functionFile);
  auto distances = computeDistanceMatrix(x);

  Dataset::Builder builder;
  Dataset *dataset = builder.withDistanceMatrix(distances)
                            .withQoi("function", y)
                            .withName(datasetName)
                            .build();
  std::cout << datasetName << " dataset loaded." << std::endl;
  return dataset;
}

Dataset* Controller::loadCrimesDataset() {
  std::string datasetName = "Crime";
  std::string path = "../../examples/crimes/";
  std::string geometryFile = "Geom.data.hdr";
  std::string functionFile = "Function.data.hdr";
  std::string namesFile = "names.txt";

  auto x = FortranLinalg::LinalgIO<Precision>::readMatrix(path + geometryFile);
  auto y = FortranLinalg::LinalgIO<Precision>::readVector(path + functionFile);
  auto distances = computeDistanceMatrix(x);

  Dataset::Builder builder;
  Dataset *dataset = builder.withDistanceMatrix(distances)
                            .withQoi("function", y)
                            .withName(datasetName)
                            .build();
  std::cout << datasetName << " dataset loaded." << std::endl;
  return dataset;
}

Dataset* Controller::loadGaussianDataset() {
  std::string datasetName = "Gaussian";
  std::string path = "../../examples/gaussian2d/";
  std::string geometryFile = "Geom.data.hdr";
  std::string functionFile = "Function.data.hdr";

  auto x = FortranLinalg::LinalgIO<Precision>::readMatrix(path + geometryFile);
  auto y = FortranLinalg::LinalgIO<Precision>::readVector(path + functionFile);
  auto distances = computeDistanceMatrix(x);

  Dataset::Builder builder;
  Dataset *dataset = builder.withDistanceMatrix(distances)
                            .withQoi("function", y)
                            .withName(datasetName)
                            .build();
  std::cout << datasetName << " dataset loaded." << std::endl;
  return dataset;
}

Dataset* Controller::loadColoradoDataset() {
  std::string datasetName = "Colorado";
  std::string path = "../../examples/truss/";
  std::string imageFolder = "images/";
  std::string distancesFile = "distances.csv";
  std::string maxStressFile = "max_stress.csv";
  std::string tsneLayoutFile = "tsne-layout.csv";

  auto distances = HDProcess::loadCSVMatrix(path + distancesFile);
  auto y = HDProcess::loadCSVColumn(path + maxStressFile);
  auto tSneLayout = HDProcess::loadCSVMatrix(path + tsneLayoutFile);

  Dataset::Builder builder;
  Dataset *dataset = builder.withDistanceMatrix(distances)
                            .withQoi("max-stress", y)
                            .withName("Colorado")
                            .build();
  std::cout << datasetName << " dataset loaded." << std::endl;
  return dataset;
}

// TODO: Move into a utility.
FortranLinalg::DenseMatrix<Precision> Controller::computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x) {
  std::vector<DenseVectorSample*> samples;
  for (int j = 0; j < x.N(); j++) {
    FortranLinalg::DenseVector<Precision> vector(x.M());
    for (int i = 0; i < x.M(); i++) {
      vector(i) = x(i, j);
    }
    DenseVectorSample *sample = new DenseVectorSample(vector);
    samples.push_back(sample);
  }

  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
  DenseVectorEuclideanMetric metric;
  return genericProcessor.computeDistances(samples, metric);
}
