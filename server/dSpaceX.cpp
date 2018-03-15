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

std::vector<Dataset*> datasets;
Dataset *currentDataset = nullptr;

/* NOTE: the data read must me moved to the back-end and then
         these globals cn be made local */
std::vector<DenseVectorSample*> samples;
FortranLinalg::DenseVector<Precision> y;

// Command Handlers
void fetchDatasetList(void *wsi, int messageId, const Json::Value &request);
void fetchDataset(void *wsi, int messageId, const Json::Value &request);

void loadAllDatasets();
void loadConcreteDataset();
void loadCrimesDataset();
void loadGaussianDataset();
void loadColoradoDataset();

FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x);

HDProcessResult *result = nullptr;

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
    loadAllDatasets();
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

  for (size_t i=0; i < datasets.size(); i++) {
    Json::Value object = Json::Value(Json::objectValue);
    object["id"] = static_cast<int>(i);
    object["name"] = datasets[i]->getName();
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
  if (datasetId < 0 || datasetId >= datasets.size()) {
    // TODO: Send back an error message.
  }
  currentDataset = datasets[datasetId];


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
 *
 */
void fetchMorseSmaleDecomposition(void *wsi, int messageId, const Json::Value &request) {
  int datasetId = request["datasetId"].asInt();
  int k = request["k"].asInt();

  // HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;

  // result = genericProcessor.processOnMetric(
  //    currentDataset->getDistanceMatrix(),
  //    currentDataset->getQoiVector(currentQoi),
  //    k        /* knn */,
  //    25        /* samples */,
  //    20        /* persistence */,
  //    true      /* random */,
  //    0.25      /* sigma */,
  //    0         /* smooth */);
}


/**
 * Load available datasets.
 */
void loadAllDatasets() {
  loadConcreteDataset();
  loadCrimesDataset();
  loadGaussianDataset();
  loadColoradoDataset();
}


void loadConcreteDataset() {
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
  datasets.push_back(dataset);

  std::cout << datasetName << " dataset loaded." << std::endl;
}

void loadCrimesDataset() {
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
  datasets.push_back(dataset);

  std::cout << datasetName << " dataset loaded." << std::endl;
}

void loadGaussianDataset() {
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
  datasets.push_back(dataset);
  std::cout << datasetName << " dataset loaded." << std::endl;
}

void loadColoradoDataset() {
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
  datasets.push_back(dataset);
  std::cout << datasetName << " dataset loaded." << std::endl;
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
