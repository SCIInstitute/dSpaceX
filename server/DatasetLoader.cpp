#include "DatasetLoader.h"
#include "hdprocess/util/csv/loaders.h"
#include "imageutils/ImageLoader.h"
#include "yaml-cpp/yaml.h"

#include <memory>
#include <vector>

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

Dataset* DatasetLoader::loadDataset(const std::string &filePath) {
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

  return builder.build();
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

std::vector<Image*> DatasetLoader::parseThumbnails(const YAML::Node &config,
                                                   const std::string &filePath) {
  if(!config["thumbnails"]) {
    throw std::runtime_error("Dataset config missing 'thumbnails' field.");
  }
  const YAML::Node &thumbnailsNode = config["thumbnails"];

  if(!thumbnailsNode["format"]) {
    throw std::runtime_error("Dataset config missing 'thumbnails.format' field.");
  }
  std::string format = thumbnailsNode["format"].as<std::string>();
  if (format != "png") {
    ("Dataset config specifies unsupported thumbnails format: " + format);
  }

  if(!thumbnailsNode["files"]) {
    throw std::runtime_error("Dataset config missing 'thumbnails.files' field.");
  }
  std::string imagePath = thumbnailsNode["files"].as<std::string>();
  int imageNameLoc = imagePath.find('?');  
  std::string imageDir = imagePath.substr(0,imageNameLoc);
  std::string imageSuffix = imagePath.substr(imageNameLoc +1);

  if(!thumbnailsNode["padZeroes"]) {
    throw std::runtime_error("Dataset config missing 'thumbnails.padZeroes' field.");
  }
  std::string padZeroes = thumbnailsNode["padZeroes"].as<std::string>();
  bool bPadZeroes = (padZeroes == "true");

  // TODO: Load in thumbnail images using ImageLoader
  ImageLoader imgLdr;
  int thumbnailCount = parseSampleCount(config);
  std::vector<Image*> thumbnailImages;

  for (int i = 0; i < thumbnailCount; i++) {
    imagePath = imageDir + std::to_string(i+1) + imageSuffix;
    std::cout << "Loading image: " << imagePath << std::endl;

    Image* image = new Image(imgLdr.loadImage(
            imagePath, ImageLoader::Format::PNG));
    thumbnailImages.push_back(image);
  }

  return thumbnailImages;
}

