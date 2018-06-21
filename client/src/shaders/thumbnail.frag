precision mediump float;
uniform float nodeOutline;
uniform float nodeSmoothness;
uniform sampler2D imageTex;
varying vec2 vertexUV;
varying vec3 geomColor;

void main(void) {
  vec2 uv = vertexUV.xy;
  int MAX_TEXTURE_SIZE = 2048;
  int THUMBNAIL_WIDTH = 80;
  int THUMBNAIL_HEIGHT = 40;
  int thumbnailsPerTextureRow = 25;

  // TODO: Provide real index.
  int geom_thumbnail = 0;

  float aspect_ratio = float(THUMBNAIL_HEIGHT) / float(THUMBNAIL_WIDTH);
  float inv_aspect = 1.0 / aspect_ratio;
  float uscale = float(THUMBNAIL_WIDTH) / float(MAX_TEXTURE_SIZE);
  float vscale = float(THUMBNAIL_HEIGHT) / float(MAX_TEXTURE_SIZE);
  int atlasOffsetX = 0; // int(geom_thumbnail) % thumbnailsPerTextureRow;
  int atlasOffsetY = 0; // int(geom_thumbnail) / thumbnailsPerTextureRow;

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