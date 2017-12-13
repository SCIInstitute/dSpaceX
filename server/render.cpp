#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>

#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"

#include "wsserver.h"


#define NCOLOR 256
#define PI     3.1415926535897931159979635


/* blue-white-red spectrum */
static float color_map[NCOLOR*3] =
{ 0.0000, 0.0000, 1.0000,    0.0078, 0.0078, 1.0000,
  0.0156, 0.0156, 1.0000,    0.0234, 0.0234, 1.0000,
  0.0312, 0.0312, 1.0000,    0.0391, 0.0391, 1.0000,
  0.0469, 0.0469, 1.0000,    0.0547, 0.0547, 1.0000,
  0.0625, 0.0625, 1.0000,    0.0703, 0.0703, 1.0000,
  0.0781, 0.0781, 1.0000,    0.0859, 0.0859, 1.0000,
  0.0938, 0.0938, 1.0000,    0.1016, 0.1016, 1.0000,
  0.1094, 0.1094, 1.0000,    0.1172, 0.1172, 1.0000,
  0.1250, 0.1250, 1.0000,    0.1328, 0.1328, 1.0000,
  0.1406, 0.1406, 1.0000,    0.1484, 0.1484, 1.0000,
  0.1562, 0.1562, 1.0000,    0.1641, 0.1641, 1.0000,
  0.1719, 0.1719, 1.0000,    0.1797, 0.1797, 1.0000,
  0.1875, 0.1875, 1.0000,    0.1953, 0.1953, 1.0000,
  0.2031, 0.2031, 1.0000,    0.2109, 0.2109, 1.0000,
  0.2188, 0.2188, 1.0000,    0.2266, 0.2266, 1.0000,
  0.2344, 0.2344, 1.0000,    0.2422, 0.2422, 1.0000,
  0.2500, 0.2500, 1.0000,    0.2578, 0.2578, 1.0000,
  0.2656, 0.2656, 1.0000,    0.2734, 0.2734, 1.0000,
  0.2812, 0.2812, 1.0000,    0.2891, 0.2891, 1.0000,
  0.2969, 0.2969, 1.0000,    0.3047, 0.3047, 1.0000,
  0.3125, 0.3125, 1.0000,    0.3203, 0.3203, 1.0000,
  0.3281, 0.3281, 1.0000,    0.3359, 0.3359, 1.0000,
  0.3438, 0.3438, 1.0000,    0.3516, 0.3516, 1.0000,
  0.3594, 0.3594, 1.0000,    0.3672, 0.3672, 1.0000,
  0.3750, 0.3750, 1.0000,    0.3828, 0.3828, 1.0000,
  0.3906, 0.3906, 1.0000,    0.3984, 0.3984, 1.0000,
  0.4062, 0.4062, 1.0000,    0.4141, 0.4141, 1.0000,
  0.4219, 0.4219, 1.0000,    0.4297, 0.4297, 1.0000,
  0.4375, 0.4375, 1.0000,    0.4453, 0.4453, 1.0000,
  0.4531, 0.4531, 1.0000,    0.4609, 0.4609, 1.0000,
  0.4688, 0.4688, 1.0000,    0.4766, 0.4766, 1.0000,
  0.4844, 0.4844, 1.0000,    0.4922, 0.4922, 1.0000,
  0.5000, 0.5000, 1.0000,    0.5078, 0.5078, 1.0000,
  0.5156, 0.5156, 1.0000,    0.5234, 0.5234, 1.0000,
  0.5312, 0.5312, 1.0000,    0.5391, 0.5391, 1.0000,
  0.5469, 0.5469, 1.0000,    0.5547, 0.5547, 1.0000,
  0.5625, 0.5625, 1.0000,    0.5703, 0.5703, 1.0000,
  0.5781, 0.5781, 1.0000,    0.5859, 0.5859, 1.0000,
  0.5938, 0.5938, 1.0000,    0.6016, 0.6016, 1.0000,
  0.6094, 0.6094, 1.0000,    0.6172, 0.6172, 1.0000,
  0.6250, 0.6250, 1.0000,    0.6328, 0.6328, 1.0000,
  0.6406, 0.6406, 1.0000,    0.6484, 0.6484, 1.0000,
  0.6562, 0.6562, 1.0000,    0.6641, 0.6641, 1.0000,
  0.6719, 0.6719, 1.0000,    0.6797, 0.6797, 1.0000,
  0.6875, 0.6875, 1.0000,    0.6953, 0.6953, 1.0000,
  0.7031, 0.7031, 1.0000,    0.7109, 0.7109, 1.0000,
  0.7188, 0.7188, 1.0000,    0.7266, 0.7266, 1.0000,
  0.7344, 0.7344, 1.0000,    0.7422, 0.7422, 1.0000,
  0.7500, 0.7500, 1.0000,    0.7578, 0.7578, 1.0000,
  0.7656, 0.7656, 1.0000,    0.7734, 0.7734, 1.0000,
  0.7812, 0.7812, 1.0000,    0.7891, 0.7891, 1.0000,
  0.7969, 0.7969, 1.0000,    0.8047, 0.8047, 1.0000,
  0.8125, 0.8125, 1.0000,    0.8203, 0.8203, 1.0000,
  0.8281, 0.8281, 1.0000,    0.8359, 0.8359, 1.0000,
  0.8438, 0.8438, 1.0000,    0.8516, 0.8516, 1.0000,
  0.8594, 0.8594, 1.0000,    0.8672, 0.8672, 1.0000,
  0.8750, 0.8750, 1.0000,    0.8828, 0.8828, 1.0000,
  0.8906, 0.8906, 1.0000,    0.8984, 0.8984, 1.0000,
  0.9062, 0.9062, 1.0000,    0.9141, 0.9141, 1.0000,
  0.9219, 0.9219, 1.0000,    0.9297, 0.9297, 1.0000,
  0.9375, 0.9375, 1.0000,    0.9453, 0.9453, 1.0000,
  0.9531, 0.9531, 1.0000,    0.9609, 0.9609, 1.0000,
  0.9688, 0.9688, 1.0000,    0.9766, 0.9766, 1.0000,
  0.9844, 0.9844, 1.0000,    0.9922, 0.9922, 1.0000,
  1.0000, 1.0000, 1.0000,    1.0000, 0.9922, 0.9922,
  1.0000, 0.9844, 0.9844,    1.0000, 0.9766, 0.9766,
  1.0000, 0.9688, 0.9688,    1.0000, 0.9609, 0.9609,
  1.0000, 0.9531, 0.9531,    1.0000, 0.9453, 0.9453,
  1.0000, 0.9375, 0.9375,    1.0000, 0.9297, 0.9297,
  1.0000, 0.9219, 0.9219,    1.0000, 0.9141, 0.9141,
  1.0000, 0.9062, 0.9062,    1.0000, 0.8984, 0.8984,
  1.0000, 0.8906, 0.8906,    1.0000, 0.8828, 0.8828,
  1.0000, 0.8750, 0.8750,    1.0000, 0.8672, 0.8672,
  1.0000, 0.8594, 0.8594,    1.0000, 0.8516, 0.8516,
  1.0000, 0.8438, 0.8438,    1.0000, 0.8359, 0.8359,
  1.0000, 0.8281, 0.8281,    1.0000, 0.8203, 0.8203,
  1.0000, 0.8125, 0.8125,    1.0000, 0.8047, 0.8047,
  1.0000, 0.7969, 0.7969,    1.0000, 0.7891, 0.7891,
  1.0000, 0.7812, 0.7812,    1.0000, 0.7734, 0.7734,
  1.0000, 0.7656, 0.7656,    1.0000, 0.7578, 0.7578,
  1.0000, 0.7500, 0.7500,    1.0000, 0.7422, 0.7422,
  1.0000, 0.7344, 0.7344,    1.0000, 0.7266, 0.7266,
  1.0000, 0.7188, 0.7188,    1.0000, 0.7109, 0.7109,
  1.0000, 0.7031, 0.7031,    1.0000, 0.6953, 0.6953,
  1.0000, 0.6875, 0.6875,    1.0000, 0.6797, 0.6797,
  1.0000, 0.6719, 0.6719,    1.0000, 0.6641, 0.6641,
  1.0000, 0.6562, 0.6562,    1.0000, 0.6484, 0.6484,
  1.0000, 0.6406, 0.6406,    1.0000, 0.6328, 0.6328,
  1.0000, 0.6250, 0.6250,    1.0000, 0.6172, 0.6172,
  1.0000, 0.6094, 0.6094,    1.0000, 0.6016, 0.6016,
  1.0000, 0.5938, 0.5938,    1.0000, 0.5859, 0.5859,
  1.0000, 0.5781, 0.5781,    1.0000, 0.5703, 0.5703,
  1.0000, 0.5625, 0.5625,    1.0000, 0.5547, 0.5547,
  1.0000, 0.5469, 0.5469,    1.0000, 0.5391, 0.5391,
  1.0000, 0.5312, 0.5312,    1.0000, 0.5234, 0.5234,
  1.0000, 0.5156, 0.5156,    1.0000, 0.5078, 0.5078,
  1.0000, 0.5000, 0.5000,    1.0000, 0.4922, 0.4922,
  1.0000, 0.4844, 0.4844,    1.0000, 0.4766, 0.4766,
  1.0000, 0.4688, 0.4688,    1.0000, 0.4609, 0.4609,
  1.0000, 0.4531, 0.4531,    1.0000, 0.4453, 0.4453,
  1.0000, 0.4375, 0.4375,    1.0000, 0.4297, 0.4297,
  1.0000, 0.4219, 0.4219,    1.0000, 0.4141, 0.4141,
  1.0000, 0.4062, 0.4062,    1.0000, 0.3984, 0.3984,
  1.0000, 0.3906, 0.3906,    1.0000, 0.3828, 0.3828,
  1.0000, 0.3750, 0.3750,    1.0000, 0.3672, 0.3672,
  1.0000, 0.3594, 0.3594,    1.0000, 0.3516, 0.3516,
  1.0000, 0.3438, 0.3438,    1.0000, 0.3359, 0.3359,
  1.0000, 0.3281, 0.3281,    1.0000, 0.3203, 0.3203,
  1.0000, 0.3125, 0.3125,    1.0000, 0.3047, 0.3047,
  1.0000, 0.2969, 0.2969,    1.0000, 0.2891, 0.2891,
  1.0000, 0.2812, 0.2812,    1.0000, 0.2734, 0.2734,
  1.0000, 0.2656, 0.2656,    1.0000, 0.2578, 0.2578,
  1.0000, 0.2500, 0.2500,    1.0000, 0.2422, 0.2422,
  1.0000, 0.2344, 0.2344,    1.0000, 0.2266, 0.2266,
  1.0000, 0.2188, 0.2188,    1.0000, 0.2109, 0.2109,
  1.0000, 0.2031, 0.2031,    1.0000, 0.1953, 0.1953,
  1.0000, 0.1875, 0.1875,    1.0000, 0.1797, 0.1797,
  1.0000, 0.1719, 0.1719,    1.0000, 0.1641, 0.1641,
  1.0000, 0.1562, 0.1562,    1.0000, 0.1484, 0.1484,
  1.0000, 0.1406, 0.1406,    1.0000, 0.1328, 0.1328,
  1.0000, 0.1250, 0.1250,    1.0000, 0.1172, 0.1172,
  1.0000, 0.1094, 0.1094,    1.0000, 0.1016, 0.1016,
  1.0000, 0.0938, 0.0938,    1.0000, 0.0859, 0.0859,
  1.0000, 0.0781, 0.0781,    1.0000, 0.0703, 0.0703,
  1.0000, 0.0625, 0.0625,    1.0000, 0.0547, 0.0547,
  1.0000, 0.0469, 0.0469,    1.0000, 0.0391, 0.0391,
  1.0000, 0.0312, 0.0312,    1.0000, 0.0234, 0.0234,
  1.0000, 0.0156, 0.0156,    1.0000, 0.0078, 0.0078 };


