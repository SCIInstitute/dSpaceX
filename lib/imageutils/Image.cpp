#include "Image.h"
#include "lodepng.h"

Image::Image(int width, int height, unsigned char *data, 
  std::vector<char> rawData, const std::string &format) : m_width(width), 
  m_height(height), m_data(data), m_rawData(rawData), m_format(format) {
  // intentionally left empty
}

unsigned char* Image::getData() {
  return m_data;
}

std::vector<char>& Image::getRawData() {
  return m_rawData;
}

const unsigned char* Image::getConstData() const {
  return m_data;
}

const std::vector<char>& Image::getConstRawData() const {
  return m_rawData;
}

int Image::getWidth() const {
  return m_width;
}

int Image::getHeight() const {
  return m_height;
}

std::string Image::getFormat() const {
  return m_format;
}

// converts w*h x 1 matrix of doubles to w x h 2d image of unsigned char, throwing an exception if dims don't match
Image Image::convertToImage(const Eigen::MatrixXf &I, const unsigned w, const unsigned h)
{
  //<ctc> note: this is about the same as in ShapeOdds::evaluateModel, accidental rewrite? consolidate.
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

