#pragma once

class Image {
 public:
  Image(int width, int height, unsigned char *data);
  unsigned char* getData();
  int getWidth();
  int getHeight();

 private:
  int m_width;
  int m_height;
  unsigned char* m_data;
};
