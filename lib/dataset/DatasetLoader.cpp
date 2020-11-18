#include "DatasetLoader.h"
#include "utils/loaders.h"
#include <yaml-cpp/yaml.h>
#include "utils/StringUtils.h"
#include "utils/IO.h"
#include "utils/utils.h"
#include "Dataset/ValueIndexPair.h"

#include <memory>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <chrono>

// This clock corresponds to CLOCK_MONOTONIC at the syscall level.
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using namespace std::literals::chrono_literals;

using namespace dspacex;

struct InputFormat
{
  enum Type { CSV, JSON, LINALG_DENSEMATRIX, LINALG_DENSEVECTOR };
  Type type;

  InputFormat() : type(CSV)
  {}
  InputFormat(const std::string &str)
  {
    if (!str.compare("csv"))                  type = CSV;                return;
    if (!str.compare("json"))                 type = JSON;               return;
    if (!str.compare("Linalg.DenseMatrix"))   type = LINALG_DENSEMATRIX; return;
    if (!str.compare("Linalg.DenseVector"))   type = LINALG_DENSEVECTOR; return;
    throw std::runtime_error("Invalid input format specified: " + str);
  }

  friend std::ostream& operator<<(std::ostream &out, const InputFormat &c); 
};

std::ostream& operator<<(std::ostream &out, const InputFormat &f) 
{ 
  switch (f.type)
  {
    case InputFormat::CSV:                out << "csv";                break;
    case InputFormat::JSON:               out << "json";               break;
    case InputFormat::LINALG_DENSEMATRIX: out << "Linalg.DenseMatrix"; break;
    case InputFormat::LINALG_DENSEVECTOR: out << "Linalg.DenseVector"; break;
    default:
      throw std::runtime_error("Invalid InputFormat type: " + (int)f.type);
  }

  return out; 
} 

std::string basePathOf(const std::string &filePath) {
  size_t found = filePath.find_last_of("/\\");
  std::string path = filePath.substr(0,found);
  if(path.empty()) {
    path = "./";
  }
  if (path[path.size() - 1] != '/') {
    path = path + "/";
  }
  return path;
}

std::unique_ptr<Dataset> DatasetLoader::loadDataset(const std::string &filePath) {
  YAML::Node config = YAML::LoadFile(filePath);
  Dataset::Builder builder;

  std::cout << "Reading " << filePath << std::endl;

  std::string name = DatasetLoader::parseName(config);
  std::cout << "name: " << name << std::endl;
  builder.withName(name);

  int sampleCount = DatasetLoader::parseSampleCount(config);
  std::cout << "samples: " << sampleCount << std::endl;
  builder.withSampleCount(sampleCount);

  if (config["parameters"]) {
    auto parameters = DatasetLoader::parseParameters(config, filePath);
    for (auto parameter : parameters) {
      builder.withParameter(parameter.first, parameter.second);
    }
  }

  auto qois = DatasetLoader::parseQois(config, filePath);
  for (auto qoi : qois) {
    builder.withQoi(qoi.first, qoi.second);
  }

  if (config["geometry"]) {
    auto geometry = DatasetLoader::parseGeometry(config, filePath);
    builder.withSamplesMatrix(geometry);
  }

  if (config["distances"]) {
    auto distances = DatasetLoader::parseDistances(config, filePath);
    builder.withDistanceMatrix(distances);
  }

  if (config["embeddings"]) {
    auto embeddings = DatasetLoader::parseEmbeddings(config, filePath);
    for (auto embedding : embeddings) {
      builder.withEmbedding(embedding.first, embedding.second);
    }
  }

  if (config["models"]) {
    auto modelmap = DatasetLoader::parseModels(config, filePath);
    for (auto models : modelmap)
      for (auto model : models.second)
        builder.withModel(models.first, model);
  }

  if (config["thumbnails"]) {
    auto thumbnails = DatasetLoader::parseThumbnails(config, filePath);
    builder.withThumbnails(thumbnails);
  }

  return builder.build();
}

