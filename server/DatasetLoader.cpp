#include "DatasetLoader.h"
#include "utils/loaders.h"
#include "imageutils/ImageLoader.h"
#include "yaml-cpp/yaml.h"
#include "utils/StringUtils.h"
#include "utils/IO.h"

#include <memory>
#include <iomanip>
#include <sstream>
#include <vector>

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
    auto ms_models = DatasetLoader::parseMSModels(config, filePath);
    for (auto ms_model : ms_models) { //<ctc> this one i'm less understanding... maybe because it's part of a pair, which is an lvalue
      builder.withMSModel(ms_model.first, ms_model.second);
      //<ctc> getting there, just putting this part on hold while I fix the indexing bug
    // for (auto ms_model : DatasetLoader::parseMSModels(config, filePath)) {
    //   builder.withMSModel(ms_model.getFieldname(), /*std::move(ms_model)*/ ms_model); // it just passes it right along. Still needs move?
    }
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

// There is a model for each crystal of each persistence level of a M-S complex... maybe too
// much to read all at once, so only read them on demand? Anyway, for now just read them all.
//
// This was the addition to the config.yaml for the dataset:
//
// models:  # seems a little tedious to write out list of models, or even a list of persistences. The latter can be determined from the CrystalPartitions csv, which can also indicate how many crystals each level contains (0-based in this file, but 1-based in each model's crystalIds.csv)
//   - fieldname: maxStress
//     type: shapeodds                                            # could be shapeodds or sharedgp
//     root: shapeodds_models_maxStress                           # directory of models for this field
//     persistences: persistence-?                                # persistence files
//     crystals: crystal-?                                        # in each persistence dir are its crystals
//     padZeroes: false                                           # for both persistence and crystal dirs/files
//     #format: csv   (just use extension of most files to determine format) # lots of csv files in each crystal: Z, crystalIds, W, wo
//     partitions: CantileverBeam_CrystalPartitions_maxStress.csv # has 20 lines of varying length and 20 persistence levels
//     embeddings: shapeodds_global_embedding.csv                 # a tsne embedding? Global for each p-lvl, and local for each crystal
//
std::vector<MSModelsPair> DatasetLoader::parseMSModels(const YAML::Node &config, const std::string &filePath)
{
  std::vector<MSModelsPair> ms_models;

  if (!config["models"]) {
    throw std::runtime_error("Dataset config missing 'models' field.");
  }
  const YAML::Node &modelsNode = config["models"];

  if (modelsNode.IsSequence()) {
    // Parse Models one field at a time if in list form:
    std::cout << "Reading sets of models for " << modelsNode.size() << " field(s)." << std::endl;
    for (std::size_t i = 0; i < modelsNode.size(); i++) {
      const YAML::Node &modelNode = modelsNode[i];
      MSModelsPair ms_model = DatasetLoader::parseMSModelsForField(modelNode, filePath);
      ms_models.push_back(ms_model);
      //<ctc> getting there, just putting this part on hold while I fix the indexing bug
      // MSModelsPair ms_model("Max Stress", DatasetLoader::parseMSModelsForField(modelNode, filePath));
      // ms_models.push_back(std::move(ms_model));
    }
  } else {
    throw std::runtime_error("Config 'models' field is not a list.");
  }

  return ms_models;
}

