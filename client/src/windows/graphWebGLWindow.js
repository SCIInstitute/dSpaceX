import React from 'react';


/**
 * A WebGL Window Component for rendering Graphs.
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
    this.vertex_array = null;
    this.vertex_buffer = null;
    this.shaderProgram = null;
    this.vertices = null;
    this.indices = null;
  }

  /**
     * Initial setup for the webgl canvas.
     */
  initGL() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    gl.clearColor(0.4, 0.4, 0.4, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    this.createGeometry();

    this.createShaders(gl);
    this.createBuffers(gl);
  }

  /**
   * Creates the geometry to be rendered.
   */
  createGeometry() {
    let width = .1;
    let height = .1;
    let posXY = [-0.5, 0.5];

    let minX = -(width / 2.0) + posXY[0];
    let maxX = minX + (width / 2.0);
    let minY = -(height / 2.0) + posXY[1];
    let maxY = minY + (height / 2.0);

    this.vertices = [
      minX, maxY,
      maxX, maxY,
      minX, minY,
      maxX, minY];

    this.indices = [
      0, 1, 2,
      2, 1, 3];

    for (let i = 1; i < 4; i++) {
      minX = maxX + width;
      maxX = minX + width + 0.02 * i;
      maxY += height + 0.01 * i;

      let additionalVerts = [
        minX, maxY,
        maxX, maxY,
        minX, minY,
        maxX, minY];

      this.vertices = this.vertices.concat(additionalVerts);

      let prevFirstIndex = this.indices[this.indices.length - 6];
      let additionalIndices = [
        prevFirstIndex + 4, prevFirstIndex + 5, prevFirstIndex + 6,
        prevFirstIndex + 6, prevFirstIndex + 5, prevFirstIndex + 7];

      this.indices = this.indices.concat(additionalIndices);
    }
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
   * React to new props. Reset views if dataset changes.
   * @param {object} nextProps
   */
  componentWillReceiveProps(nextProps) {
    // TODO:  Add logic to recompute vertexbuffers etc as required.
  }

  /**
   * Renders the OpenGL Content to the canvas.
   */
  drawScene() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    // TODO: pull out into method
    let indexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(this.indices),
      gl.STATIC_DRAW);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);

    let coordinateAttrib =
            gl.getAttribLocation(this.shaderProgram, 'coordinates');
    gl.vertexAttribPointer(coordinateAttrib, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coordinateAttrib);
    gl.enable(gl.DEPTH_TEST);

    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.viewport(0, 0, canvas.width, canvas.height);

    // gl.drawArrays(gl.TRIANGLES, 0, 6);
    gl.drawElements(gl.TRIANGLES, this.indices.length,
      gl.UNSIGNED_SHORT, indexBuffer);
  }

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