std::string DatasetLoader::getDatasetName(const std::string &filePath) {
  YAML::Node config = YAML::LoadFile(filePath);
  std::string name = DatasetLoader::parseName(config);
  return name;
}


std::string DatasetLoader::parseName(const YAML::Node &config) {
  if (!config["name"]) {
    throw std::runtime_error("Dataset config missing 'name' field.");
  }
  return config["name"].as<std::string>();
}


int DatasetLoader::parseSampleCount(const YAML::Node &config) {
  if(!config["samples"]) {
    throw std::runtime_error("Dataset config missing 'samples' field.");
  }
  if (!config["samples"]["count"]) {
    throw std::runtime_error("Dataset config samples missing 'counts' field.");
  }
  return config["samples"]["count"].as<int>();
}


std::vector<ParameterNameValuePair> DatasetLoader::parseParameters(
    const YAML::Node &config, const std::string &filePath) {
  std::vector<ParameterNameValuePair> parameters;
  if (!config["parameters"]) {
    throw std::runtime_error("Dataset config missing 'parameters' field.");
  }
  const YAML::Node &parametersNode = config["parameters"];

  if (parametersNode.IsSequence()) {
    // Parse Parameters one by one if in list form:
    std::cout << "Reading sequence of " << parametersNode.size()
              << " parameters." << std::endl;
    for (std::size_t i = 0; i < parametersNode.size(); i++) {
      const YAML::Node &parameterNode = parametersNode[i];
      auto parameter = DatasetLoader::parseParameter(parameterNode, filePath);
      parameters.push_back(parameter);
    }
  } else {
    // Parse all Parameters from a file if not in list form.
    std::string format = parametersNode["format"].as<std::string>();
    if (format != "csv") {
      throw std::runtime_error(
        "Dataset config specifies unsupported multi-parameter format: " + format);
    }
    if (!parametersNode["file"]) {
      throw std::runtime_error("Parameters missing 'file' field.");
    }
    std::string filename = parametersNode["file"].as<std::string>();
    std::cout << "parameters filename: " << filename << std::endl;
    std::string path = basePathOf(filePath);

    // TODO: Factor out some file format reading handler.
    //       Fileformat could be a key for map to loading function.
    std::cout << "Loading " << format << " from " << path + filename << std::endl;
    auto columnNames = HDProcess::loadCSVColumnNames(path + filename);

    for (auto name : columnNames) {
      std::cout << "Loading QOI: " << name << std::endl;
      auto parameter = HDProcess::loadCSVColumn(path + filename, name);
      parameters.push_back(ParameterNameValuePair(name, parameter));
    }
  }

  return parameters;
}


ParameterNameValuePair DatasetLoader::parseParameter(
    const YAML::Node &parameterNode, const std::string &filePath) {
  if (!parameterNode["name"]) {
    throw std::runtime_error("Parameter missing 'name' field.");
  }
  std::string name = parameterNode["name"].as<std::string>();
  std::cout << "parameter name: " << name << std::endl;

  if (!parameterNode["format"]) {
     throw std::runtime_error("Parameter missing 'format' field.");
  }
  std::string format = parameterNode["format"].as<std::string>();
  // TODO: Move Format Strings to typed enums or constants.
  std::cout << "parameter format: " << format << std::endl;
  if (format != "Linalg.DenseVector" &&
      format != "csv") {
    throw std::runtime_error(
        "Dataset config specifies unsupported parameter format: " + format);
  }

  if (!parameterNode["file"]) {
    throw std::runtime_error("Parameter missing 'file' field.");
  }
  std::string filename = parameterNode["file"].as<std::string>();
  std::cout << "parameter filename: " << filename << std::endl;
  std::string path = basePathOf(filePath);

  // TODO: Factor out some file format reading handler.
  //       Fileformat could be a key for map to loading function.
  std::cout << "Loading " << format << " from " << path + filename << std::endl;
  FortranLinalg::DenseVector<Precision> parameter;
  if (format == "Linalg.DenseVector") {
    parameter = FortranLinalg::LinalgIO<Precision>::readVector(path + filename);
  } else if (format == "csv") {
    parameter = HDProcess::loadCSVColumn(path + filename);
  }

  return ParameterNameValuePair(name, parameter);
}

