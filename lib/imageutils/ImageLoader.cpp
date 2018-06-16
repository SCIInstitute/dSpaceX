#include "ImageLoader.h"
#include "png.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <stdexcept>

// TODO: Add support for JPG images.
Image ImageLoader::loadImage(const std::string filename, ImageLoader::Format format) {
  switch(format) {
    case ImageLoader::Format::PNG:
      return loadPNG(filename);
   	default:
      throw std::runtime_error("Unsupported file format");
  }
}

Image ImageLoader::loadPNG(std::string filename) {
  int x, y;
  
  png_structp png_ptr;
  png_infop info_ptr;    

  char header[8];    // 8 is the maximum size that can be checked
  unsigned int sig_read = 0;

  // Open file and test for it being a png.
  FILE *fp = fopen(filename.c_str(), "rb");
  if (fp == nullptr) {
    throw std::runtime_error("Error: File could not be opened for reading.");
  }

  fread(header, 1, 8, fp);
  if (png_sig_cmp((const unsigned char*)header, 0, 8)) {
    throw std::runtime_error("File is not recognized as a PNG file.");
  } 


  // Create and initialize the png_struct with the desired error handler functions. 
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == nullptr) {
    fclose(fp);    
    throw std::runtime_error("Failed to initialize png struct.");
  }
          
  // Allocate/initialize the memory 
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    throw std::runtime_error("Failed to initialize png info struct.");
  }
   
  if (setjmp(png_jmpbuf(png_ptr))) {
    // Free all of the memory associated with the png_ptr and info_ptr.
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    throw std::runtime_error("Failed to set jmpbuffer");
  }    

  // init png reading
  png_init_io(png_ptr, fp);

  // let libpng know you already read the first 8 bytes
  png_set_sig_bytes(png_ptr, 8);

  // read all info up to the image data
  png_read_info(png_ptr, info_ptr);

  int width = png_get_image_width(png_ptr, info_ptr);
  int height = png_get_image_height(png_ptr, info_ptr);
  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  // if(bit_depth == 16)
  //   png_set_strip_16(png_ptr);

  int number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);


  // Set Error Handling to read image.
  if (setjmp(png_jmpbuf(png_ptr))) {
    fclose(fp);
    throw std::runtime_error("[read_png_file] Error during read_image");
  }    

  // Get row size in bytes.
  unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);

  // glTexImage2d requires rows to be 4-byte aligned?
  // rowbytes += 3 - ((rowbytes-1) % 4);

  // Allocate the image_data as a big block, to be given to opengl
  png_byte *image_data = (png_byte*) malloc(row_bytes * height * sizeof(png_byte)+15);

  png_bytep *row_pointers = (png_bytep*) malloc(height * sizeof(png_bytep));
  for (int y = 0; y < height; y++) {    
    row_pointers[height - 1 - y] = image_data + y*row_bytes;
    // row_pointers[y] = image_data + y*row_bytes;
  }

  // Read the png into imageData through the row_pointers
  png_read_image(png_ptr, row_pointers);

  /* Clean up after the read,
   * and free any memory allocated 
   */
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  /* Close the file */
  fclose(fp);

  return Image(width, height, image_data);
}

