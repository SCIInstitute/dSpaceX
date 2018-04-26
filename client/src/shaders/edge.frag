precision mediump float;
uniform vec3 edgeOpacity;
varying vec2 vertexUV;

void main(void) {
  float center = 0.5;
  float thickness = 0.25; // edgeOpacity.x;
  float blur = 0.2; // edgeOpacity.y;
  float t = abs(vertexUV.y - 0.5);
  vec4 black = vec4(0.0, 0.0, 0.0, edgeOpacity.z);
  vec4 clear = vec4(1.0, 1.0, 1.0, 0.0);
  float step1 = thickness;
  float step2 = thickness + blur;
  gl_FragColor = mix(black, clear, smoothstep(step1, step2, t));
}

