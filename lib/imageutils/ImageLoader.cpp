#include "ImageLoader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

// TODO: Add support for JPG images.
Image ImageLoader::loadImage(const std::string filename) {
  int width, height;
  unsigned char* data;
  loadPNG(filename, &width, &height, &data);
  Image image(width, height, data);
  return image;
}

bool ImageLoader::loadPNG(std::string filename, int *width, int *height, png_byte **imageData) {
  int x, y;
  
  png_structp png_ptr;
  png_infop info_ptr;    

  char header[8];    // 8 is the maximum size that can be checked
  unsigned int sig_read = 0;

  // Open file and test for it being a png.
  FILE *fp = fopen(filename.c_str(), "rb");
  if (fp ==  nullptr) {
    std::cout << "Error: File could not be opened for reading" << std::endl;
    return false;
  }

  fread(header, 1, 8, fp);
  if (png_sig_cmp((const unsigned char*)header, 0, 8)) {
    std::cout << "File is not recognized as a PNG file." << std::endl;
    return false;
  } 


  // Create and initialize the png_struct with the desired error handler functions. 
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == nullptr) {
    fclose(fp);    
    return false;
  }
          
  // Allocate/initialize the memory 
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return false;
  }
   
  if (setjmp(png_jmpbuf(png_ptr))) {
    // Free all of the memory associated with the png_ptr and info_ptr.
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return false;
  }    

  // init png reading
  png_init_io(png_ptr, fp);

  // let libpng know you already read the first 8 bytes
  png_set_sig_bytes(png_ptr, 8);

  // read all info up to the image data
  png_read_info(png_ptr, info_ptr);

  *width = png_get_image_width(png_ptr, info_ptr);
  *height = png_get_image_height(png_ptr, info_ptr);
  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  // if(bit_depth == 16)
  //   png_set_strip_16(png_ptr);

  int number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);


  // Set Error Handling to read image.
  if (setjmp(png_jmpbuf(png_ptr))) {
    std::cout << "[read_png_file] Error during read_image" << std::endl;
    fclose(fp);
    return false;
  }    

  // Get row size in bytes.
  unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);

  // glTexImage2d requires rows to be 4-byte aligned?
  // rowbytes += 3 - ((rowbytes-1) % 4);

  // Allocate the image_data as a big block, to be given to opengl
  png_byte *image_data = (png_byte*) malloc(row_bytes * (*height) * sizeof(png_byte)+15);

  png_bytep *row_pointers = (png_bytep*) malloc(*height * sizeof(png_bytep));
  for (int y = 0; y < *height; y++) {    
    row_pointers[*height - 1 - y] = image_data + y*row_bytes;
    // row_pointers[y] = image_data + y*row_bytes;
  }
  

  *imageData = image_data;

  // Read the png into imageData through the row_pointers
  png_read_image(png_ptr, row_pointers);

  /* Clean up after the read,
   * and free any memory allocated 
   */
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  /* Close the file */
  fclose(fp);

  return true;
}

