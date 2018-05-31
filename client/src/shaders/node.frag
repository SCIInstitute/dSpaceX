precision mediump float;
uniform float nodeOutline;
uniform float nodeSmoothness;
varying vec2 vertexUV;
varying vec3 geomColor;
void main(void) {
  vec2 uv = vertexUV.xy;
  vec2 center = vec2(0.5);
  float radius = 0.425;
  float thickness = nodeOutline;
  float blur = nodeSmoothness;
  float t = distance(uv, center) - radius;
  vec4 fillColor = vec4(1.0, 1.0, 1.0, 1.0);
  vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 lineColor = vec4(mix(black.xyz, geomColor, 0.4), 1.0);
  vec4 clear = vec4(1.0, 1.0, 1.0, 0.0);
  vec4 fill = clear;
  if (t < 0.0) {
    t = abs(t);
    fill = vec4(geomColor,1.0);
  }
  float step1 = thickness;
  float step2 = thickness + blur;
  gl_FragColor = mix(lineColor, fill, smoothstep(step1, step2, t));
}