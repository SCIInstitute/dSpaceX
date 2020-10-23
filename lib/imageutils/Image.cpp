#include "Image.h"
#include "lodepng.h"

namespace dspacex {

Image::Image(const Eigen::MatrixXf &I, unsigned w, unsigned h, unsigned c, bool rotate) :
  m_width(w), m_height(h), m_format(c == 1 ? LCT_GREY : c == 3 ? LCT_RGB : LCT_RGBA),
  m_data(w * h * c), m_decompressed(true)
{
  // throw an exception if dims/channels don't match
  if (I.size() != w * h * c)
  { 
    throw std::runtime_error("Warning: w * h * c (" + std::to_string(w) + " * " + std::to_string(h) + " * " + std::to_string(c) + ") != input matrix size (" + std::to_string(I.size()) + ")");
  }

  if (c != 1 && c != 3 && c != 4) {
    throw std::runtime_error("Fixme: currently only support 1-, 3-, or 4-channel images");
  }

  /* convert w x h x c matrix of floats to w x h 2d image of unsigned char (note: tricky use of Eigen::Map) */
  using EigenImage = Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

  // 1. create a map to an EigenImage matrix using the already-allocated m_data vector
  unsigned cols = w * c;
  unsigned rows = h;
  Eigen::Map<EigenImage> tmp(m_data.data(), rows, cols);

  // 2. using the Eigen assignment operator from CwiseUnaryOp to (mapped) Matrix, copy the casted results into m_data
  if (rotate) {
    Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> Itmp(const_cast<float*>(I.data()), cols, rows);
    EigenImage coltmp = (Itmp.array() * 255.0).cast<unsigned char>();
 
    // manually rotate image by copying each column to its row
    for (int i = 0; i < m_height; i++)
      tmp.row(i) = coltmp.col(i);
  }
  else {
    Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> Itmp(const_cast<float*>(I.data()), rows, cols);
    tmp = (Itmp.array() * 255.0).cast<unsigned char>();
  }

  // 3. use "placement new" syntax to set tmp's data ptr to null so it doesn't try to deallocate it when it goes out of scope
  new (&tmp) Eigen::Map<EigenImage>(nullptr, -1, -1);
}

// initialize by copying from a char array of the raw image
Image::Image(const unsigned char data[], unsigned w, unsigned h, unsigned c) :
  m_width(w), m_height(h), m_format(c == 1 ? LCT_GREY : c == 3 ? LCT_RGB : LCT_RGBA),
  m_data(data, data + w * h * c), m_decompressed(true)
{
  if (c != 1 && c != 3 && c != 4) {
    throw std::runtime_error("Fixme: currently only support 1-, 3-, or 4-channel images");
  }
}

// initialize by moving an input vector of the raw image
Image::Image(std::vector<unsigned char> &&data, unsigned w, unsigned h, unsigned c) :
  m_width(w), m_height(h), m_format(c == 1 ? LCT_GREY : c == 3 ? LCT_RGB : LCT_RGBA),
  m_data(data), m_decompressed(true)
{
  if (c != 1 && c != 3 && c != 4) {
    throw std::runtime_error("Fixme: currently only support 1-, 3-, or 4-channel images");
  }
  if (data.size() != w * h * c) {
    throw std::runtime_error("tried to initialize Image from an array of different size than expected");
  }
}

// initialize by copying from string of the raw image
Image::Image(const std::string &data, unsigned w, unsigned h, unsigned c) :
  m_width(w), m_height(h), m_format(c == 1 ? LCT_GREY : c == 3 ? LCT_RGB : LCT_RGBA),
  m_data(data.begin(), data.end()), m_decompressed(true)
{
  if (c != 1 && c != 3 && c != 4) {
    throw std::runtime_error("Fixme: currently only support 1-, 3-, or 4-channel images");
  }
  if (data.length() != w * h * c) {
    throw std::runtime_error("tried to initialize Image from an array of different size than expected");
  }
}
Image::Image(const std::string& filename, bool decompress) : m_decompressed(decompress) {
  // identify resolution and format
  lodepng::State state;
  unsigned error;
  if (lodepng::load_file(m_pngData, filename))
    throw std::runtime_error("error loading png");
  if (lodepng_inspect(&m_width, &m_height, &state, m_pngData.data(), m_pngData.size()))
    if (error) throw std::runtime_error("error loading png");

  // either load it as a single channel raw buffer or a 3-channel buffer
  if (state.info_png.color.colortype == LCT_GREY) {
    state.info_raw.colortype = LCT_GREY;
    m_format = LCT_GREY;
  }
  else if (state.info_png.color.colortype == LCT_RGBA) {
    state.info_raw.colortype = LCT_RGBA;
    m_format = LCT_RGBA;
  }
  else {
    state.info_raw.colortype = LCT_RGB;
    m_format = LCT_RGB;
  }
  
  // decompress the png data to this Image's raw buffer
  if (decompress)
    lodepng::decode(m_data, m_width, m_height, state, m_pngData);
}

/// write image
void Image::write(const std::string& outpath) const {
  if (lodepng::save_file(getPNGData(), outpath))
    throw std::runtime_error("error writing png to " + outpath);
}

const std::vector<unsigned char> Image::getData() const {
  if (!m_decompressed)
    throw std::runtime_error("must decompress png on load");
  return m_data;
}

const std::vector<unsigned char> Image::getPNGData() const {
  if (m_pngData.empty()) {
    unsigned error = lodepng::encode(const_cast<std::vector<unsigned char>&>(m_pngData), m_data, m_width, m_height, m_format, 8);
    if (error) {
      throw std::runtime_error("encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));
    } 
  }

  return m_pngData;
}

int Image::numChannels() const {
  return m_format == LCT_GREY ? 1 : m_format == LCT_RGB ? 3 : 4;
}

Image::Format Image::getFormat() const {
  return m_format == LCT_GREY ? GREY : m_format == LCT_RGB ? RGB : RGBA;
}

unsigned char Image::getPixel(unsigned i) const {
  return getData()[i];
}

} // dspacex
