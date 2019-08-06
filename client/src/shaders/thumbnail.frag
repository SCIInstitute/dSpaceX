precision mediump float;
uniform float nodeOutline;
uniform float nodeSmoothness;
uniform sampler2D imageTex;
uniform float thumbnailWidth;
uniform float thumbnailHeight;
varying vec2 vertexUV;
varying vec3 geomColor;
varying float index;


float modi(float a,float b) {
  float m = a - floor((a+0.5)/b)*b;
  return floor(m+0.5);
}

void main(void) {
  vec2 uv = vertexUV.xy;
  float MAX_TEXTURE_SIZE = 2048.0;
  float THUMBNAIL_WIDTH = thumbnailWidth;
  float THUMBNAIL_HEIGHT = thumbnailHeight;
  float thumbnailsPerTextureRow = floor(MAX_TEXTURE_SIZE / THUMBNAIL_WIDTH);

  float aspect_ratio = THUMBNAIL_HEIGHT / THUMBNAIL_WIDTH;
  float inv_aspect = 1.0 / aspect_ratio;
  float uscale = THUMBNAIL_WIDTH / MAX_TEXTURE_SIZE;
  float vscale = THUMBNAIL_HEIGHT / MAX_TEXTURE_SIZE;
  float atlasOffsetX = modi(index, thumbnailsPerTextureRow);
  float atlasOffsetY = floor(index / thumbnailsPerTextureRow);

  // Account for Thumbnail Aspect Ratio - Scale to Fit
  float aspect_u = uv.x;
  float aspect_v = (uv.y - 0.5*(1.0 - aspect_ratio)) / aspect_ratio;
  
  // Account for Lookup into Texture Atlas
  float atlas_u = (float(atlasOffsetX) + aspect_u) * uscale;
  float atlas_v = (float(atlasOffsetY) + aspect_v) * vscale;
  vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 transparent = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 lineColor = vec4(mix(black.xyz, geomColor, 0.4), 1.0);
  vec4 frag_color = vec4(texture2D(imageTex, vec2(atlas_u,atlas_v)).rgb, 1); 

  // Add QoI outline color\n                                            
  if (aspect_u <= nodeOutline || aspect_u >= (1.0 - nodeOutline) ||        
      aspect_v <= inv_aspect*nodeOutline ||                             
      aspect_v >= (1.0 - inv_aspect*nodeOutline)) {                       
    frag_color = vec4(geomColor, 1.0);                                 
  }                                                                     
                                                                        
  // Make area outside of thumbnail transparent.\n                      
  if (aspect_u < 0.0 || aspect_u > 1.0 || aspect_v < 0.0 || aspect_v > 1.0) {   
    frag_color = transparent;                                           
  }      
  gl_FragColor = frag_color;
}