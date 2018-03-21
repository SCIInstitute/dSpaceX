/*
 *      dSpaceX server
 */
#include "HDGenericProcessor.h"
#include "SimpleHDVizDataImpl.h"
#include "TopologyData.h"
#include "LegacyTopologyDataImpl.h"
#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "util/DenseVectorSample.h"
#include "util/csv/loaders.h"
#include "Dataset.h"

#include <jsoncpp/json.h>

#include <exception>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>		// usleep

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#define strtok_r strtok_s
#endif

#include "wst.h"

#include "dsxdyn.h"

/* globals used in these functions */
static wstContext *cntxt;
static dsxContext dsxcntxt;

typedef Dataset* (*LoadFunction)(void);
std::vector<std::pair<std::string, LoadFunction>> availableDatasets;
std::vector<Dataset*> datasets;
Dataset *currentDataset = nullptr;
HDProcessResult *result = nullptr;
HDVizData *data = nullptr;
TopologyData *topoData = nullptr;

/* NOTE: the data read must me moved to the back-end and then
         these globals cn be made local */
std::vector<DenseVectorSample*> samples;
FortranLinalg::DenseVector<Precision> y;

// Command Handlers
void fetchDatasetList(void *wsi, int messageId, const Json::Value &request);
void fetchDataset(void *wsi, int messageId, const Json::Value &request);
void fetchKNeighbors(void *wsi, int messageId, const Json::Value &request);
void fetchMorseSmalePersistence(void *wsi, int messageId, const Json::Value &request);
void fetchMorseSmalePersistenceLevel(void *wsi, int messageId, const Json::Value &request);
void fetchMorseSmaleCrystal(void *wsi, int messageId, const Json::Value &request);
void fetchMorseSmaleDecomposition(void *wsi, int messageId, const Json::Value &request);

void configureAvailableDatasets();
Dataset* loadConcreteDataset();
Dataset* loadCrimesDataset();
Dataset* loadGaussianDataset();
Dataset* loadColoradoDataset();

FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x);