std::vector<QoiNameValuePair> DatasetLoader::parseQois(
    const YAML::Node &config, const std::string &filePath) {
  std::vector<QoiNameValuePair> qois;
  if (!config["qois"]) {
    throw std::runtime_error("Dataset config missing 'qois' field.");
  }
  const YAML::Node &qoisNode = config["qois"];

  if (qoisNode.IsSequence()) {
    // Parse QoIs one by one if in list form:
    std::cout << "Reading sequence of " << qoisNode.size() << " qois." << std::endl;
    for (std::size_t i = 0; i < qoisNode.size(); i++) {
      const YAML::Node &qoiNode = qoisNode[i];
      auto qoi = DatasetLoader::parseQoi(qoiNode, filePath);
      qois.push_back(qoi);
    }
  } else {
    // Parse all Qois from a file if not in list form.
    std::string format = qoisNode["format"].as<std::string>();
    if (format != "csv") {
      throw std::runtime_error(
        "Dataset config specifies unsupported multi-qoi format: " + format);
    }
    if (!qoisNode["file"]) {
      throw std::runtime_error("Qois missing 'file' field.");
    }
    std::string filename = qoisNode["file"].as<std::string>();
    std::cout << "qois filename: " << filename << std::endl;
    std::string path = basePathOf(filePath);

    // TODO: Factor out some file format reading handler.
    //       Fileformat could be a key for map to loading function.
    std::cout << "Loading " << format << " from " << path + filename << std::endl;
    auto columnNames = HDProcess::loadCSVColumnNames(path + filename);

    for (auto name : columnNames) {
      std::cout << "Loading QOI: " << name << std::endl;
      auto qoi = HDProcess::loadCSVColumn(path + filename, name);
      qois.push_back(QoiNameValuePair(name, qoi));
    }
  }

  return qois;
}


QoiNameValuePair DatasetLoader::parseQoi(
    const YAML::Node &qoiNode, const std::string &filePath) {
  if (!qoiNode["name"]) {
    throw std::runtime_error("Qoi missing 'name' field.");
  }
  std::string name = qoiNode["name"].as<std::string>();
  std::cout << "qoi name: " << name << std::endl;

  if (!qoiNode["format"]) {
     throw std::runtime_error("Qoi missing 'format' field.");
  }
  std::string format = qoiNode["format"].as<std::string>();
  // TODO: Move Format Strings to typed enums or constants.
  std::cout << "qoi format: " << format << std::endl;
  if (format != "Linalg.DenseVector" &&
      format != "csv") {
    throw std::runtime_error(
        "Dataset config specifies unsupported qoi format: " + format);
  }

  if (!qoiNode["file"]) {
    throw std::runtime_error("Qoi missing 'file' field.");
  }
  std::string filename = qoiNode["file"].as<std::string>();
  std::cout << "qoi filename: " << filename << std::endl;
  std::string path = basePathOf(filePath);

  // TODO: Factor out some file format reading handler.
  //       Fileformat could be a key for map to loading function.
  std::cout << "Loading " << format << " from " << path + filename << std::endl;
  FortranLinalg::DenseVector<Precision> qoi;
  if (format == "Linalg.DenseVector") {
    qoi = FortranLinalg::LinalgIO<Precision>::readVector(path + filename);
  } else if (format == "csv") {
    qoi = HDProcess::loadCSVColumn(path + filename);
  }

  return QoiNameValuePair(name, qoi);
}


