#include "Dataset.h"
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
#include <string>
#include <thread>

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#define strtok_r strtok_s
#endif


void configureAvailableDatasets();
Dataset* loadConcreteDataset();
Dataset* loadCrimesDataset();
Dataset* loadGaussianDataset();
Dataset* loadColoradoDataset();

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

FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x);

typedef Dataset* (*LoadFunction)(void);
std::vector<std::pair<std::string, LoadFunction>> availableDatasets;

// TODO: Refactor current state out into a separate component.
std::vector<Dataset*> datasets;
Dataset *currentDataset = nullptr;
int currentDatasetId = -1;
int currentK = -1;
HDProcessResult *currentProcessResult = nullptr;
HDVizData *currentVizData = nullptr;
TopologyData *currentTopoData = nullptr;



const int kDefaultPort = 7681;

int main(int argc, char *argv[])
{
  if (argc > 2) {
    printf("\n Usage: dSpaceX [port]\n\n");
    return 1;
  }
  
  int port = kDefaultPort;
  if (argc == 2) {
    port = atoi(argv[1]);
  }

  try {
    configureAvailableDatasets();
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  // Create the Web Socket Transport context.
  wstContext *cntxt = wst_createContext();
  if (!cntxt) {
    std::cout << "Failed to create wstContext." << std::endl;
    return -1;
  }

  // Start listening for connections.  
  int status = wst_startServer(port, nullptr, nullptr, nullptr, 0, cntxt);
  if (status != 0) {
    std::cout << "FATAL: wst_startServer returned failure." << std::endl;
    return -1;
  }

  /** 
   * If browser command supplied, open the url. For example on a Mac:
   * setenv DSX_START "open -a /Applications/Firefox.app ../client/dSpaceX.html"
   */
  char *startapp = getenv("DSX_START");
  if (startapp) {
    system(startapp);
  }
  
  // Keep server alive.
  while (wst_statusServer(0)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));    
  }
  wst_cleanupServers();
  return 0;
}


extern "C" void browserData(void *wsi, void *data)
{
  // TODO:  Implement
}

