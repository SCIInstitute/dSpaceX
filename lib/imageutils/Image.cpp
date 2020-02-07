#include "Image.h"

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
