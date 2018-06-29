precision mediump float;
uniform sampler2D imageTex;
varying vec2 uv;

void main(void) {
  vec4 texColor = vec4(texture2D(imageTex, uv).rgb, 1.0);
  vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 color = texColor;
  if (uv.x <= 0.015 || uv.x >= 0.985 ||
      uv.y <= 0.025 || uv.y >= 0.975) {
    color = black;
  }
  gl_FragColor = color;
}