extern "C" void browserText(void *wsi, char *text, int lena)
{
  Json::Reader reader;
  Json::Value request;

  try {
    reader.parse(std::string(text), request);
    int messageId = request["id"].asInt();
    std::string commandName = request["name"].asString();
    std::cout << "[" << messageId << "] " << commandName << std::endl;

    if (commandName == "fetchDatasetList") {
      fetchDatasetList(wsi, messageId, request);
    } else if (commandName == "fetchDataset") {
      fetchDataset(wsi, messageId, request);
    } else if (commandName == "fetchKNeighbors") {
      fetchKNeighbors(wsi, messageId, request);
    } else if (commandName == "fetchMorseSmaleDecomposition") {
      fetchMorseSmaleDecomposition(wsi, messageId, request);
    } else if (commandName == "fetchMorseSmalePersistence") {
      fetchMorseSmalePersistence(wsi, messageId, request);
    } else if (commandName == "fetchMorseSmalePersistenceLevel") {
      fetchMorseSmalePersistenceLevel(wsi, messageId, request);
    } else if (commandName == "fetchMorseSmaleCrystal") {
      fetchMorseSmaleCrystal(wsi, messageId, request);
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
void fetchDatasetList(void *wsi, int messageId, const Json::Value &request) {
  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasets"] = Json::Value(Json::arrayValue);

  for (size_t i=0; i < availableDatasets.size(); i++) {
    Json::Value object = Json::Value(Json::objectValue);
    object["id"] = static_cast<int>(i);
    object["name"] = availableDatasets[i].first;
    response["datasets"].append(object);
  }

  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
}

/**
 * Handle the command to load and fetch details of a dataset.
 */
void fetchDataset(void *wsi, int messageId, const Json::Value &request) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  maybeLoadDataset(datasetId);

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasetId"] = datasetId;
  response["name"] = currentDataset->getName();
  response["numberOfSamples"] = currentDataset->numberOfSamples();
  response["qoiNames"] = Json::Value(Json::arrayValue);

  for (std::string qoiName : currentDataset->getQoiNames()) {
    response["qoiNames"].append(qoiName);
  }

  response["attributeNames"] = Json::Value(Json::arrayValue);
  for (std::string attributeName : currentDataset->getAttributeNames()) {
    response["attributeNames"].append(attributeName);
  }


  Json::StyledWriter writer;
  std::string text = writer.write(response);
  wst_sendText(wsi, const_cast<char*>(text.c_str()));
}

/**
 * Handle the command to fetch the k-nearest-neighbor graph of a dataset.
 */
void fetchKNeighbors(void *wsi, int messageId, const Json::Value &request) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= datasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  // TODO: Don't reprocess data if previous commands already did so.
  int n = currentDataset->getDistanceMatrix().N();
  auto KNN = FortranLinalg::DenseMatrix<int>(k, n);
  auto KNND = FortranLinalg::DenseMatrix<Precision>(k, n);
  Distance<Precision>::findKNN(
      currentDataset->getDistanceMatrix(), KNN, KNND);


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
void fetchMorseSmalePersistence(void *wsi, int messageId, const Json::Value &request) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= datasets.size()) {
    // TODO: Send back an error message.
  }

  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = currentTopoData->getMaxPersistenceLevel();

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasetId"] = datasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["minPersistenceLevel"] = minLevel;
  response["maxPersistenceLevel"] = maxLevel;
  response["complexSizes"] = Json::Value(Json::arrayValue);
  for (int level = minLevel; level <= maxLevel; level++) {
    MorseSmaleComplex *complex = currentTopoData->getComplex(level);    
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
void fetchMorseSmalePersistenceLevel(void *wsi, int messageId, const Json::Value &request) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= datasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = currentTopoData->getMaxPersistenceLevel();

  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  MorseSmaleComplex *complex = currentTopoData->getComplex(persistenceLevel);

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
void fetchMorseSmaleCrystal(void *wsi, int messageId, const Json::Value &request) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= datasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = currentTopoData->getMaxPersistenceLevel();

  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  MorseSmaleComplex *complex = currentTopoData->getComplex(persistenceLevel);


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
void fetchMorseSmaleDecomposition(void *wsi, int messageId, const Json::Value &request) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= datasets.size()) {
    // TODO: Send back an error message.
  }
  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }


  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  unsigned int minLevel = currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = currentTopoData->getMaxPersistenceLevel();

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasetId"] = datasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["minPersistenceLevel"] = minLevel;
  response["maxPersistenceLevel"] = maxLevel;
  response["complexes"] = Json::Value(Json::arrayValue);
  for (unsigned int level = minLevel; level <= maxLevel; level++) {
    MorseSmaleComplex *complex = currentTopoData->getComplex(level);
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
void maybeLoadDataset(int datasetId) {
  if (datasetId == currentDatasetId) {
    return;
  }

  if (currentDataset) {
    delete currentDataset;
    currentDataset = nullptr;
  }
  if (currentVizData) {
    delete currentVizData;
    currentVizData = nullptr;
  }
  if (currentTopoData) {
    delete currentTopoData;
    currentTopoData = nullptr;
  }
  currentK = -1;

  auto loadDataset = availableDatasets[datasetId].second;
  currentDataset = loadDataset();
  currentDatasetId = datasetId;
}

/**
 * Checks if the requested dataset has been processed
 * with the chosen k value. If not, processes the data.
 */
void maybeProcessData(int k) { 
  if (k == currentK) {
    return;
  }

  if (currentProcessResult) {
    delete currentProcessResult;
    currentProcessResult = nullptr;
  }
  if (currentVizData) {
    delete currentVizData;
    currentVizData = nullptr;
  }
  if (currentTopoData) {
    delete currentTopoData;
    currentTopoData = nullptr;
  }  
  
  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
  currentK = k;

  // TODO: Provide mechanism to select QoI used.
  // TODO: Expose processing parameters to function interface.
  try {
    currentProcessResult = genericProcessor.processOnMetric(
        currentDataset->getDistanceMatrix(),
        currentDataset->getQoiVector(0),
        currentK  /* knn */,
        25        /* samples */,
        20        /* persistence */,
        true      /* random */,
        0.25      /* sigma */,
        0         /* smooth */);
    currentVizData = new SimpleHDVizDataImpl(currentProcessResult);
    currentTopoData = new LegacyTopologyDataImpl(currentVizData);
  } catch (const char *err) {
    std::cerr << err << std::endl;
    // TODO: Return Error Message.
  }
}
  

/**
 * Construct list of available datasets.
 */
void configureAvailableDatasets() {
  availableDatasets.push_back({"Concrete", loadConcreteDataset});
  availableDatasets.push_back({ "Crimes",  loadCrimesDataset});
  availableDatasets.push_back({"Gaussian", loadGaussianDataset});
  availableDatasets.push_back({"Colorado", loadColoradoDataset});
}


Dataset* loadConcreteDataset() {
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

Dataset* loadCrimesDataset() {
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

Dataset* loadGaussianDataset() {
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

Dataset* loadColoradoDataset() {
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
FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
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
