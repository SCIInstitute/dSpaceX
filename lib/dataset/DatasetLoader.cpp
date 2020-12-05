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

namespace dspacex {

struct InputFormat
{
  enum Type { BIN, CSV, JSON, LINALG_DENSEMATRIX, LINALG_DENSEVECTOR, INVALID = -1 };
  Type type{INVALID};

  bool operator==(InputFormat f) const { return f.type == type; }
  bool operator!=(InputFormat f) const { return !(f.type == type); }
  bool operator==(InputFormat::Type f) const { return f == type; }
  bool operator!=(InputFormat::Type f) const { return !(f == type); }

  InputFormat(std::string filename) {
    auto ext = filename.substr(filename.rfind('.')+1);

    if (!ext.compare("csv"))
      type = CSV;
    else if (!ext.compare("json"))
      type = JSON;
    else if (!ext.compare("bin"))
      type = BIN;
    else if (ext == "hdr") {
      if (FortranLinalg::LinalgIO<Precision>::isDenseVectorHeader(filename))
        type = LINALG_DENSEVECTOR;
      else if (FortranLinalg::LinalgIO<Precision>::isDenseMatrixHeader(filename))
        type = LINALG_DENSEMATRIX;
    }
  }

  operator std::string() const {
    switch (type)
    {
      case InputFormat::BIN:                return "bin";
      case InputFormat::CSV:                return "csv";
      case InputFormat::JSON:               return "json";
      case InputFormat::LINALG_DENSEMATRIX: return "Linalg.DenseMatrix";
      case InputFormat::LINALG_DENSEVECTOR: return "Linalg.DenseVector";
      default:                              return "INVALID InputFormat";
    }
  }
  
  bool valid() const {
    return (type == Type::CSV || type == Type::BIN ||type == Type::JSON ||
            type == Type::LINALG_DENSEVECTOR || type == Type::LINALG_DENSEMATRIX);
  }
  