MSModelsPair DatasetLoader::parseMSModelsForField(const YAML::Node &modelNode, const std::string &filePath)
///*dspacex::MSComplex*/MSModelsPair DatasetLoader::parseMSModelsForField(const YAML::Node &modelNode, const std::string &filePath)
{
  using namespace dspacex;

  if (!modelNode["fieldname"]) {
    throw std::runtime_error("Model missing 'fieldname' field.");
  }
  std::string fieldname = modelNode["fieldname"].as<std::string>();

  if (!modelNode["type"]) {
    throw std::runtime_error("Model missing 'type' field.");
  }
  std::string modelsType = modelNode["type"].as<std::string>();
  std::cout << "Reading '" << modelsType << "' models for '" << fieldname << "' field." << std::endl;

  if (!modelNode["partitions"]) {
     throw std::runtime_error("Model missing 'partitions' field (specifyies samples for the crystals at each persistence level) .");
  }
  std::string partitions = modelNode["partitions"].as<std::string>();
  std::string partitions_suffix = partitions.substr(partitions.rfind('.')+1);
  InputFormat partitions_format = InputFormat(partitions_suffix);
  std::cout << "Partitions file format: " << partitions_format << std::endl;

  if (!modelNode["embeddings"]) {
     throw std::runtime_error("Missing 'embeddings' field (name of global embeddings for each persistence level's models).");
  }
  std::string embeddings = modelNode["embeddings"].as<std::string>();
  std::string embeddings_suffix = embeddings.substr(embeddings.rfind('.')+1);
  InputFormat embeddings_format = InputFormat(embeddings_suffix);
  std::cout << "Global embeddings for each persistence level file format: " << embeddings_format << std::endl;

  if (!modelNode["root"]) {
    throw std::runtime_error("Model missing 'root' field.");
  }
  std::string modelsBasePath = basePathOf(filePath) + modelNode["root"].as<std::string>();

  if (!modelNode["persistences"]) {
    throw std::runtime_error("Model missing 'persistences' field specifying base filename for persistence levels.");
  }
  std::string persistencesBase = modelNode["persistences"].as<std::string>();
  int persistenceNameLoc = persistencesBase.find('?');
  std::string persistencesBasePath = modelsBasePath + '/' + persistencesBase.substr(0, persistenceNameLoc);

  if (!modelNode["crystals"]) {
    throw std::runtime_error("Model missing 'crystals' field specifying base filename for the crystals at each level.");
  }
  std::string crystalsBasename = modelNode["crystals"].as<std::string>();
  int crystalIndexLoc = crystalsBasename.find('?');
  crystalsBasename = crystalsBasename.substr(0, crystalIndexLoc);

  bool shouldPadZeroes = false;
  if (modelNode["padZeroes"]) {
    std::string padZeroes = modelNode["padZeroes"].as<std::string>();
    if (padZeroes == "true") {
      shouldPadZeroes = true;
    } else if (padZeroes != "false") {
      throw std::runtime_error("Model's padZeroes contains invalid value: " + padZeroes);
    }
  }

  // Read crystals, an array of P persistence levels x N samples per level
  // We only read this to determine the number of samples, the number of persistence levels, and the number of crystals per level.
  // <ctc> TODO: actually, this file renders moot all the (ob1) crystalID.csv files, so let's use it instead.
  Eigen::MatrixXi crystalPartitions_eigen;
  if (partitions_format.type == InputFormat::CSV)
    crystalPartitions_eigen = IO::readCSVMatrix<int>(basePathOf(filePath) + '/' + partitions);
  else
  {
    std::stringstream out("Crystal partitions file should be a csv, but it's a ");
    out << partitions_format;
    throw std::runtime_error(out.str());
  }
  unsigned npersistences = crystalPartitions_eigen.rows();
  unsigned nsamples = crystalPartitions_eigen.cols();
  std::cout << "There are " << npersistences << " persistence levels of the M-S complex computed from "
            << nsamples << " samples." << std::endl;

  // Now read all the models
  MSComplex ms_of_models(fieldname, nsamples, npersistences);
  for (unsigned persistence = 0; persistence < npersistences; /*persistence+=10)//*/persistence++) //<ctc> hack to load just a couple lvls for testing
  {
    unsigned persistence_idx = persistence;
    MSPersistenceLevel &P = ms_of_models.getPersistenceLevel(persistence_idx);

    // compute ncrystals for this persistence level using crystalPartitions
    unsigned ncrystals = crystalPartitions_eigen.row(persistence).maxCoeff()+1;
    P.setNumCrystals(ncrystals);
    std::cout << "There are " << ncrystals << " crystals in persistence level " << persistence << std::endl;

    // compute padding if necessary
    unsigned persistenceIndexPadding = 0;
    unsigned crystalIndexPadding = 0;
    if (shouldPadZeroes)
    {
      persistenceIndexPadding = paddedStringWidth(npersistences);
      crystalIndexPadding = paddedStringWidth(ncrystals);
    }

    // read global embeddings for the set of models (one per crystal) at this persistence level
    std::string persistenceIndexStr(shouldPadZeroes ? paddedIndexString(persistence, persistenceIndexPadding) : std::to_string(persistence));
    std::string persistencePath(persistencesBasePath + persistenceIndexStr);
    P.setGlobalEmbeddings(IO::readCSVMatrix<double>(persistencePath + '/' + embeddings));
    
    // read the model for each crystal at this persistence level
    std::string modelPath;
    for (unsigned crystal = 0; crystal < ncrystals; crystal++)
    {
      std::string crystalIndexStr(shouldPadZeroes ? paddedIndexString(crystal, crystalIndexPadding) : std::to_string(crystal));
      std::string crystalPath(persistencePath + '/' + crystalsBasename + crystalIndexStr);
      dspacex::MSCrystal &c = P.getCrystal(crystal);
      c.getModel().setFieldname(fieldname); // <ctc> see TODOs in dspacex::Model
      parseModel(crystalPath, c.getModel());

      modelPath = crystalPath;  // outside this scope because we need to use it to read crystalIds
    }

    // read crystalIds (same for every model at this plevel, so only need to read/set them once)
    Eigen::MatrixXi crystal_ids = IO::readCSVMatrix<int>(modelPath + "/crystalID.csv" );
  
    // FIXME: until fixed in data (produced by MATLAB), crystal ids are 1-based, so adjust them right away to be 0-based
    crystal_ids.array() -= 1.0;
      
    // set group of samples for each model at this persistence level
    P.setCrystalSamples(crystal_ids);
  }

  return MSModelsPair(fieldname, std::move(ms_of_models) /* must explicitly move b/c it's being added to a struct and still could be used locally*/);
  //return ms_of_models;
}

