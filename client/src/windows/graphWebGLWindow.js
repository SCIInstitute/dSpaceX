import React from 'react';


/**
 * //Quad will need to be pulled into a seperate utils.js or better shapes.js and inherit from a base 'Shape'
 * @param {any} centerPositionInPixels
 * @param {any} widthInPixels
 * @param {any} heightInPixels
 */
var Quad = function (centerPositionInPixels, widthInPixels, heightInPixels) {
  this.position = centerPositionInPixels;
  this.width = widthInPixels;
  this.height = heightInPixels;

  this.numVerts = 4;
  this.numIndices = 6;

  let canvas = this.refs.canvas;
  var quadWidth = this.width / canvas.clientWidth;
  var quadHeight = this.height / canvas.clientHeight;
  var minX = -(quadWidth / 2.0);
  var maxX = (quadWidth / 2.0);
  var minY = -(quadHeight / 2.0);
  var maxY = (quadHeight / 2.0);

  this.vertices = [
    minX, maxY,
    maxX, maxY,
    minX, minY,
    maxX, minY];
}


/**
 * A base WebGL Window Component.
 */
class GraphWebGLWindow extends React.Component {
  /**
     * GraphWebGLWindow constructor.
     * @param {object} props
     */
  constructor(props) {
    super(props);

    this.gl = null;
    this.canvas = null;
    this.vertices = null;
    this.vertex_array = null;
    this.vertex_buffer = null;
    this.shaderProgram = null;
  }

  /**
     * Initial setup for the webgl canvas.
     */
  initGL() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    gl.clearColor(0.4, 0.4, 0.4, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    this.createShaders(gl);
    this.createBuffers(gl);
  }

  /**
     * Compiles vertex and fragment shader programs.
     * @param {object} gl The OpenGL context.
     */
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
            '  gl_FragColor = vec4(0.8, 0.2, 0.2, 1.0);  ' +
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


  
  /**
     * Creates vertex buffers for dummy data.
     * @param {object} gl The OpenGL context.
     */
  createBuffers(gl) {
    let canvas = this.refs.canvas;
    var quadWidth = 500 / canvas.clientWidth;
    var quadHeight = 500 / canvas.clientHeight;
    var minX = -(quadWidth / 2.0);
    var maxX = (quadWidth / 2.0);
    var minY = -(quadHeight / 2.0);
    var maxY = (quadHeight / 2.0);

    this.vertices =
      [minX, maxY,
      maxX, maxY,
        minX, minY,
        maxX, minY];
    this.vertex_buffer = gl.createBuffer();
    this.vertex_array = new Float32Array(this.vertices);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.vertex_array, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
  }

  /**
     * Resize the canvas renderer to match the dom size.
     * @param {object} gl The OpenGL context.
     */
  resizeCanvas() {
    let canvas = this.refs.canvas;
    canvas.width = canvas.clientWidth;
    canvas.height = canvas.clientHeight;
    
    //this.createBuffers(gl);
  }

  /**
     * Callback invoked before the component receives new props.
     */
  componentWillUnmount() {
    window.removeEventListener('resize', this.resizeCanvas.bind(this));
  }

  /**
     * Callback invoked immediately after the component is mounted.
     */
  componentDidMount() {
    this.initGL();
    this.resizeCanvas();
    window.addEventListener('resize', this.resizeCanvas.bind(this));
    requestAnimationFrame(this.drawScene.bind(this));
  }

  /**
     * Renders the OpenGL Content to the canvas.
     */
  drawScene() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    ////pull out into method
    var indices = [0, 1, 2, 2, 1, 3];
    var index_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, index_buffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);
    ////

    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);

    let coordinateAttrib =
            gl.getAttribLocation(this.shaderProgram, 'coordinates');
    gl.vertexAttribPointer(coordinateAttrib, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coordinateAttrib);
    gl.enable(gl.DEPTH_TEST);

    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.viewport(0, 0, canvas.width, canvas.height);

    //gl.drawArrays(gl.TRIANGLES, 0, 6);
    gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, index_buffer);
  }

  /**
    * function to 

  /**
     * Renders the component to HTML.
     * @return {HTML}
     */
  render() {
    let style = {
      width: '100%',
      height: '100%',
      borderRight: '1px dashed gray',
      boxSizing: 'border-box',
    };
    return (
      <canvas ref='canvas' className='glCanvas' style={style} />
    );
  }
}

export default GraphWebGLWindow;