static void
spec_col(float *lims, float scalar, float color[])
{
  int   indx;
  float frac;
  
  if (lims[0] == lims[1]) {
    color[0] = 0.0;
    color[1] = 1.0;
    color[2] = 0.0;
  } else if (scalar <= lims[0]) {
    color[0] = color_map[0];
    color[1] = color_map[1];
    color[2] = color_map[2];
  } else if (scalar >= lims[1]) {
    color[0] = color_map[3*255  ];
    color[1] = color_map[3*255+1];
    color[2] = color_map[3*255+2];
  } else {
    frac  = 255.0*(scalar - lims[0])/(lims[1] - lims[0]);
    if (frac < 0  ) frac = 0;
    if (frac > 255) frac = 255;
    indx  = frac;
    frac -= indx;
    if (indx == 255) {
      indx--;
      frac += 1.0;
    }
    color[0] = frac*color_map[3*(indx+1)  ] + (1.0-frac)*color_map[3*indx  ];
    color[1] = frac*color_map[3*(indx+1)+1] + (1.0-frac)*color_map[3*indx+1];
    color[2] = frac*color_map[3*(indx-1)+2] + (1.0-frac)*color_map[3*indx+2];
  }
}


void
dsx_drawKey(wvContext *cntxt, float *lims, /*@null@*/ char *name)
{
  int stat;
  
  if (name == NULL) {
    stat = wv_setKey(cntxt,      0, color_map, lims[0], lims[1], name);
  } else {
    stat = wv_setKey(cntxt, NCOLOR, color_map, lims[0], lims[1], name);
  }
  if (stat < 0) printf(" wv_setKey = %d!\n", stat);
}


