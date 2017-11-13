#version 150

in vec2 Vertex_UV;
in vec3 geom_color;
out vec4 frag_color;

void main() {
  vec2 uv = Vertex_UV.xy;
  vec2 center = vec2(0.5);
  float radius = 0.45;
  float thickness = 0.01;
  float blur = 0.05;
  float t = distance(uv, center) - radius;
  vec4 fillColor = vec4(1.0, 1.0, 1.0, 1.0);
  vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 clear = vec4(1.0, 1.0, 1.0, 0.0);
  vec4 fill = clear;
  if (t < 0.0) {
    t = abs(t);
    fill = vec4(geom_color, 1.0);
  }
  
  float step1 = thickness;
  float step2 = thickness + blur;
  frag_color = mix(black, fill, smoothstep(step1, step2, t));
}