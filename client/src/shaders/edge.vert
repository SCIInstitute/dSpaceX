attribute vec3 coordinates;
attribute vec3 vertex_color;
uniform mat4 uProjectionMatrix;
verying vec2 vertexUV;
varying vec3 geomColor;

void main(void) {
  int UVindex = int(coordinates.z);
  if (UVindex == 0)
    vertexUV = vec2(0, 0);
  else if (UVindex == 1)
    vertexUV = vec2(1, 0);
  else if (UVindex == 2)
    vertexUV = vec2(0, 1);
  else if (UVindex == 3)
    vertexUV = vec2(1, 1);
  geomColor = vertex_color;
  gl_Position = uProjectionMatrix * vec4(coordinates.xy, 0.0, 1.0);
}