int main(int argc, char *argv[])
{
  int port = 7681;

  /* get our starting application line
   *
   * for example on a Mac:
   * setenv DSX_START "open -a /Applications/Firefox.app ../client/dSpaceX.html"
   */
  char  *startapp = getenv("DSX_START");

  if ((argc != 1) && (argc != 2)) {
    printf("\n Usage: dSpaceX [port]\n\n");
    return 1;
  }
  if (argc == 2) port = atoi(argv[1]);

  /* initialize the back-end subsystem
     assume we have started where we can file the DLL/so */
  int stat = dsx_Load(&dsxcntxt, "dSpaceXbe");
  if (stat < 0) {
    printf(" failed to Load Back-End %d!\n", stat);
    return 1;
  }

  try {
    configureAvailableDatasets();
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
  }


  /* create the Web Socket Transport context */
  cntxt = wst_createContext();
  if (cntxt == NULL) {
    printf(" failed to create wstContext!\n");
    return -1;
  }

  // start the server code
  stat = 0;
  if (wst_startServer(port, NULL, NULL, NULL, 0, cntxt) == 0) {

    /* we have a single valid server */
    while (wst_statusServer(0)) {
      usleep(500000);
      if (stat == 0) {
        if (startapp != NULL) system(startapp);
        stat++;
      }
      if (wst_nClientServer(0) == 0) {
        continue;
      }
      // wst_broadcastText("I'm here!");
    }
  }

  /* finish up */
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
    std::cout << "messageId:" << messageId << std::endl;

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

  if (currentDataset) {
    delete currentDataset;
    currentDataset = nullptr;
  }
  auto loadDataset = availableDatasets[datasetId].second;
  currentDataset = loadDataset();

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

  // TODO: Don't reprocess data if previous commands already did so.
  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;

  // TODO: Provide mechanism to select QoI used.
  try {
    result = genericProcessor.processOnMetric(
        currentDataset->getDistanceMatrix(),
        currentDataset->getQoiVector(0),
        k        /* knn */,
        25        /* samples */,
        20        /* persistence */,
        true      /* random */,
        0.25      /* sigma */,
        0         /* smooth */);
    data = new SimpleHDVizDataImpl(result);
    topoData = new LegacyTopologyDataImpl(data);
  } catch (const char *err) {
    std::cerr << err << std::endl;
    // TODO: Return Error Message.
  }

  unsigned int minLevel = topoData->getMinPersistenceLevel();
  unsigned int maxLevel = topoData->getMaxPersistenceLevel();

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasetId"] = datasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["minPersistenceLevel"] = minLevel;
  response["maxPersistenceLevel"] = maxLevel;

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

  // TODO: Don't reprocess data if previous commands already did so.
  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;

  // TODO: Provide mechanism to select QoI used.
  try {
    result = genericProcessor.processOnMetric(
        currentDataset->getDistanceMatrix(),
        currentDataset->getQoiVector(0),
        k        /* knn */,
        25        /* samples */,
        20        /* persistence */,
        true      /* random */,
        0.25      /* sigma */,
        0         /* smooth */);
    data = new SimpleHDVizDataImpl(result);
    topoData = new LegacyTopologyDataImpl(data);
  } catch (const char *err) {
    std::cerr << err << std::endl;
    // TODO: Return Error Message.
  }

  unsigned int minLevel = topoData->getMinPersistenceLevel();
  unsigned int maxLevel = topoData->getMaxPersistenceLevel();

  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  MorseSmaleComplex *complex = topoData->getComplex(persistenceLevel);

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
    crystalObject["numberOfSamples"] = (int)(crystal->getAllSamples().size());
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

  // TODO: Don't reprocess data if previous commands already did so.
  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;

  // TODO: Provide mechanism to select QoI used.
  try {
    result = genericProcessor.processOnMetric(
        currentDataset->getDistanceMatrix(),
        currentDataset->getQoiVector(0),
        k        /* knn */,
        25        /* samples */,
        20        /* persistence */,
        true      /* random */,
        0.25      /* sigma */,
        0         /* smooth */);
    data = new SimpleHDVizDataImpl(result);
    topoData = new LegacyTopologyDataImpl(data);
  } catch (const char *err) {
    std::cerr << err << std::endl;
    // TODO: Return Error Message.
  }

  unsigned int minLevel = topoData->getMinPersistenceLevel();
  unsigned int maxLevel = topoData->getMaxPersistenceLevel();

  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  MorseSmaleComplex *complex = topoData->getComplex(persistenceLevel);


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

  std::cout << "datasetId = " << datasetId << std::endl;
  // TODO: Verify that currentData == datasetId.
  //       Reload/set currentData if different.

  // TODO: Don't reprocess data if previous commands already did so.
  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;

  // TODO: Provide mechanism to select QoI used.
  try {
    result = genericProcessor.processOnMetric(
        currentDataset->getDistanceMatrix(),
        currentDataset->getQoiVector(0),
        k        /* knn */,
        25        /* samples */,
        20        /* persistence */,
        true      /* random */,
        0.25      /* sigma */,
        0         /* smooth */);
    data = new SimpleHDVizDataImpl(result);
    topoData = new LegacyTopologyDataImpl(data);
  } catch (const char *err) {
    std::cerr << err << std::endl;
    // TODO: Return Error Message.
  }

  unsigned int minLevel = topoData->getMinPersistenceLevel();
  unsigned int maxLevel = topoData->getMaxPersistenceLevel();

  Json::Value response(Json::objectValue);
  response["id"] = messageId;
  response["datasetId"] = datasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["minPersistenceLevel"] = minLevel;
  response["maxPersistenceLevel"] = maxLevel;
  response["complexes"] = Json::Value(Json::arrayValue);
  for (unsigned int level = minLevel; level <= maxLevel; level++) {
    MorseSmaleComplex *complex = topoData->getComplex(level);
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


FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x) {
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
