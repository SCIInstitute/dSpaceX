#pragma once

#include <string>
#include <vector>

class Image {
 public:
  Image(int width, int height, unsigned char *data, std::vector<char> rawData, 
    const std::string &format);

  unsigned char* getData();
  std::vector<char>& getRawData();

  const unsigned char* getConstData() const;
  const std::vector<char>& getConstRawData() const;

  int getWidth() const;
  int getHeight() const;
  std::string getFormat() const;
  
 private:
  int m_width;
  int m_height;
  unsigned char* m_data;
  std::vector<char> m_rawData;
  std::string m_format;
};
