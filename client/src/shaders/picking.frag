precision mediump float;
uniform float nodeOutline;
uniform float nodeSmoothness;
uniform sampler2D imageTex;
varying vec2 vertexUV;
varying vec3 geomColor;
varying float index;


float modi(float a,float b) {
  float m = a - floor((a+0.5)/b)*b;
  return floor(m+0.5);
}

void main(void) {
  vec2 uv = vertexUV.xy;
  vec2 center = vec2(0.5);
  float radius = 0.425;
  float thickness = nodeOutline;
  float blur = nodeSmoothness;
  float t = distance(uv, center) - radius;
  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
  if (t < (thickness + blur)) {
    int baseIndex = int(floor(index));
    int range = 255*255;   // for now limit to size of .gb channels
    int n = 1000;
    int mappedIndex = baseIndex * (range/n);

    int g = mappedIndex / 255;
    int b = mappedIndex - 255*g;  
    color = vec4(0, float(g)/255.0, float(b)/255.0, 255.0);
  }
  gl_FragColor = color;
}