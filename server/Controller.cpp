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
#include <chrono>

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
const int MAX_DATASET_DEPTH = 3;


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
  m_commandMap.insert({"fetchMorseSmalePersistenceLevel", std::bind(&Controller::fetchMorseSmalePersistenceLevel, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleCrystal", std::bind(&Controller::fetchMorseSmaleCrystal, this, _1, _2)});
  m_commandMap.insert({"fetchSingleEmbedding", std::bind(&Controller::fetchSingleEmbedding, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleRegression", std::bind(&Controller::fetchMorseSmaleRegression, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleExtrema", std::bind(&Controller::fetchMorseSmaleExtrema, this, _1, _2)});
  m_commandMap.insert({"fetchCrystalPartition", std::bind(&Controller::fetchCrystalPartition, this, _1, _2)});
  m_commandMap.insert({"fetchEmbeddingsList", std::bind(&Controller::fetchEmbeddingsList, this, _1, _2)});
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
    std::cout << "[" << messageId << "] " << commandName << "..." << std::endl;

    Json::Value response(Json::objectValue);
    response["id"] = messageId;

    auto command = m_commandMap[commandName];

    if (command) {
      time_point<Clock> start = Clock::now();
      command(request, response);
      time_point<Clock> end = Clock::now();
      milliseconds diff = duration_cast<milliseconds>(end - start);
      std::cout << "[" << messageId << "] " << commandName << " completed in " << static_cast<float>(diff.count())/1000.0f << "s" << std::endl;
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
  auto modelNames = m_currentDataset->getModelNames();
  if (modelNames.size() > 0)
    response["models"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < modelNames.size(); ++i) {
    Json::Value modelObject(Json::objectValue);
    modelObject["name"] = modelNames[i];
    modelObject["id"] = i;
    response["models"].append(modelObject);
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
    std::cout << "Controller::fetchMorseSmaleDecomposition persistence " << level << " num crystals: " << size << std::endl;
  }
}

/**
 * Write the current morse smale decomposition of a dataset.
 */
void Controller::writeMorseSmaleDecomposition(const Json::Value &request, Json::Value &response)
{
  using json = nlohmann::json;

//  if (m_currentVizData->m_data) // if there is a current decomposition and it's valid
  {
//    auto filename(uniqueFilename(request["basePath"].asString() + "crystalpartitions", ".csv"));
//
//    std::ofstream outfile;
//    outfile.open(filename);
//
//    json ms(m_currentVizData->m_data->asJson());
//    std::cout << "M-S crystal partitions:\n" << ms << std::endl;

    // fixme: for now just do this
//    DataExport::exportCrystalPartitions(m_currentVizData->m_data->crystalPartitions, start, filename);
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
 * Handle the command to fetch the details of a single crystal in a persistence level.
 */
// TODO: NOT USED BY ANYTHING!
void Controller::fetchMorseSmaleCrystal(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  std::shared_ptr<MorseSmaleComplex> complex = m_currentTopoData->getComplex(persistence);

  int crystalId = request["crystalId"].asInt();
  if (crystalId < 0 || crystalId >= complex->getCrystals().size())
    return setError(response, "invalid crystal id");

  std::shared_ptr<Crystal> crystal = complex->getCrystals()[crystalId];

  response["datasetId"] = m_currentDatasetId;
  response["decompositionMode"] = "Morse-Smale";
  response["persistence"] = persistence;
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
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

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
  Eigen::Map<Eigen::VectorXf> fieldvals = m_currentDataset->getFieldvalues(m_currentField, m_currentCategory);
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
  auto layout = m_currentVizData->getLayout(HDVizLayout::ISOMAP, persistence);
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

  auto extremaLayout = m_currentVizData->getExtremaLayout(HDVizLayout::ISOMAP, persistence);
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

void Controller::fetchCrystalPartition(const Json::Value &request, Json::Value &response) {
  if (!maybeLoadDataset(request, response))
    return setError(response, "invalid datasetId");

  if (!maybeProcessData(request, response))
    return; // response will contain the error

  // get requested persistence level
  int persistence = getPersistence(request, response);
  if (persistence < 0) return; // response will contain the error

  // the crystal id to look for in this persistence level
  int crystalID = request["crystalID"].asInt();

  auto crystal_partition = m_currentVizData->getCrystalPartitions(persistence);

  response["crystalSamples"] = Json::Value(Json::arrayValue);
  for(unsigned int i = 0; i < crystal_partition.N(); ++i) {
    if(crystal_partition(i) == crystalID) {
      response["crystalSamples"].append(i);
    }
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
  Eigen::Map<Eigen::VectorXf> fieldvals = m_currentDataset->getFieldvalues(parameterName, Fieldtype::DesignParameter);
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
  Eigen::Map<Eigen::VectorXf> fieldvals = m_currentDataset->getFieldvalues(qoiName, Fieldtype::QoI);
  if (!fieldvals.data())
    return setError(response, "invalid fieldname");
  
  response["qoiName"] = qoiName;
  response["qoi"] = Json::Value(Json::arrayValue);
  for (int i = 0; i < fieldvals.size(); i++) {
    response["qoi"].append(fieldvals(i));
  }
}

/**
 * adds the given image to the respose's "thumbnails" array
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
 *   crystalID     - crystal of the given persistence level
 *   modelname     - model from which to fetch interpolated samples
 *   numSamples    - number of evenly-spaced samples of the field at which to generate new latent images
 *   modelSigma    - Gaussian sigma to use for generating new z_coord for shape
 *   showOrig      - simply return original samples for this crystal
 *   validate      - generate model-interpolated images using the z_coords it provided
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
  auto modelSigma = request["modelSigma"].asDouble();
  
  // try to find the requested model
  auto modelset(m_currentDataset->getModelset(fieldname, modelname));
  auto persistence_idx = getAdjustedPersistenceLevelIdx(persistence, modelset);
  auto model(modelset? modelset->getModel(persistence_idx, crystalId) : nullptr);

  // if there isn't a model or original images requested just show its original samples' images 
  bool showOrig = request["showOrig"].asBool();
  if (showOrig || !model)
    return fetchCrystalOriginalSampleImages(request, response);

  // if requested, validate model by generating interpolated images using z_coords of its own samples
  bool validate = request["validate"].asBool();
  if (validate)
    return regenOriginalImagesForCrystal(*modelset, model, persistence_idx, crystalId, response);

  // interpolate the model for the given samples
  auto numZ = request["numSamples"].asInt();
  auto percent = request["percent"].asDouble();
  std::cout << "fetchNImagesForCrystal: " << numZ << " samples requested for crystal "<<crystalId<<" of persistence level "<<persistence <<"; datasetId is "<<m_currentDatasetId<<", fieldname is "<<fieldname<<", modelname is " << modelname;
  if (numZ == 1) std::cout << " (percent is " << percent;
  std::cout << std::endl;
  
  // get the vector of values for the field
  auto samples(modelset->getCrystalSamples(persistence_idx, crystalId));
  Eigen::Map<Eigen::VectorXf> fieldvals(samples.data(), samples.size());
  if (!fieldvals.data())
    return setError(response, "Invalid fieldname or empty field");

  const Image& sample_image = m_currentDataset->getThumbnail(0); // just using this to get dims of image created by model

  // partition the field into numZ values and evaluate model for each
  float minval = model->minFieldValue();
  float maxval = model->maxFieldValue();
  float delta = (maxval - minval) / static_cast<double>(numZ-1); // generates samples for crystal min and max

  // if numZ == 1, evaluate at given percent along crystal
  if (numZ == 1) {
    delta = 1.0;
    minval = minval + (maxval - minval) * percent;
  }
  
  for (unsigned i = 0; i < numZ; i++)
  {
    double fieldval = minval + delta * i;

    // get new latent space coordinate for this field_val
    Eigen::RowVectorXf z_coord = model->getNewLatentSpaceValue(fieldvals, model->getZCoords(), fieldval, modelSigma);

    // evaluate model at this coordinate
    Eigen::MatrixXf I = model->evaluate(z_coord);
    
    // add result image to response // TODO: add cols to this (test that it works using CBII)
    addImageToResponse(response, Image(I, sample_image.getWidth(), sample_image.getHeight()));

    // add field value to response
    response["fieldvals"].append(fieldval);
  }

  response["msg"] = std::string("returning " + std::to_string(numZ) + " requested images predicted by " + modelname + " model at crystal " + std::to_string(crystalId) + " of persistence level " + std::to_string(persistence));
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
 */
void Controller::regenOriginalImagesForCrystal(MSModelset &modelset, std::shared_ptr<Model> model, int persistence_idx, int crystalId, Json::Value &response) {
  // interpolate the model using its original samples
  std::cout << "regenOriginalImagesForCrystal\n";

  auto samples(modelset.getCrystalSamples(persistence_idx, crystalId));
  std::cout << "Testing all latent space variables computed for the " << samples.size() << " samples in this model.\n";

  //z coords are sorted by fieldvalue in Model::setFieldValues (TODO: not anymore, and not sure they need to be)
  for (auto i = 0; i < samples.size(); i++)
  {
    // load thumbnail corresponding to this z_idx for comparison to evaluated model at same z_idx (they should be close)
    if (!m_currentDataset)
      throw std::runtime_error("ERROR: tried to access controller's current dataset, but it's NULL.");

    const Image& sample_image = m_currentDataset->getThumbnail(modelset.getPersistenceLevel(persistence_idx).crystals[crystalId].getSampleIndices()[i]);
    unsigned sampleWidth = sample_image.getWidth(), sampleHeight = sample_image.getHeight();

    //std::string outputBasepath(datapath + "/debug/outimages");
    //std::string outpath(outputBasepath + "/pidx" + std::to_string(persistence_idx) + "-c" + std::to_string(crystalid) + "-z" + std::to_string(samples[i].idx) + ".png");

    Eigen::MatrixXf I = model->evaluate(model->getZCoord(i));//, false /*writeToDisk*/,
    //""/*outpath*/, sample_image.getWidth(), sample_image.getHeight());

    //todo: simplify this to use the images passed in rather than re-generating (rename to compareImages)
    float quality = Model::testEvaluateModel(model, model->getZCoord(i),
                                             sample_image);//, false /*writeToDisk*/, ""/*outputBasepath*/);

    std::cout << "Quality of generation of image for model at persistence_idx "
              << persistence_idx << ", crystalid " << crystalId << ": " << quality << std::endl;

    // add image to response
    addImageToResponse(response, Image(I, sample_image.getWidth(), sample_image.getHeight()));

    // add field value to response
    response["fieldvals"].append(samples[i]);
  }

  response["msg"] = std::string("returning interpolated images by model at crystal " + std::to_string(crystalId) + " of persistence_idx" + std::to_string(persistence_idx));
}

/* 
 * Returns vector of global sample ids and their fieldvalue for the samples from which the crystal is comprised.
 */
std::vector<ValueIndexPair> Controller::getSamples(Fieldtype category, const std::string &fieldname,
                                                   unsigned persistence, unsigned crystalid, bool sort) {
  std::vector<ValueIndexPair> fieldvalues_and_indices;

  // get the vector of values for the field
  Eigen::Map<Eigen::VectorXf> fieldvals = m_currentDataset->getFieldvalues(fieldname, category);
  if (!fieldvals.data()) {
    std::cerr << "Invalid fieldname or empty field\n";
    return fieldvalues_and_indices;
  }

  // create a vector of global sample ids and their fieldvalue for the samples from which this crystal is comprised
  FortranLinalg::DenseVector<int> &crystal_partition(m_currentVizData->getCrystalPartitions(persistence));
  Eigen::Map<Eigen::VectorXi> partitions(crystal_partition.data(), crystal_partition.N());
  for (unsigned i = 0; i < partitions.size(); i++)
  {
    if (partitions(i) == crystalid)
    {
      ValueIndexPair sample;
      sample.idx = i;
      sample.val = fieldvals(i);
      fieldvalues_and_indices.push_back(sample);
    }
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

  std::string fieldname = request["fieldname"].asString();
  int crystalid = request["crystalID"].asInt();
  std::cout << "fetchCrystalOriginalSampleImages: datasetId is "<<m_currentDatasetId<<", persistence is "<<persistence<<", crystalid is "<<crystalid<<std::endl;

  auto fieldvalues_and_indices(getSamples(category, fieldname, persistence, crystalid));
  std::cout << "Returning images for the " << fieldvalues_and_indices.size() << " samples in this crystal.\n";

  for (auto sample: fieldvalues_and_indices)
  {
    // load thumbnail corresponding to this z_idx
    const Image& sample_image = m_currentDataset->getThumbnail(sample.idx);
    unsigned sampleWidth = sample_image.getWidth(), sampleHeight = sample_image.getHeight();

    // add image to response
    addImageToResponse(response, sample_image);  // todo: add index to response so drawer can display it

    // add field value to response
    response["fieldvals"].append(sample.val);
  }

  response["msg"] = std::string("returning " + std::to_string(fieldvalues_and_indices.size()) + " images for crystal " + std::to_string(crystalid) + " of persistence level " + std::to_string(persistence));
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
  auto curvepoints = request.isMember("curvepoints") ? request["curvepoints"].asInt()            : m_currentNumSamples;
  auto sigma       = request.isMember("sigma")       ? request["sigma"].asFloat()                : m_currentSigma;
  auto smoothing   = request.isMember("smooth")      ? request["smooth"].asFloat()               : m_currentSmoothing;
  auto addnoise    = request.isMember("noise")       ? request["noise"].asBool()                 : m_currentAddNoise;
  auto depth       = request.isMember("depth")       ? request["depth"].asInt()                  : m_currentNumPersistences;
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
  return !(m_currentCategory        == category         &&
           m_currentField           == fieldname        &&
           m_currentKNN             == knn              &&
           m_currentNumSamples      == num_samples      &&
           m_currentSigma           == sigma            &&
           m_currentSmoothing       == smoothing        &&
           m_currentAddNoise        == add_noise        &&
           m_currentNumPersistences == num_persistences &&
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
  Eigen::Map<Eigen::Matrix<Precision, Eigen::Dynamic, 1>> fieldvals = m_currentDataset->getFieldvalues(fieldname, category);
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
    m_currentVizData.reset(new SimpleHDVizDataImpl(
      genericProcessor.processOnMetric(m_currentDistanceMatrix,
                                       FortranLinalg::DenseVector<Precision>(fieldvals.size(), fieldvals.data()),
                                       knn,              /* k nearest neighbors to consider */
                                       num_samples,      /* points along each crystal regression curve */
                                       num_persistences, /* generate this many at most; -1 generates all of 'em */
                                       add_noise,        /* adds very slight noise to field values */
                                       sigma,            /* soften crystal regression curves */
                                       smoothing)));      /* smooth topology */
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
  m_currentNumSamples = num_samples;
  m_currentSigma = sigma;
  m_currentSmoothing = smoothing;
  m_currentAddNoise = add_noise;
  m_currentNumPersistences = num_persistences;
  m_currentNormalize = normalize;
  
  return true;
}

} // dspacex