// read the components of each model (Z, W, w0) from their respective csv files
void DatasetLoader::parseModel(const std::string &modelPath, dspacex::Model &m)
{
  // read W
  auto W = IO::readCSVMatrix<double>(modelPath + "/W.csv");

  // read w0
  auto w0 = IO::readCSVMatrix<double>(modelPath + "/w0.csv");

  // read latent space Z
  auto Z = IO::readCSVMatrix<double>(modelPath + "/Z.csv");

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
      format != "csv") {
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

  bool shouldPadZeroes = false;
  if (thumbnailsNode["padZeroes"]) {
    std::string padZeroes = thumbnailsNode["padZeroes"].as<std::string>();
    if (padZeroes == "true") {
      shouldPadZeroes = true;
    } else if (padZeroes != "false") {
      throw std::runtime_error("Dataset's padZeroes contains invalid value: " + padZeroes);
    }
  }

  unsigned int indexOffset = 0;
  if (thumbnailsNode["offset"]) {
    indexOffset = thumbnailsNode["offset"].as<int>();
  }

  ImageLoader imageLoader;
  unsigned int thumbnailCount = parseSampleCount(config);
  std::vector<Image> thumbnails;
  for (int i = 0; i < thumbnailCount; i++) {
    std::string path = createThumbnailPath(imageBasePath, i+indexOffset,
      imageSuffix, indexOffset, shouldPadZeroes, thumbnailCount);
    std::cout << "Loading image: " << path << std::endl;
    Image image = imageLoader.loadImage(path, ImageLoader::Format::PNG);
    thumbnails.push_back(image);
  }

  return thumbnails;
}

