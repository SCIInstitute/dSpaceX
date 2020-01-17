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
#include "sharedgp/SharedGP.h"
#include "shapeodds/ShapeOdds.h"
#include "lodepng.h"

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


Controller::Controller(const std::string &datapath_) : datapath(datapath_) {
  configureCommandHandlers();
  configureAvailableDatasets(datapath);
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
  m_commandMap.insert({"fetchSingleEmbedding", std::bind(&Controller::fetchSingleEmbedding, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleRegression", std::bind(&Controller::fetchMorseSmaleRegression, this, _1, _2)});
  m_commandMap.insert({"fetchMorseSmaleExtrema", std::bind(&Controller::fetchMorseSmaleExtrema, this, _1, _2)});
  m_commandMap.insert({"fetchCrystalPartition", std::bind(&Controller::fetchCrystalPartition, this, _1, _2)});
  m_commandMap.insert({"fetchEmbeddingsList", std::bind(&Controller::fetchEmbeddingsList, this, _1, _2)});
  m_commandMap.insert({"fetchParameter", std::bind(&Controller::fetchParameter, this, _1, _2)});
  m_commandMap.insert({"fetchQoi", std::bind(&Controller::fetchQoi, this, _1, _2)});
  m_commandMap.insert({"fetchThumbnails", std::bind(&Controller::fetchThumbnails, this, _1, _2)});
  //m_commandMap.insert({"fetchNewLatentSpaceCoord_ShapeOdds", std::bind(&Controller::fetchNewLatentSpaceCoord_ShapeOdds, this, _1, _2)});
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
    // TODO: Send back an error message.
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
      response["graph"][i].append(KNN(i, j));
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
    MorseSmaleComplex *complex = m_currentTopoData->getComplex(level);  // <ctc> is this where the crystal numbering bug lives?
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
void Controller::fetchSingleEmbedding(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }

  int embeddingId = request["embeddingId"].asInt();

  int k = request["k"].asInt();
  if (k < 0) {
    // TODO: Send back an error message.
  }

  unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
  unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();
  int persistenceLevel = request["persistenceLevel"].asInt();
  if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
    // TODO: Send back an error message. Invalid persistenceLevel.
  }

  maybeLoadDataset(datasetId);
  maybeProcessData(k);

  // TODO:  Modify logic to return layout based on chosen layout type.
  //        For now, only send back embeddings provided with the dataset.
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

  // Get colors based on current QOI
  std::string qoiName = request["qoiName"].asString();
  auto qois = m_currentDataset->getQoiNames();
  auto result = std::find(std::begin(qois), std::end(qois), qoiName);
  if (result == std::end(qois)) {
    // TODO: Send back an error message. Invalid Qoi Name.
  }
  int qoiIndex = result - std::begin(qois);
  auto qoiVector = m_currentDataset->getQoiVector(qoiIndex);

  auto colorMap = m_currentVizData->getColorMap(persistenceLevel);
  response["colors"] = Json::Value(Json::arrayValue);
  for(unsigned int i = 0; i < qoiVector.N(); ++i) {
    auto colorRow = Json::Value(Json::arrayValue);
    auto color = colorMap.getColor(qoiVector(i));
    colorRow.append(color[0]);
    colorRow.append(color[1]);
    colorRow.append(color[2]);
    response["colors"].append(colorRow);
  }
}

void Controller::fetchMorseSmaleRegression(const Json::Value &request, Json::Value &response) {
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
  auto layout = m_currentVizData->getLayout(HDVizLayout::ISOMAP, persistenceLevel);
  int rows = m_currentVizData->getNumberOfSamples() + 2;
  double points[rows][3];
  double colors[rows][3];

  // For each crystal
  response["curves"] = Json::Value(Json::arrayValue);
  for (unsigned int i = 0; i < m_currentVizData->getCrystals(persistenceLevel).N(); i++) {

    // Get all the points and node colors
    for (unsigned int n = 0; n < layout[i].N(); ++n) {
      auto color = m_currentVizData->getColorMap(persistenceLevel).getColor(
                                                                            m_currentVizData->getMean(persistenceLevel)[i](n));
      colors[n + 1][0] = color[0];
      colors[n + 1][1] = color[1];
      colors[n + 1][2] = color[2];

      for (unsigned int m = 0; m < layout[i].M(); ++m) {
        points[n + 1][m] = layout[i](m, n);
      }
      points[n + 1][2] = m_currentVizData->getMeanNormalized(persistenceLevel)[i](n);
    }

    for (unsigned int j = 0; j < 3; ++j) {
      points[0][j] = points[1][j] + points[2][j] - points[1][j];
      points[m_currentVizData->getNumberOfSamples() + 1][j] =
      points[m_currentVizData->getNumberOfSamples()][j] +
      points[m_currentVizData->getNumberOfSamples()][j] -
      points[m_currentVizData->getNumberOfSamples() - 1][j];
    }

    for (unsigned int j = 0; j < 3; ++j) {
      colors[0][j] = colors[1][j];
      colors[m_currentVizData->getNumberOfSamples() + 1][j] = colors[m_currentVizData->getNumberOfSamples()][j];
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
    int datasetId = request["datasetId"].asInt();
    if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
        // TODO: Send back an error message.
    }

    unsigned int minLevel = m_currentTopoData->getMinPersistenceLevel();
    unsigned int maxLevel = m_currentTopoData->getMaxPersistenceLevel();

    int persistenceLevel = request["persistenceLevel"].asInt();
    if (persistenceLevel < minLevel || persistenceLevel > maxLevel) {
        // TODO: Send back an error message. Invalid persistenceLevel.
    }

    int crystalID = request["crystalID"].asInt();

    auto crystal_partition = m_currentProcessResult->crystalPartitions[persistenceLevel];

    response["crystalSamples"] = Json::Value(Json::arrayValue);
    for(unsigned int i = 0; i < crystal_partition.N(); ++i) {
        if(crystal_partition(i) == crystalID) {
            response["crystalSamples"].append(i);
        }
    }
}

void Controller::fetchEmbeddingsList(const Json::Value &request, Json::Value &response) {
    int datasetId = request["datasetId"].asInt();
    if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
        // TODO: Send back an error message.
    }

    maybeLoadDataset(datasetId);

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
    // imageObject["data"] = base64_encode(  // <ctc> not needed by anything, so why are we sending this?
    //                                     reinterpret_cast<const unsigned char *>(image.getData()),
    //                                     4 * image.getWidth() * image.getHeight());
    imageObject["rawData"] = base64_encode(
                                           reinterpret_cast<const unsigned char *>(&image.getRawData()[0]),
                                           image.getRawData().size());
    response["thumbnails"].append(imageObject);
  }
}

/**
 * This computes a new latent space variable for the given model using:
 * 2d coord, Mu, Theta, then Estep, etc. (<ctc> look at matlab code and review discussions w/ Shireen and Wei)
 *
 * Parameters: 
 *   datasetId - should already be loaded
 *   fieldname - (e.g., one of the QoIs)
 *   persistenceId - persistence level of the M-S for this field of the dataset
 *   crystalId - crystal of the given persistence level
 *   ... (whatever is needed to decide how to compute the new z_coord (ex: 2d coord, Mu, Theta, ...)
 */
// void Controller::fetchNewLatentSpaceCoord_ShapeOdds(const Json::Value &request, Json::Value &response) {
//   int datasetId = request["datasetId"].asInt();
  // <ctc> TODO:
  
/**
 * This computes and returns a new sample (image) for the given latent space coordinate using
 * the ShapeOdds model for the specified crystal at this persistence level
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
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  maybeLoadDataset(datasetId);
  // maybeLoadModel(persistence,crystal); //<ctc> if for speed we decide not to load models till their evaluation is requested

  int persistence = 15; //request["persistence"].asInt();
  int crystalid = 6; //request["crystalid"].asInt();
  //int zidx = request["zidx"].asInt();  // <ctc> really, we want to pass a latent space variable z, which we'll also generate serverside
  std::cout << "fetchImageForLatentSpaceCoord_Shapeodds: datasetId is "<<datasetId<<", persistence is "<<persistence<<", crystalid is "<<crystalid<<std::endl;

  //create images using the elements of this model's Z
  const Shapeodds::Model &model(m_currentDataset->getMSModels()[0].getModel(persistence, crystalid).second);

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
 *                   <ctc> for now, just calling fetchAllImageForCrystal_Shapeodds
 */
void Controller::fetchNImagesForCrystal_Shapeodds(const Json::Value &request, Json::Value &response) {
  // <ctc> TODO: partition level set into numZ evenly-spaced field vals and compute a z_coord for each
  //       for now, just calling fetchAllImageForCrystal_Shapeodds
  int datasetId = request["datasetId"].asInt();
  int persistence = request["persistenceLevel"].asInt();
  int crystalid = request["crystalID"].asInt();
  int numZ = request["numSamples"].asInt();
  std::cout << "fetchNImagesForCrystal_Shapeodds: " << numZ << " samples requested for crystal "<<crystalid<<" of persistence level "<<persistence<<" (datasetId is "<<datasetId<<")\n";
  Controller::fetchAllImagesForCrystal_Shapeodds(request, response);
}

// converts w*h x 1 matrix of doubles to w x h 2d image of unsigned char, throwing an exception if dims don't match
// <ctc> TODO move this to utils (or somewhere) so it can be used by others
Image convertToImage(const Eigen::MatrixXd &I, const unsigned w, const unsigned h)
{
  Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> image_mat = (I.array() * 255.0).cast<unsigned char>();

  if (image_mat.size() != w * h)
  { 
    throw std::runtime_error("Warning: w * h (" + std::to_string(w) + " * " + std::to_string(h) + ") != computed image size (" + std::to_string(image_mat.size()) + ")");
  }
  image_mat.resize(h, w);
  image_mat.transposeInPlace();  // make data row-order

  std::vector<unsigned char> png;
  unsigned error = lodepng::encode(png, image_mat.data(), w, h, LCT_GREY, 8);
  if (error) {
    throw std::runtime_error("encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));
  } 

  std::vector<char> char_png_vec(w * h);        //<ctc> grumble-grumble... it'll just turn around and be converted back to unsigned char*
  std::copy(png.begin(), png.end(), char_png_vec.begin());
  return Image(w, h, NULL, char_png_vec, "png");

#if 0
  //<ctc> this doesn't work since when the Eigen::Matrix goes out of scope it still deletes its data.
  //  char *image_data = reinterpret_cast<char *>(std::move(image.data()));
  // ...so instead we just copy it for now, but I put a question out there to see if Eigen's matrix can relinquish its data.
  std::vector<char> image_data_vec(w * h);
  char *idata = reinterpret_cast<char *>(image.data());
  std::copy(idata, idata + w * h, image_data_vec.begin());
  //return Image(w, h, NULL, image_data_vec, "raw");
  unsigned char *foo = std::reinterpret_cast<unsigned char*>(idata); // why the error calling the Image constructor with this argument? -> must use static_cast!
  //return Image(w, h, std::reinterpret_cast<unsigned char*>(idata), image_data_vec, "raw");
  return Image(w, h, (unsigned char*)idata, image_data_vec, "raw");
#endif
}

/**
 * This returns a set of new samples (images) computed with the given ShapeOdds model for the
 * specified crystal at this persistence level using thelatent space coordinates for each of
 * the original samples of model/crystal (each sample has a z_coord).
 *
 * Parameters: 
 *   datasetId - should already be loaded
 *   fieldname     - (e.g., one of the QoIs)
 *   persistenceId - persistence level of the M-S for this field of the dataset
 *   crystalId     - crystal of the given persistence level
 */
void Controller::fetchAllImagesForCrystal_Shapeodds(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  maybeLoadDataset(datasetId);
  // maybeLoadModel(persistence,crystal); //<ctc> if for speed we decide not to load models till their evaluation is requested

  int persistence = request["persistenceLevel"].asInt();
  int crystalid = request["crystalID"].asInt();
  crystalid += 1; //<ctc> fixme: client is requesting incorrect crystal
  std::cout << "fetchAllImagesForCrystal_Shapeodds: datasetId is "<<datasetId<<", persistence is "<<persistence<<", crystalid is "<<crystalid<<std::endl;

  //create images using the elements of this model's Z
  const Shapeodds::Model &model(m_currentDataset->getMSModels()[0].getModel(persistence, crystalid).second);
  response["thumbnails"] = Json::Value(Json::arrayValue);

  auto sample_indices(model.getSampleIndices());
  std::cout << "Testing all latent space variables computed for the " << sample_indices.size() << " samples in this model.\n";
  for (unsigned zidx = 0; zidx < sample_indices.size(); zidx++)
  {
    // load thumbnail corresponding to this z_idx for comparison to evaluated model at same z_idx (they should be close)
    extern Controller *controller;
    if (!controller || !controller->m_currentDataset)
      throw std::runtime_error("ERROR: tried to access controller's current dataset, but it's NULL.");

    const Image& sample_image = controller->m_currentDataset->getThumbnail(zidx);
    unsigned sampleWidth = sample_image.getWidth(), sampleHeight = sample_image.getHeight();

    std::string outputBasepath(datapath + "/CantileverBeam_wclust_wraw/outimages"); //<ctc> todo: dataset_name or /debug/datasetname/outimages
    std::string outpath(outputBasepath + "/p" + std::to_string(persistence) + "-c" + std::to_string(crystalid) +
                        "-z" + std::to_string(zidx) + ".png");
    Eigen::MatrixXd I = Shapeodds::ShapeOdds::evaluateModel(model, model.getZCoord(zidx), true /*writeToDisk*/,
                                                            outpath, sample_image.getWidth(), sample_image.getHeight());

    //<ctc> simplify this to use the images passed in rather than re-generating (rename it to compareImages or something)
    float quality = Shapeodds::ShapeOdds::testEvaluateModel(model, model.getZCoord(zidx), persistence, crystalid, zidx, sample_image,
                                                            true /*writeToDisk*/, outputBasepath);

    // todo: is "quality" the correct term for comparison of generated image vs original?
    std::cout << "Quality of generation of image for model at persistence level "
              << persistence << ", crystalid " << crystalid << ": " << quality << std::endl;

    // add image to response
    Image image = convertToImage(I, sample_image.getWidth(), sample_image.getHeight());
    Json::Value imageObject = Json::Value(Json::objectValue);
    imageObject["width"] = image.getWidth();
    imageObject["height"] = image.getHeight();
    //imageObject["data"] = base64_encode(image.getConstData(), 4 * image.getWidth() * image.getHeight());  //<ctc> not used by client
    imageObject["rawData"] = base64_encode(reinterpret_cast<const unsigned char *>(&image.getConstRawData()[0]), image.getConstRawData().size());
    response["thumbnails"].append(imageObject);
  }

  response["msg"] = std::string("returning " + std::to_string(sample_indices.size()) + " images for model at crystal " + std::to_string(crystalid) + " of persistence level " + std::to_string(persistence));
}

/**
 * This fetches the QoI, design params, and image for a given latent space produced by the SharedGP library.
 */
void Controller::fetchAllForLatentSpaceUsingSharedGP(const Json::Value &request, Json::Value &response) {
  int datasetId = request["datasetId"].asInt();
  if (datasetId < 0 || datasetId >= m_availableDatasets.size()) {
    // TODO: Send back an error message.
  }
  int qoi = request["qoi"].asInt();
  std::cout << "fetchAllForLatentSpaceUsingSharedGP: datasetId is "<<datasetId<<", qoi is "<<qoi<<std::endl;
  if (qoi < 0) {
    // TODO: Send back an error message.
  }

  std::cout << "TODO: return something :-)\n";
  response["msg"] = std::string("need to return desired QoI, DPs, and an image for the given shared_gp latent space");
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
                                                              50        /* samples */,
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
