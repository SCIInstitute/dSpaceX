attribute vec2 coordinates;
uniform mat4 uProjectionMatrix;
void main(void) {
  gl_Position = uProjectionMatrix * vec4(coordinates, 0.0, 1.0);
}