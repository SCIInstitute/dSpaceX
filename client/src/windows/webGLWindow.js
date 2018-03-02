import React from 'react';


class WebGLWindow extends React.Component {
  constructor(props) {
    super(props);

    this.gl = null;
    this.canvas = null;
    this.vertices = null;
    this.vertex_buffer = null;
    this.shaderProgram = null;
  }

  initGL() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    gl.clearColor(1.0, 1.0, 1.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    this.createShaders(gl);
    this.createBuffers(gl);
  }

  createShaders(gl) {    
    const vertexShaderSource = 
      'attribute vec2 coordinates;                 ' +
      'void main(void) {                           ' +
      '  gl_Position = vec4(coordinates, 0.0, 1.0);' +
      '}                                           ';
    let vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShader, vertexShaderSource);
    gl.compileShader(vertexShader);

    let fragmentShaderSource = 
      'void main(void) {                           ' +
      '  gl_FragColor = vec4(0.0, 0.5, 0.0, 1.0);  ' +
      '}                                           ';
    let fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShader, fragmentShaderSource);
    gl.compileShader(fragmentShader); 

    let shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);
    gl.useProgram(shaderProgram);   

    this.shaderProgram = shaderProgram;
  }

  createBuffers(gl) {
    this.vertices = [0, 0.5, -0.5, -0.5, 0.5, -0.5];
    this.vertex_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(this.vertices), gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null); 
  }

  resizeCanvas() {
    let canvas = this.refs.canvas;
    canvas.width  = canvas.clientWidth;
    canvas.height = canvas.clientHeight;
  }

  componentDidMount() {
    this.initGL();     
    this.resizeCanvas();
    window.addEventListener("resize", this.resizeCanvas.bind(this)); 
    requestAnimationFrame(this.drawScene.bind(this));
  }

  componentWillUnmount() {
    window.removeEventListener("resize", this.resizeCanvas.bind(this));
  }

  drawScene() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);

    let coordinateAttrib = gl.getAttribLocation(this.shaderProgram, 'coordinates');
    gl.vertexAttribPointer(coordinateAttrib, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coordinateAttrib);
    gl.enable(gl.DEPTH_TEST);

    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.drawArrays(gl.TRIANGLES, 0, 3);
  }

  render() {
    return (
      <canvas ref='canvas' className='glCanvas'
              style={{width: '100%', height: '100%', border: '1px dashed gray'}}>
      </canvas>
    );
  }
}

export default WebGLWindow;