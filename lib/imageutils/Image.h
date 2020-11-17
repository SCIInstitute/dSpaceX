#pragma once

#include <string>
#include <vector>
#include <Eigen/Core>
#include <lodepng.h>

namespace dspacex {

class Image {
  enum Format { GREY, RGB, RGBA, UNKNOWN };
  
 public:
  /// Creates image from w x h matrix of floats, throwing an exception if dims don't match
  Image(const Eigen::MatrixXf &I, unsigned width, unsigned height, unsigned channels = 1, bool rotate = false);

  /// load an image from a png file (throw exception on failure)
  Image(const std::string& filename, bool decompress = false);

  Image(const unsigned char data[], unsigned w, unsigned h, unsigned c);
  Image(std::vector<unsigned char> &&data, unsigned w, unsigned h, unsigned c);
  Image(const std::string &data, unsigned w, unsigned h, unsigned c);

  /// write image (throw exception on failure)
  void write(const std::string& filename) const;

  const std::vector<unsigned char>& getData() const;     // returns uncompressed image data
  const std::vector<unsigned char>& getPNGData() const;  // returns png-encoded (i.e., compressed) data

  unsigned getWidth() const { return m_width; }
  unsigned getHeight() const { return m_height; }
  int numChannels() const;
  Format getFormat() const;
  
  unsigned char getPixel(unsigned i) const;

 private:
  LodePNGColorType m_format;
  lodepng::State m_state;
  bool m_decompressed;
  unsigned m_width, m_height;
  std::vector<unsigned char> m_data;    // uncompressed image or float [0,1] data from EigenImage * 255.0
  std::vector<unsigned char> m_pngData; // compressed png-encoded data
};

} // dspacex
