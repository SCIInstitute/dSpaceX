#pragma once

#include <string>
#include <vector>
#include <Eigen/Core>

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
  
  static Image convertToImage(const Eigen::MatrixXd &I, const unsigned w, const unsigned h);

 private:
  int m_width;
  int m_height;
  unsigned char* m_data;
  std::vector<char> m_rawData;
  std::string m_format;
};