std::vector<EmbeddingPair> DatasetLoader::parseEmbeddings(
    const YAML::Node &config, const std::string &filePath) {
  std::vector<EmbeddingPair> embeddings;
  if (!config["embeddings"]) {
    throw std::runtime_error("Dataset config missing 'embeddings' field.");
  }
  const YAML::Node &embeddingsNode = config["embeddings"];

  if (embeddingsNode.IsSequence()) {
    // Parse Embeddings one by one if in list form:
    std::cout << "Reading sequence of " << embeddingsNode.size()
              << " embeddings." << std::endl;
    for (std::size_t i = 0; i < embeddingsNode.size(); i++) {
      const YAML::Node &embeddingNode = embeddingsNode[i];
      auto embedding = DatasetLoader::parseEmbedding(embeddingNode, filePath);
      embeddings.push_back(embedding);
    }
  } else {
    throw std::runtime_error("Config 'embeddings' field is not a list.");
  }

  return embeddings;
}


EmbeddingPair DatasetLoader::parseEmbedding(
    const YAML::Node &embeddingNode, const std::string &filePath) {
  if (!embeddingNode["name"]) {
    throw std::runtime_error("Embedding missing 'name' field.");
  }
  std::string name = embeddingNode["name"].as<std::string>();
  std::cout << "Embedding name: " << name << std::endl;

  if (!embeddingNode["format"]) {
     throw std::runtime_error("Embedding missing 'format' field.");
  }
  std::string format = embeddingNode["format"].as<std::string>();
  // TODO: Move Format Strings to typed enums or constants.
  std::cout << "Embedding format: " << format << std::endl;
  if (format != "Linalg.DenseMatrix" &&
      format != "csv") {
    throw std::runtime_error(
        "Dataset config specifies unsupported embedding format: " + format);
  }

  if (!embeddingNode["file"]) {
    throw std::runtime_error("Embedding missing 'file' field.");
  }
  std::string filename = embeddingNode["file"].as<std::string>();
  std::cout << "Embedding filename: " << filename << std::endl;
  std::string path = basePathOf(filePath);

  // TODO: Factor out some file format reading handler.
  //       Fileformat could be a key for map to loading function.
  std::cout << "Loading " << format << " from " << path + filename << std::endl;
  FortranLinalg::DenseMatrix<Precision> embedding;
  if (format == "Linalg.DenseMatrix") {
    embedding = FortranLinalg::LinalgIO<Precision>::readMatrix(path + filename);
  } else if (format == "csv") {
    embedding = HDProcess::loadCSVMatrix(path + filename);
  }

  return EmbeddingPair(name, embedding);
}

