precision mediump float;
uniform float edgeOpacity;
varying vec2 vertexUV;
varying vec3 geomColor;

void main(void) {
  float center = 0.5;
  float thickness = 0.25;
  float blur = 0.2;
  float t = abs(vertexUV.y - 0.5);
  vec4 black = vec4(0.0, 0.0, 0.0, edgeOpacity);
  vec4 clear = vec4(1.0, 1.0, 1.0, 0.0);
  float step1 = thickness;
  float step2 = thickness + blur;
  gl_FragColor = vec4(0, 0, 0, 1); // mix(black, clear, smoothstep(step1, step2, t));
}

