attribute vec3 verts;
varying vec2 uv;

void main(void) {
  float u = max(min(verts.x, 1.0), 0.0);
  float v = max(min(verts.y, 1.0), 0.0);
  uv = vec2(u,v);

  float x = 0.5*verts.x + 0.5;
  float y = 0.5*verts.y + 0.5;

  gl_Position = vec4(x, y, 0, 1.0);
}