// There is a model for each crystal of each persistence level of a M-S complex...
//
// In the config.yaml for the dataset:
// models:
//   - fieldname: Max Stress
//     type: shapeodds                                            # could be shapeodds, pca, sharedgp, custom
//     python_evaluator: None                                     # custom evaluator classname and its module
//     python_renderer: [VolumeRenderer, data.thumbnails]         # custom renderer classname and its module
//     python_renderer: [RendererName, module, meshname.ply]      # for meshes meshname used to initialize connected mesh faces
//     root: shapeodds_models_maxStress                           # directory of models for this field
//     persistences: persistence-?                                # persistence files
//     crystals: crystal-?                                        # in each persistence dir are its crystals
//     padZeroes: false                                           # for both persistence and crystal dirs/files
//     partitions: CantileverBeam_CrystalPartitions_maxStress.csv # has 20 lines of varying length and 20 persistence levels
//     rotate: false                                              # the shape produced by this model needs to be rotated 90 degrees
//     ms:                                                        # Morse-Smale parameters used to compute partitions
//      - knn: 15                                                 # k-nearest neighbors
//      - sigma: 0.25                                             # 
//      - smooth: 15.0                                            # 
//      - depth: 20                                               # num persistence levels; -1 means compute them all
//      - noise: true                                             # add mild noise to the field to ensure inequality
//      - curvepoints: 50                                         # vis only? Not sure if this matters for crystal partitions 
//      - normalize: false                                        # vis only? Not sure if this matters for crystal partitions
//     params:                                                    # model interpolation parameters used
//      - sigma: 0.15                                             # Gaussian width
//
ModelMap DatasetLoader::parseModels(const YAML::Node &config, const std::string &filePath)
{
  ModelMap models;

  if (config["models"] && config["models"].IsSequence()) {
    const YAML::Node &modelsNode = config["models"];
    std::cout << "Reading " << modelsNode.size() << " model sets..." << std::endl;

    for (auto i = 0; i < modelsNode.size(); i++) {
      const YAML::Node &modelNode = modelsNode[i];
      
      // load 
      time_point<Clock> start = Clock::now();
      auto modelset(DatasetLoader::parseModel(modelNode, filePath));
      time_point<Clock> end = Clock::now();
      milliseconds diff = duration_cast<milliseconds>(end - start);
      std::cout << "Loaded " << i << "th modelset in " << static_cast<float>(diff.count())/1000.0f << "s" << std::endl;
      
      if (modelset) {
        // ensure modelset has a unique name in the set of modelsets for this field and add it (TODO: better name, see issue)
        auto num_of_type_for_this_field = std::count_if(models[modelset->fieldName()].begin(), models[modelset->fieldName()].end(),
                                   [&modelset](std::shared_ptr<MSModelset> m) { return m->modelType() == modelset->modelType(); });
        if (num_of_type_for_this_field > 0) // results in PCA, PCA2, PCA3, ...
          modelset->setModelName(modelset->modelName() + std::to_string(num_of_type_for_this_field + 1));
        
        models[modelset->fieldName()].push_back(std::move(modelset));
      }
      else
        std::cerr << "Unknown error reading " << i <<"th modelNode. Skipping it.\n";
    }
  }
  else
    std::cerr << "Config 'models' field missing or not a list.\n";

  return models;
}

/*
 * Sets the parameters used to compute the M-S in which these models reside.
 * (technically, the M-S that partitioned the data with which these models were learned)
 */
bool setMSParams(MSModelset& modelset, const YAML::Node& ms) {
  if (ms["knn"] && ms["sigma"] && ms["smooth"] && ms["curvepoints"] && ms["depth"] && ms["noise"] && ms["normalize"]) {
    auto knn         = ms["knn"].as<int>();
    auto sigma       = ms["sigma"].as<double>();
    auto smooth      = ms["smooth"].as<double>();
    auto curvepoints = ms["curvepoints"].as<int>();
    auto depth       = ms["depth"].as<int>();
    auto noise       = ms["noise"].as<bool>();
    auto normalize   = ms["normalize"].as<bool>();
    std::cout << "\tknn:         " << knn         << std::endl
              << "\tsigma:       " << sigma       << std::endl
              << "\tsmooth:      " << smooth      << std::endl
              << "\tcurvepoints: " << curvepoints << std::endl
              << "\tdepth:       " << depth       << std::endl
              << "\tnoise:       " << noise       << std::endl
              << "\tnormalize:   " << normalize   << std::endl;
    modelset.setParams(knn, sigma, smooth, curvepoints, depth, noise, normalize);
    return true;
  }
  return false;
}