  friend std::ostream& operator<<(std::ostream &out, const InputFormat &c); 
};

std::ostream& operator<<(std::ostream &out, const InputFormat &f) 
{ 
  out << std::string(f);
  return out; 
} 

std::unique_ptr<Dataset> DatasetLoader::loadDataset(const std::string &basePath) {
  YAML::Node config = YAML::LoadFile(basePath);
  DatasetBuilder builder;

  std::cout << "Reading " << basePath << std::endl;

  std::string name = DatasetLoader::parseName(config);
  std::cout << "name: " << name << std::endl;
  builder.withName(name);

  int sampleCount = DatasetLoader::parseSampleCount(config);
  std::cout << "samples: " << sampleCount << std::endl;
  builder.withSampleCount(sampleCount);

  if (config["parameters"]) {
    auto parameters = DatasetLoader::parseParameters(config, basePath);
    for (auto parameter : parameters) {
      builder.withParameter(parameter.first, parameter.second);
    }
  }

  if (config["qois"]) {
    auto qois = DatasetLoader::parseQois(config, basePath);
    for (auto qoi : qois) {
      builder.withQoi(qoi.first, qoi.second);
    }
  }
  
  if (config["geometry"]) {
    auto geometry = DatasetLoader::parseGeometry(config, basePath);
    builder.withGeometryMatrix(geometry);
  }

  if (config["distances"]) {
    auto distances = DatasetLoader::parseDistances(config, basePath);
    builder.withDistances(distances);
  }

  if (config["embeddings"]) {
    auto embeddings = DatasetLoader::parseEmbeddings(config, basePath);
    for (auto embedding : embeddings) {
      builder.withEmbeddings(embedding.first, embedding.second);
    }
  }

  if (config["modelsets"]) {
    auto metricModelsets = DatasetLoader::parseMetricModelsets(config, basePath);
    for (auto modelsets : metricModelsets) {
      builder.withModelsets(modelsets.first, modelsets.second);
    }
  }

  if (config["thumbnails"]) {
    auto thumbnails = DatasetLoader::parseThumbnails(config, basePath);
    builder.withThumbnails(thumbnails);
  }

  return builder.build();
}

std::string DatasetLoader::getDatasetName(const std::string &basePath) {
  YAML::Node config = YAML::LoadFile(basePath);
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


std::vector<FieldNameValuePair> DatasetLoader::parseFields(const YAML::Node &node, const std::string &basePath) {

  std::vector<FieldNameValuePair> fields;

  if (!node["file"]) {
    throw std::runtime_error("Node missing 'file' field.");
  }
  auto filename = node["file"].as<std::string>();
  std::cout << "fields filename: " << filename << std::endl;

  auto format = InputFormat(filename);
  if (format != InputFormat::CSV) {
    throw std::runtime_error("Dataset config specifies unsupported multi-field format: " + std::string(format));
  }
  auto columnNames = HDProcess::loadCSVColumnNames(filepath(basePath, filename));

  for (auto name : columnNames) {
    std::cout << "Loading field: " << name << std::endl;
    auto field = HDProcess::loadCSVColumn(filepath(basePath, filename), name);
    fields.push_back(FieldNameValuePair(name, field));
  }

  return fields;
}

std::vector<FieldNameValuePair> DatasetLoader::parseParameters(const YAML::Node &config, const std::string &basePath) {

  if (!config["parameters"]) {
    throw std::runtime_error("Dataset config missing 'parameters' field.");
  }
  return parseFields(config["parameters"], basePath);
}

std::vector<FieldNameValuePair> DatasetLoader::parseQois(const YAML::Node &config, const std::string &basePath) {

  if (!config["qois"]) {
    throw std::runtime_error("Dataset config missing 'qois' field.");
  }
  return parseFields(config["qois"], basePath);
}


std::map<std::string, std::vector<EmbeddingPair>> DatasetLoader::parseEmbeddings(const YAML::Node &config, const std::string &basePath) {

  if (!config["embeddings"] || !config["embeddings"].IsSequence()) {
    throw std::runtime_error("Config 'embeddings' field missing or not a list.");
  }

  std::map<std::string, std::vector<EmbeddingPair>> embeddingsMap;
  const YAML::Node &embeddingsNode = config["embeddings"];
  for (std::size_t i = 0; i < embeddingsNode.size(); i++) {
    const YAML::Node &metricEmbeddingsNode = embeddingsNode[i];
    auto metric = metricEmbeddingsNode["metric"].as<std::string>();

    auto list = metricEmbeddingsNode["embeddings"];
    if (!list || !list.IsSequence()) {
      throw std::runtime_error("embeddings' embeddings list is missing.");
    }
    std::vector<EmbeddingPair> embeddings;
    for (auto j = 0; j < list.size(); j++) {
      embeddings.push_back(DatasetLoader::parseEmbedding(list[j], basePath));
    }
    embeddingsMap[metric] = embeddings;
  }

  return embeddingsMap;
}


EmbeddingPair DatasetLoader::parseEmbedding(const YAML::Node &embeddingNode, const std::string &basePath) {
  if (!embeddingNode["name"]) {
    throw std::runtime_error("Embedding missing 'name' field.");
  }
  std::string name = embeddingNode["name"].as<std::string>();
  std::cout << "Embedding name: " << name << std::endl;

  if (!embeddingNode["file"]) {
    throw std::runtime_error("Embedding missing 'file' field.");
  }
  std::string filename = embeddingNode["file"].as<std::string>();

  auto format = InputFormat(filename);
  std::cout << "Loading " << format << " from embedding filename: " << filename << std::endl;
  FortranLinalg::DenseMatrix<Precision> embedding;
  switch (format.type) {
    case InputFormat::LINALG_DENSEMATRIX:
      embedding = FortranLinalg::LinalgIO<Precision>::readMatrix(filepath(basePath, filename));
      break;
    case InputFormat::CSV:
      embedding = HDProcess::loadCSVMatrix(filepath(basePath, filename));
      break;
    default:
      throw std::runtime_error("Dataset config specifies unsupported embedding format: " + std::string(format));
  }

  return EmbeddingPair(name, embedding);
}

/*
 * There is a model for each crystal of each persistence level of a M-S complex...

 In the config.yaml for the dataset:
 modelsets:
   - metric: hamming                                              # a set of modelsets per distance metric
     models:                                                      # a modelset in this set
     - fieldname: Max Stress                                      # a model in this modelset
       type: shapeodds                                            # could be shapeodds, pca, sharedgp, custom
       python_evaluator: None                                     # custom evaluator classname and its module
       python_renderer: [VolumeRenderer, data.thumbnails]         # custom renderer classname and its module
       python_renderer: [RendererName, module, meshname.ply]      # for meshes meshname used to initialize connected mesh faces
       root: shapeodds_models_maxStress                           # directory of models for this field
       persistences: persistence-?                                # persistence files
       crystals: crystal-?                                        # in each persistence dir are its crystals
       padZeroes: false                                           # for both persistence and crystal dirs/files
       partitions: CantileverBeam_CrystalPartitions_maxStress.csv # has 20 lines of varying length and 20 persistence levels
       rotate: false                                              # the shape produced by this model needs to be rotated 90 degrees
       ms:                                                        # Morse-Smale parameters used to compute partitions
        - knn: 15                                                 # k-nearest neighbors
        - sigma: 0.25                                             # 
        - smooth: 15.0                                            # 
        - depth: 20                                               # num persistence levels; -1 means compute them all
        - noise: true                                             # add mild noise to the field to ensure inequality
        - curvepoints: 50                                         # vis only? Not sure if this matters for crystal partitions 
        - normalize: false                                        # vis only? Not sure if this matters for crystal partitions
       params:                                                    # model interpolation parameters used
        - sigma: 0.15                                             # Gaussian width
     - fieldname: Max Stress                                      # can have more than one model per field
       ...
     - fieldname: Confidence
       ...
   - metric: l2
     models:
     - fieldname: Max Stress
     - fieldname: Max Stress
       ...
     - fieldname: Confidence
       ...     

 Please see documentation/configuration.md for more details of config.yaml layout.
*/
std::vector<ModelMapPair> DatasetLoader::parseMetricModelsets(const YAML::Node &config, const std::string &basePath)
{
  std::vector<ModelMapPair> modelmaps;
  if (!config["modelsets"]) {
    std::cout << "no models specified\n";
    return modelmaps;
  }
  auto modelsetsNode = config["modelsets"];
  if (!modelsetsNode.IsSequence()) {
    throw std::runtime_error("Config 'modelsets' node must be a list of modelsets, each with a unique distance metric");
  }

  for (auto i = 0; i < modelsetsNode.size(); i++) {
    auto metric = modelsetsNode[i]["metric"].as<std::string>();
    auto modelsNode = modelsetsNode[i]["models"];
    if (!modelsNode || !modelsNode.IsSequence()) {
      std::cerr << "Error: each entry in 'modelsets' must contain a list of models. Skipping\n";
      continue;
    }
    auto modelmap = parseModels(modelsNode, basePath);
    modelmaps.push_back(ModelMapPair(metric, modelmap));
  }
  return modelmaps;
}

ModelMap DatasetLoader::parseModels(const YAML::Node &modelsNode, const std::string &basePath)
{
  ModelMap modelsets;
  std::cout << "Reading " << modelsNode.size() << " model sets..." << std::endl;

  for (auto i = 0; i < modelsNode.size(); i++) {
    const YAML::Node &modelsetNode = modelsNode[i];
    
    time_point<Clock> start = Clock::now();
    
    auto modelset(DatasetLoader::parseModelset(modelsetNode, basePath));
    
    time_point<Clock> end = Clock::now();
    milliseconds diff = duration_cast<milliseconds>(end - start);
    std::cout << "Loaded " << i << "th modelset of current metric in "
              << static_cast<float>(diff.count())/1000.0f << "s" << std::endl;
      
    if (!modelset) {
      std::cout << "Error: " << i << "th modelset of current metric invalid." << std::endl;
      continue;
    }
    
    // ensure modelset has a unique name in the set of modelsets for this field and add it (TODO: better name)
    auto num = std::count_if(modelsets[modelset->fieldName()].begin(), modelsets[modelset->fieldName()].end(),
                             [&modelset](std::shared_ptr<MSModelset> m) { return m->modelType() == modelset->modelType(); });
    if (num > 0) // results in PCA, PCA2, PCA3, ...
      modelset->setModelName(modelset->modelName() + std::to_string(num + 1));
        
    modelsets[modelset->fieldName()].push_back(std::move(modelset));
  }

  return modelsets;
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

std::unique_ptr<MSModelset> DatasetLoader::parseModelset(const YAML::Node& modelNode, const std::string& basePath)
{
  if (!modelNode["root"]) {
    std::cerr << "Model missing 'root' field.\n";
    return nullptr;
  }
  std::string modelBasePath = filepath(basePath, modelNode["root"].as<std::string>());

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
      python_renderer[2] = filepath(basePath, python_renderer[2]);
    }
    std::cout << "Model provides custom python thumbnail renderer\n";
  }

  if (!modelNode["partitions"]) {
    std::cerr << "Model missing 'partitions' field (specifyies samples for the crystals at each persistence level).\n";
    return nullptr;
  }
  std::string partitions = modelNode["partitions"].as<std::string>();
  auto partitions_format = InputFormat(partitions);
  std::cout << "Partitions file format: " << partitions_format << std::endl;
  if (partitions_format != InputFormat::CSV) {
    std::cerr << "Partitions must be provided as a CSV (TODO: handle .bins)\n";
    return nullptr;
  }

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
    const YAML::Node &config, const std::string &basePath) {
  if(!config["geometry"]) {
    throw std::runtime_error("Dataset config missing 'geometry' field.");
  }
  const YAML::Node &geometryNode = config["geometry"];

  if (!geometryNode["file"]) {
    throw std::runtime_error("Dataset config missing 'geometry.file' field.");
  }
  std::string filename = geometryNode["file"].as<std::string>();
  auto format = InputFormat(filename);
  if (format != InputFormat::LINALG_DENSEMATRIX) {
    std::cout << "WARNING: " <<
          "Dataset config specifies unsupported geometry format: " << format;
    return FortranLinalg::DenseMatrix<Precision>();
  }

  auto x = FortranLinalg::LinalgIO<Precision>::readMatrix(filepath(basePath, filename));
  return x;
}


std::vector<DistancePair> DatasetLoader::parseDistances(const YAML::Node &config, const std::string &basePath) {

  if (!config["distances"] || !config["distances"].IsSequence()) {
    throw std::runtime_error("Config 'distances' field missing or not a list.");
  }

  std::vector<DistancePair> distances;
  const YAML::Node &distancesNode = config["distances"];
  for (auto i = 0; i < distancesNode.size(); i++) {
    const YAML::Node &node = distancesNode[i];

    if (!node["metric"]) {
      throw std::runtime_error("Dataset config missing 'distances.metric' field.");
    }
    std::string metric = node["metric"].as<std::string>();

    if (!node["file"]) {
      throw std::runtime_error("Dataset config missing 'distances.file' field.");
    }
    std::string filename = node["file"].as<std::string>();

    auto format = InputFormat(filename);
    std::cout << "Loading " << format << " from distances filename " << filename << std::endl;
    switch (format.type) {
      case InputFormat::LINALG_DENSEMATRIX:
        distances.push_back(DistancePair(metric, FortranLinalg::LinalgIO<Precision>::readMatrix(filepath(basePath, filename))));
        break;
      case InputFormat::CSV:
        distances.push_back(DistancePair(metric, HDProcess::loadCSVMatrix(filepath(basePath, filename))));
        break;
      case InputFormat::BIN:
      {
        auto dist = IO::readBinMatrix<Precision, Eigen::ColMajor>(filepath(basePath, filename));
        distances.push_back(DistancePair(metric, toDenseMatrix<Precision>(dist)));
      }
        break;
      default:
        throw std::runtime_error("Dataset config specifies unsupported distances format: " + std::string(format));
    }
  }

  return distances;
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
    const YAML::Node &config, const std::string &basePath) {
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
  std::string imageBasePath = filepath(basePath, imagePath.substr(0, imageNameLoc));
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


///////////////////////////////////////////////////////////////////////////////
// DatasetBuilder

std::unique_ptr<Dataset> DatasetBuilder::build()
{
  if (!m_dataset)
    throw std::runtime_error("Dataset in DatasetBuilder is not valid.");

  return std::move(m_dataset); // std::move releases ownership of m_dataset
}

DatasetBuilder& DatasetBuilder::withSampleCount(int count) {
  m_dataset->m_sampleCount = count;
  return (*this);
}

DatasetBuilder& DatasetBuilder::withGeometryMatrix(FortranLinalg::DenseMatrix<Precision> &geometryMatrix) {
  m_dataset->m_geometryMatrix = geometryMatrix;
  m_dataset->m_hasGeometryMatrix = true;
  return (*this);
}

DatasetBuilder& DatasetBuilder::withDistances(std::vector<DistancePair>& distances) {
  for (auto dist : distances) {
    auto metric = dist.first;
    m_dataset->m_distanceMetricNames.push_back(metric);
    m_dataset->m_distances[metric] = dist.second;
  }
  return (*this);
}

DatasetBuilder& DatasetBuilder::withParameter(std::string name, FortranLinalg::DenseVector<Precision> &parameter) {
  m_dataset->m_parameterNames.push_back(name);
  m_dataset->m_parameters.push_back(parameter);
  m_dataset->m_normalized_parameters.push_back(normalize(parameter));
  return (*this);
}

DatasetBuilder& DatasetBuilder::withQoi(std::string name, FortranLinalg::DenseVector<Precision> &qoi) {
  m_dataset->m_qoiNames.push_back(name);
  m_dataset->m_qois.push_back(qoi);
  m_dataset->m_normalized_qois.push_back(normalize(qoi));
  return (*this);
}

DatasetBuilder& DatasetBuilder::withEmbeddings(std::string metric, std::vector<EmbeddingPair>& embeddings) {
  if (m_dataset->hasDistanceMetric(metric)) {
    for (auto embedding : embeddings) {
      m_dataset->m_embeddingNames[metric].push_back(embedding.first);
      m_dataset->m_embeddings[metric].push_back(embedding.second);
    }
  }
  else {
    throw std::runtime_error("tried to add embeddings for unknown metric " + metric);
  }
  return (*this);
}

DatasetBuilder& DatasetBuilder::withModelsets(std::string metric, ModelMap& modelmap) {
  if (!m_dataset->hasDistanceMetric(metric)) {
    throw std::runtime_error("tried to add modelsets for unknown metric " + metric);
  }

  // add the fields that have modelssets for this metric
  for (auto models : modelmap) {
    auto fieldname = models.first;
    m_dataset->m_msModelFields[metric].push_back(fieldname);
    for (auto modelset : models.second) {
      modelset->setFieldvals(m_dataset->getFieldvalues(fieldname, Fieldtype::Unknown, false));
      m_dataset->m_models[metric][fieldname].push_back(modelset);
    }
  }
  return (*this);
}

DatasetBuilder& DatasetBuilder::withName(std::string name) {
  m_dataset->m_name = name;
  return (*this);
}

DatasetBuilder& DatasetBuilder::withThumbnails(std::vector<Image> thumbnails) {
  m_dataset->m_thumbnails = thumbnails;
  return (*this);
}


} // dspacex
