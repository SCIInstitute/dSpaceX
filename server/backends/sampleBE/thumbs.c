/*
 *  dSpaceX - thumbnail image reader
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <png.h>

#ifdef WIN32
#define strcasecmp  stricmp
#endif


static int
loadPng(const char *Name, int *Width, int *Height, unsigned char **Image)
{
  int           width, height, number_of_passes, i, x, y;
  unsigned char header[8], *pDummy; // 8 is the maximum size that can be checked
  png_structp   png_ptr;
  png_infop     info_ptr;
  png_byte      color_type, bit_depth, *row, *ptr;
  png_bytep     *row_pointers;
  FILE          *fp;
  
  /* open file and test for it being a png */
  fp = fopen(Name, "rb");
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8)) {
    fclose(fp);
    return -5;
  }
  
  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fclose(fp);
    return -6;
  }
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return -7;
  }
  
  if (setjmp(png_jmpbuf(png_ptr))) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return -8;
  }
  
  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  
  png_read_info(png_ptr, info_ptr);
  
  width  = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth  = png_get_bit_depth(png_ptr, info_ptr);
  if ((color_type != PNG_COLOR_TYPE_RGB) &&
      (color_type != PNG_COLOR_TYPE_RGBA)) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return -10;
  }
  
  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);
  
  /* read file */
  if (setjmp(png_jmpbuf(png_ptr))) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return -9;
  }
  
  row_pointers = (png_bytep *) malloc(sizeof(png_bytep)*height);
  if (row_pointers == NULL) return -4;
  for (y = 0; y < height; y++) {
    row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(png_ptr,info_ptr));
    if (row_pointers[y] == NULL) {
      for (i = 0; i < y; i++) free(row_pointers[i]);
      free(row_pointers);
      fclose(fp);
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return -4;
    }
  }
  
  png_read_image(png_ptr, row_pointers);
  fclose(fp);
  
  pDummy = (unsigned char *) malloc(width*height*4*sizeof(unsigned char));
  if (pDummy == NULL) {
    for (y = 0; y < height; y++) free(row_pointers[y]);
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return -4;
  }
  for (i = y = 0; y < height; y++) {
    row = row_pointers[y];
    for (x = 0; x < width; x++, i++) {
      if (color_type == PNG_COLOR_TYPE_RGBA) {
        ptr = &(row[x*4]);
        pDummy[4*i+3] = ptr[3];
      } else {
        ptr = &(row[x*3]);
        pDummy[4*i+3] = 255;
      }
      pDummy[4*i  ] = ptr[0];
      pDummy[4*i+1] = ptr[1];
      pDummy[4*i+2] = ptr[2];
    }
  }

  for (y = 0; y < height; y++) free(row_pointers[y]);
  free(row_pointers);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  
  *Image  = pDummy;
  *Height = height;
  *Width  = width;
  return 0;
}


static int
loadJpg(const char *Name, int *Width, int *Height, unsigned char **Image)
{
  unsigned char a, r, g, b, *pDummy, *pTest;
  int           x, width, height;
  struct        jpeg_decompress_struct cinfo;
  struct        jpeg_error_mgr jerr;
  FILE          *infile;        /* source file */
  JSAMPARRAY    pJpegBuffer;    /* Output row buffer */
  int           row_stride;     /* physical row width in output buffer */

  infile    = fopen(Name, "rb");
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);
  width  = cinfo.output_width;
  height = cinfo.output_height;
  
  pDummy = (unsigned char *) malloc(width*height*4*sizeof(unsigned char));
  pTest  = pDummy;
  if (pDummy == NULL) {
    printf("NO MEM FOR JPEG CONVERT!\n");
    return -4;
  }
  row_stride  = width * cinfo.output_components;
  pJpegBuffer = (*cinfo.mem->alloc_sarray)
  ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
  
  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
    for (x = 0; x < width; x++) {
      a = 255; // alpha value is not supported on jpg
      r = pJpegBuffer[0][cinfo.output_components * x];
      if (cinfo.output_components > 2) {
        g = pJpegBuffer[0][cinfo.output_components * x + 1];
        b = pJpegBuffer[0][cinfo.output_components * x + 2];
      } else {
        g = r;
        b = r;
      }
      *(pDummy++) = r;
      *(pDummy++) = g;
      *(pDummy++) = b;
      *(pDummy++) = a;
    }
  }
  fclose(infile);
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  
  *Image  = pTest;
  *Height = height;
  *Width  = width;
  return 0;
}


int
dsx_getThumbNail(const char *Name, int *Width, int *Height,
                 unsigned char **Image)
{
  int  len, idot, stat;
  FILE *fp;
  
  /* does file exist? */
  fp = fopen(Name, "r");
  if (fp == NULL) return -1;
  fclose(fp);
  
  /* find the file extension */
  len = strlen(Name);
  for (idot = len-1; idot > 0; idot--)
    if (Name[idot] == '.') break;
  if (idot == 0) return -2;
/*@-unrecog@*/
  if ((strcasecmp(&Name[idot],".jpeg") != 0) &&
      (strcasecmp(&Name[idot],".jpg")  != 0) &&
      (strcasecmp(&Name[idot],".png")  != 0)) return -3;
/*@+unrecog@*/

  if (strcasecmp(&Name[idot],".png") == 0) {
    stat = loadPng(Name, Width, Height, Image);
  } else {
    stat = loadJpg(Name, Width, Height, Image);
  }
  
  return stat;
}