std::unique_ptr<MSModelset> DatasetLoader::parseModel(const YAML::Node& modelNode, const std::string& filePath)
{
  if (!modelNode["root"]) {
    std::cerr << "Model missing 'root' field.\n";
    return nullptr;
  }
  std::string modelBasePath = basePathOf(filePath) + '/' + modelNode["root"].as<std::string>();

  if (!modelNode["fieldname"]) {
    std::cerr << "Model entry missing 'fieldname'.\n";
    return nullptr;
  }
  std::string fieldname = modelNode["fieldname"].as<std::string>();

  if (!modelNode["type"]) {
    std::cerr << "Models entry missing 'type' field.\n";
    return nullptr;
  }
  Model::Type modelType = Model::strToType(modelNode["type"].as<std::string>());
  std::cout << "Reading a " << modelType << " model for '" << fieldname << "' field." << std::endl;

  std::vector<std::string> python_evaluator;
  if (modelNode["python_evaluator"]) {
    python_evaluator = modelNode["python_evaluator"].as<std::vector<std::string>>();
    std::cout << "Model provides custom python thumbnail evaluator\n";
  }

  std::vector<std::string> python_renderer;
  if (modelNode["python_renderer"]) {
    python_renderer = modelNode["python_renderer"].as<std::vector<std::string>>();
    if (python_renderer.size() > 2) {
      // first arg is assumed to be default filename to initialize renderer
      python_renderer[2] = basePathOf(filePath) + '/' + python_renderer[2];
    }
    std::cout << "Model provides custom python thumbnail renderer\n";
  }

  if (!modelNode["partitions"]) {
    std::cerr << "Model missing 'partitions' field (specifyies samples for the crystals at each persistence level).\n";
    return nullptr;
  }
  std::string partitions = modelNode["partitions"].as<std::string>();
  std::string partitions_suffix = partitions.substr(partitions.rfind('.')+1);
  InputFormat partitions_format = InputFormat(partitions_suffix);
  std::cout << "Partitions file format: " << partitions_format << std::endl;

  if (!modelNode["persistences"]) {
    std::cerr << "Model missing 'persistences' field specifying base filename for persistence levels.\n";
    return nullptr;
  }
  std::string persistencesBase = modelNode["persistences"].as<std::string>();
  int persistenceNameLoc = persistencesBase.find('?');
  std::string persistencesBasePath = modelBasePath + '/' + persistencesBase.substr(0, persistenceNameLoc);

  if (!modelNode["crystals"]) {
    std::cerr << "Model missing 'crystals' field specifying base filename for the crystals at each level.\n";
    return nullptr;
  }
  std::string crystalsBasename = modelNode["crystals"].as<std::string>();
  int crystalIndexLoc = crystalsBasename.find('?');
  crystalsBasename = crystalsBasename.substr(0, crystalIndexLoc);

  bool padIndices = false;
  if (modelNode["padZeroes"]) {
    std::string padZeroes = modelNode["padZeroes"].as<std::string>();
    if (padZeroes == "true") {
      padIndices = true;
    } else if (padZeroes != "false") {
      std::cerr << "Model's padZeroes contains invalid value: " << padZeroes << std::endl;
      return nullptr;
    }
  }

  bool rotate = false;
  if (modelNode["rotate"] && modelNode["rotate"].as<std::string>() == "true") {
    rotate = true;
  }

  // crystalPartitions: array of P persistence levels x N samples per level, indicating the crystal to which each sample belongs
  auto partition_file = modelBasePath + '/' + partitions;
  Eigen::MatrixXi crystalPartitions(IO::readCSVMatrix<int>(partition_file));
  auto npersistences = crystalPartitions.rows(), nsamples = crystalPartitions.cols();

  // create the modelset and read its M-S computation parameters (MUST be specified or misalignment of results)
  auto ms_of_models(std::make_unique<MSModelset>(modelType, fieldname, nsamples, npersistences, rotate));
  std::cout << "Models for each crystal of top " << npersistences << "plvls of M-S computed from " << nsamples << " samples using:\n";
  if (!(modelNode["ms"] && setMSParams(*ms_of_models, modelNode["ms"]))) {
    std::cerr << "Error: model missing M-S computation parameters used for its crystal partitions.\n";
    return nullptr;
  }
  ms_of_models->setCustomEvaluator(python_evaluator);
  ms_of_models->setCustomRenderer(python_renderer);

  int plvl_start = 0;
  if (modelNode["first_partition"]) {
    plvl_start = modelNode["first_partition"].as<int>();
  }

  // read paths to all the models (the models themselves are read on demand)
  for (auto pidx = npersistences-1; pidx >= 0; pidx--)
  {
    auto &P = ms_of_models->getPersistenceLevel(pidx);
    auto persistencePath(persistencesBasePath + maybePadIndex(plvl_start + pidx, padIndices, npersistences));

    // use crystalPartitions to determine number of crystals at this level
    auto ncrystals = crystalPartitions.row(pidx).maxCoeff()+1;
    P.setNumCrystals(ncrystals);

    // read crystalIds indicating to which crystal each sample belongs at this persistencej
    P.setCrystalSampleIds(crystalPartitions.row(pidx));

    // read the model path for each crystal
    for (unsigned crystal = 0; crystal < ncrystals; crystal++) {
      std::string crystalPath(persistencePath + '/' + crystalsBasename + maybePadIndex(crystal, padIndices, ncrystals));
      P.crystals[crystal].setModelPath(crystalPath);
    }
  }

  return ms_of_models;
}