/* place-holder for scatter-plot rendering */
void
dsx_draw2D(wvContext *cntxt, FortranLinalg::DenseMatrix<Precision> layout, 
  std::vector<unsigned int> edgeIndices, float *lims, int nCrystal, int flag)
{
  int    i, stat, segs[8];
  float  xy[12], focus[4], colrs[3];
  char   gpname[33];
  wvData items[3];
  
  if (flag != 0) return;
  
  focus[0] = focus[1] = focus[2] = 0.0;
  focus[3] = 1.0;
  
  xy[ 0]  = xy[ 1] = -0.9;
  xy[ 3]  =  0.9;
  xy[ 4]  = -0.9;
  xy[ 6]  = xy[ 7] =  0.9;
  xy[ 9]  = -0.9;
  xy[10]  =  0.9;
  xy[ 2]  = xy[5] = xy[8] = xy[11] = 0.0;
  segs[0] = 0;
  segs[1] = 1;
  segs[2] = 1;
  segs[3] = 2;
  segs[4] = 2;
  segs[5] = 3;
  segs[6] = 3;
  segs[7] = 0;
  
  sprintf(gpname, "Scatter Lines");
  stat = wv_setData(WV_REAL32, 4, (void *) xy,  WV_VERTICES, &items[0]);
  if (stat < 0) {
    printf(" wv_setData = %d for %s/item 0!\n", stat, gpname);
    return;
  }
  wv_adjustVerts(&items[0], focus);
  colrs[0]  = 0.5;
  colrs[1]  = 0.5;
  colrs[2]  = 0.5;
  stat = wv_setData(WV_REAL32, 1, (void *) colrs,  WV_COLORS, &items[1]);
  if (stat < 0) {
    printf(" wv_setData = %d for %s/item 1!\n", stat, gpname);
    return;
  }
  stat = wv_setData(WV_INT32, 2*4, (void *) segs, WV_INDICES, &items[2]);
  if (stat < 0) {
    printf(" wv_setData = %d for %s/item 2!\n", stat, gpname);
    return;
  }
  stat = wv_addGPrim(cntxt, gpname, WV_LINE2D, WV_ON, 3, items);
  if (stat < 0)
    printf(" wv_addGPrim = %d for %s!\n", stat, gpname);
  
  sprintf(gpname, "Scatter Dots");
  stat = wv_setData(WV_REAL32, 4, (void *) xy,  WV_VERTICES, &items[0]);
  if (stat < 0) {
    printf(" wv_setData = %d for %s/item 0!\n", stat, gpname);
    return;
  }
  wv_adjustVerts(&items[0], focus);
  colrs[0]  = 0.0;
  colrs[1]  = 0.0;
  colrs[2]  = 1.0;
  stat = wv_setData(WV_REAL32, 1, (void *) colrs,  WV_COLORS, &items[1]);
  if (stat < 0) {
    printf(" wv_setData = %d for %s/item 1!\n", stat, gpname);
    return;
  }
  stat = wv_addGPrim(cntxt, gpname, WV_POINT2D, WV_ON, 2, items);
  if (stat < 0) {
    printf(" wv_addGPrim = %d for %s!\n", stat, gpname);
  } else {
    cntxt->gPrims[stat].pSize = 5.0;
  }
  
}