// read the components of each model (Z, W, w0) from their respective bin or csv files
void DatasetLoader::parseModel(const std::string& modelPath, Model& m,
                               const std::vector<ValueIndexPair> &samples)
{
  // read W
  Eigen::MatrixXf W, w0, Z;
  if (IO::fileExists(modelPath + "/W.bin"))
    W = IO::readBinMatrix<Precision>(modelPath + "/W.bin");
  else
    W = IO::readCSVMatrix<Precision>(modelPath + "/W.csv");

  // read w0
  if (IO::fileExists(modelPath + "/w0.bin"))
    w0 = IO::readBinMatrix<Precision>(modelPath + "/w0.bin");
  else
    w0 = IO::readCSVMatrix<Precision>(modelPath + "/w0.csv");

  // read latent space Z
  if (IO::fileExists(modelPath + "/Z.bin"))
    Z = IO::readBinMatrix<Precision>(modelPath + "/Z.bin");
  else
    Z = IO::readCSVMatrix<Precision>(modelPath + "/Z.csv");

  // ShapeOdds models have z_coords for all samples, not only those used for their construction
  if (m.getType() == Model::ShapeOdds) {
    Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic> z_coords(samples.size(), Z.cols());
    auto i = 0;
    for (auto sample : samples)
       z_coords.row(i++) = Z.row(sample.idx);
    Z = z_coords;
  }
  
  m.setModel(W, w0, Z);
}

FortranLinalg::DenseMatrix<Precision> DatasetLoader::parseGeometry(
    const YAML::Node &config, const std::string &filePath) {
  if(!config["geometry"]) {
    throw std::runtime_error("Dataset config missing 'geometry' field.");
  }
  const YAML::Node &geometryNode = config["geometry"];

  if (!geometryNode["format"]) {
    throw std::runtime_error("Dataset config missing 'geometry.format' field.");
  }
  std::string format = geometryNode["format"].as<std::string>();
  if (format != "Linalg.DenseMatrix") {
    std::cout << "WARNING: " <<
          "Dataset config specifies unsupported geometry format: " << format;
    return FortranLinalg::DenseMatrix<Precision>();
  }

  if (!geometryNode["file"]) {
    throw std::runtime_error("Dataset config missing 'geometry.file' field.");
  }
  std::string filename = geometryNode["file"].as<std::string>();
  std::string path = basePathOf(filePath);

  auto x = FortranLinalg::LinalgIO<Precision>::readMatrix(path + filename);
  return x;
}


FortranLinalg::DenseMatrix<Precision> DatasetLoader::parseDistances(
    const YAML::Node &config, const std::string &filePath) {
  if(!config["distances"]) {
    throw std::runtime_error("Dataset config missing 'distances' field.");
  }
  const YAML::Node &distancesNode = config["distances"];

  if (!distancesNode["format"]) {
    throw std::runtime_error("Dataset config missing 'distances.format' field.");
  }
  std::string format = distancesNode["format"].as<std::string>();
  if (format != "Linalg.DenseMatrix" &&
      format != "csv" &&
      format != "bin") {
    throw std::runtime_error(
        "Dataset config specifies unsupported distances format: " + format);
  }

  if (!distancesNode["file"]) {
    throw std::runtime_error("Dataset config missing 'distances.file' field.");
  }
  std::string filename = distancesNode["file"].as<std::string>();
  std::string path = basePathOf(filePath);

  // TODO: Factor out some file format reading handler.
  //       Fileformat could be a key for map to loading function.
  std::cout << "Loading " << format << " from " << path + filename << std::endl;
  if (format == "Linalg.DenseVector") {
    auto distances = FortranLinalg::LinalgIO<Precision>::readMatrix(path + filename);
		return distances;
  } else if (format == "csv") {
    auto distances = HDProcess::loadCSVMatrix(path + filename);
		return distances;
  } else if (format == "bin") {
    auto distances = IO::readBinMatrix<Precision, Eigen::ColMajor>(path + filename);
		return toDenseMatrix<Precision>(distances);
  } else {
		throw std::runtime_error(
		  	"No loader configured to read " + format + " qoi file.");
	}
}

std::string DatasetLoader::createThumbnailPath(const std::string& imageBasePath, int index,
    const std::string imageSuffix, unsigned int indexOffset,
    bool padZeroes, unsigned int thumbnailCount) {
  std::string imageName = std::to_string(index);

  if (padZeroes) {
    unsigned digitCount = paddedStringWidth(indexOffset + thumbnailCount);
    imageName = paddedIndexString(index, digitCount);
  }

  std::string path = imageBasePath + imageName + imageSuffix;
  return path;
}

std::vector<Image> DatasetLoader::parseThumbnails(
    const YAML::Node &config, const std::string &filePath) {
  if (!config["thumbnails"]) {
    throw std::runtime_error("Dataset config missing 'thumbnails' field.");
  }
  const YAML::Node &thumbnailsNode = config["thumbnails"];

  if (!thumbnailsNode["format"]) {
    throw std::runtime_error("Dataset config missing 'thumbnails.format' field.");
  }
  std::string format = thumbnailsNode["format"].as<std::string>();
  if (format != "png") {
    throw std::runtime_error("Dataset config specifies unsupported thumbnails format: " + format);
  }

  if (!thumbnailsNode["files"]) {
    throw std::runtime_error("Dataset config missing 'thumbnails.files' field.");
  }
  std::string imagePath = thumbnailsNode["files"].as<std::string>();
  int imageNameLoc = imagePath.find('?');
  std::string imageBasePath = basePathOf(filePath) + imagePath.substr(0, imageNameLoc);
  std::string imageSuffix = imagePath.substr(imageNameLoc + 1);

  bool padIndices = false;
  if (thumbnailsNode["padZeroes"]) {
    std::string padZeroes = thumbnailsNode["padZeroes"].as<std::string>();
    if (padZeroes == "true") {
      padIndices = true;
    } else if (padZeroes != "false") {
      throw std::runtime_error("Dataset's padZeroes contains invalid value: " + padZeroes);
    }
  }

  unsigned int indexOffset = 0;
  if (thumbnailsNode["offset"]) {
    indexOffset = thumbnailsNode["offset"].as<int>();
  }

  unsigned int thumbnailCount = parseSampleCount(config);
  std::vector<Image> thumbnails;
  for (int i = 0; i < thumbnailCount; i++) {
    std::string path = createThumbnailPath(imageBasePath, i+indexOffset,
      imageSuffix, indexOffset, padIndices, thumbnailCount);
    std::cout << "Loading image: " << path << std::endl;
    Image image(path, false/*decompress*/);
    thumbnails.push_back(image);
  }

  return thumbnails;
}