/* place-holder for Morse-Smale Complex rendering */
void
dsx_draw3D(wvContext *cntxt, float *lims, int nCrystal, int flag)
{
  int          i, j, k, k1, m, stat, index, segs[2*24], tris[6*18*24];
  double       ang, rad;
  float        xz[25*2], width[25], xyzs[25*3], colrs[3*25], focus[4];
  float        tube[3*18*25];
  static float scalar[25];
  char         gpname[33];
  wvData       items[3];
  
  focus[0] = focus[1] = focus[2] = 0.0;
  focus[3] = 2.0;
  
  /* fill up a single branch */
  if (flag == 0) {
    for (i = 0; i < 25; i++) {
      xz[2*i  ] =       sin(i*PI/50.0);
      xz[2*i+1] = 1.0 - cos(i*PI/50.0);
      scalar[i] = lims[0] + i*(lims[1]-lims[0])/24.0;
      width[i]  = 0.05 + 0.2*sin(i*PI/24.0);
    }
    for (i = 0; i < 24; i++) {
      segs[2*i  ] = i;
      segs[2*i+1] = i+1;
    }
    wv_removeAll(cntxt);
  }
  
  /* draw our crystals */
  for (j = 0; j < nCrystal; j++) {
    ang  = 2.0*PI*j;
    ang /= nCrystal;
    
    /* plot core as a line */
    sprintf(gpname, "Crystal %d Core 1", j+1);
    
    if (flag == 0) {
      
      /* plot the entire suite */
      for (i = 0; i < 25; i++) {
        xyzs[3*i  ] = xz[2*i+1]*cos(ang);
        xyzs[3*i+1] = xz[2*i+1]*sin(ang);
        xyzs[3*i+2] = xz[2*i  ];
        spec_col(lims, scalar[i], &colrs[3*i]);
      }
      stat = wv_setData(WV_REAL32, 25, (void *) xyzs,  WV_VERTICES, &items[0]);
      if (stat < 0) {
        printf(" wv_setData = %d for %s/item 0!\n", stat, gpname);
        continue;
      }
      wv_adjustVerts(&items[0], focus);
      stat = wv_setData(WV_INT32, 2*24, (void *) segs, WV_INDICES, &items[1]);
      if (stat < 0) {
        printf(" wv_setData = %d for %s/item 1!\n", stat, gpname);
        continue;
      }
      stat = wv_setData(WV_REAL32, 25, (void *) colrs,  WV_COLORS, &items[2]);
      if (stat < 0) {
        printf(" wv_setData = %d for %s/item 2!\n", stat, gpname);
        continue;
      }
      stat = wv_addGPrim(cntxt, gpname, WV_LINE, WV_ON|WV_SHADING, 3, items);
      if (stat < 0)
        printf(" wv_addGPrim = %d for %s!\n", stat, gpname);
      
      /* plot extent transparent */
      sprintf(gpname, "Crystal %d Tube 1", j+1);
      for (m = i = 0; i < 25; i++)
        for (k = 0; k < 18; k++, m++) {
          rad          = 2.0*PI*k;
          rad         /= 18.0;
          tube[3*m  ]  = xyzs[3*i  ] + width[i]*sin(i*PI/50.0)*cos(rad);
          tube[3*m+1]  = xyzs[3*i+1] + width[i]*cos(i*PI/50.0)*sin(rad);
          tube[3*m+2]  = xyzs[3*i+2];
        }
      for (m = i = 0; i < 24; i++) {
        for (k = 0; k < 18; k++, m++) {
                       k1 = k + 1;
          if (k == 17) k1 = 0;
          tris[6*m  ] =  i   *18 + k;
          tris[6*m+1] =  i   *18 + k1;
          tris[6*m+2] = (i+1)*18 + k;
          tris[6*m+3] =  i   *18 + k1;
          tris[6*m+4] = (i+1)*18 + k1;
          tris[6*m+5] = (i+1)*18 + k;
        }
      }
      stat = wv_setData(WV_REAL32, 18*25, (void *) tube,  WV_VERTICES, &items[0]);
      if (stat < 0) {
        printf(" wv_setData = %d for %s/item 0!\n", stat, gpname);
        continue;
      }
      wv_adjustVerts(&items[0], focus);
      stat = wv_setData(WV_INT32, 6*18*24, (void *) tris, WV_INDICES, &items[1]);
      if (stat < 0) {
        printf(" wv_setData = %d for %s/item 1!\n", stat, gpname);
        continue;
      }
      colrs[0]  = 0.5;
      colrs[1]  = 0.5;
      colrs[2]  = 0.5;
      stat = wv_setData(WV_REAL32, 1, (void *) colrs,  WV_COLORS, &items[2]);
      if (stat < 0) {
        printf(" wv_setData = %d for %s/item 2!\n", stat, gpname);
        continue;
      }
      stat = wv_addGPrim(cntxt, gpname, WV_TRIANGLE, WV_ON|WV_TRANSPARENT,
                         3, items);
      if (stat < 0)
        printf(" wv_addGPrim = %d for %s!\n", stat, gpname);
      
    } else {
      
      /* only the colors have changed */
      index = wv_indexGPrim(cntxt, gpname);
      for (i = 0; i < 25; i++) spec_col(lims, scalar[i], &colrs[3*i]);
      stat = wv_setData(WV_REAL32, 25, (void *) colrs,  WV_COLORS, &items[0]);
      if (stat < 0) {
        printf(" wv_setData = %d for %s/item 2!\n", stat, gpname);
        continue;
      }
      stat = wv_modGPrim(cntxt, index, 1, items);
      if (stat < 0)
        printf(" wv_modGPrim = %d for %s (%d)!\n", stat, gpname, index);
    }
    
  }
}
