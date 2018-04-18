import { Edge, Quad } from './primitives';
import React from 'react';
import { mat4 } from 'gl-matrix';


/**
 * WebGL error check wrapper - logs to console
 * @param {object} gl
 */
let webGLErrorCheck = function(gl) {
  let error = gl.getError();
  if (error != gl.NO_ERROR) {
    const e = new Error();
    const regex = /\((.*):(\d+):(\d+)\)$/;
    const match = regex.exec(e.stack.split('\n')[2]);

    let str = 'GL Error @ line:' + match[2] +
      'in file:' + match[1] + ' Error#: ' + error;
    console.log(str);
  }
};

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

    this.client = this.props.client;
    this.gl = null;
    this.canvas = null;
    this.vertex_array = null;
    this.vertex_buffer = null;
    this.shaderProgram = null;
    this.vertices = null;
    this.indices = null;
    this.nodes = null;
    this.edges = null;
    this.edgeVerts = null;
    this.edgeVerts_array = null;
    this.edgeVerts_buffer = null;
    this.projectionMatrix = mat4.create();

    this.scale = 1;
    this.xOffset = 0;
    this.yOffset = 0;

    this.fakeNodesPositions = null;
    this.fakeEdgesIndices = null;

    this.handleScrollEvent = this.handleScrollEvent.bind(this);
    this.handleMouseDown = this.handleMouseDown.bind(this);
    this.handleMouseRelease = this.handleMouseRelease.bind(this);
    this.resizeCanvas = this.resizeCanvas.bind(this);
  }

  /**
   * Event Handling for scroll wheel
   * @param {Event} evt
   */
  handleScrollEvent(evt) {
    if (evt.deltaY < 0 && this.scale > 0) {
      this.scale -= 0.01;
    }
    if (evt.deltaY > 0 && this.scale < 10) {
      this.scale += 0.01;
    }

    this.resizeCanvas();
  }

  /**
   * Event handling for mouse click down
   * @param {Event} evt
   */
  handleMouseDown(evt) {
    // Handle Right click
    if (evt.button == 2) {
      console.log('Right Mouse Down Event');
      let XY = [evt.offsetX, evt.offsetY];
      let orthoXY = this.convertToGLCoords(XY);
      this.xOffset = -orthoXY[0];
      this.yOffset = -orthoXY[1];

      this.resizeCanvas();
    }
  }

  /**
   * Event handling for mouse click up
   * @param {Event} evt
   */
  handleMouseRelease(evt) {
    console.log('Mouse Up Event');
  }

  /**
   * Initial setup for the webgl canvas.
   */
  initGL() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    gl.clearColor(1.0, 1.0, 1.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    this.createFakeNodePositions();
    this.createFakeEdges();
    this.createGeometry(this.fakeNodesPositions, this.fakeEdgesIndices);
    this.createShaders(gl);
    this.createBuffers(gl);

    webGLErrorCheck(gl);
  }

  /**
   * Set up an orthographic projection.
   * @param {number} width
   * @param {number} height
   */
  setupOrtho(width, height) {
    let sx = 1;
    let sy = 1;

    if (width > height) {
      sx = width / height;
    } else {
      sy = height / width;
    }

    mat4.ortho(
      this.projectionMatrix,
      -1*this.scale*sx + this.xOffset, // left
      +1*this.scale*sx + this.xOffset, // right
      -1*this.scale*sy + this.yOffset, // bottom
      +1*this.scale*sy + this.yOffset, // top
      +1, // near
      -1); // far
  }

  /**
   * Return x,y pixel coords in gl coords
   * @param {array} XY
   * @return {array}
   */
  convertToGLCoords(XY) {
    let canvas = this.refs.canvas;
    let resolution = [canvas.clientWidth, canvas.clientHeight];
    let zeroToOne = [XY[0] / resolution[0], XY[1] / resolution[1]];
    let zeroToTwo = [zeroToOne[0] * 2, zeroToOne[1] * 2];
    let clipSpace = [zeroToTwo[0] - 1, zeroToTwo[1] - 1];
    let returnXY = [clipSpace[0], -clipSpace[1]];
    return returnXY;
  }

  /**
   * Creates the a fake set of nodes for proof of concept
   */
  createFakeNodePositions() {
    let width = 0.2;
    let height = 0.2;

    this.fakeNodesPositions = [];
    for (let y = 0; y < 5; y++) {
      for (let x = 0; x < 5; x++) {
        let pX = ((x - 2) * (width * 2));
        let pY = ((y - 2) * (height * 2));

        this.fakeNodesPositions.push([pX, pY]);
      }
    }
  }

  /**
   * Creates the a fake set of edges for proof of concept
   */
  createFakeEdges() {
    this.fakeEdgesIndices = [];
    // build the edges
    for (let i = 0; i < this.fakeNodesPositions.length-1; i++) {
      this.fakeEdgesIndices[i] = i;
      this.fakeEdgesIndices[i + 1] = i + 1;
    }
  }

  /**
   * Creates the geometry to be rendered.
   * @param {array} array2DVertsForNodes
   * @param {array} arrayBeginEndIndicesForEdges
   * @param {number} quadHeight
   * @param {number} quadWidth
   */
  createGeometry(array2DVertsForNodes, arrayBeginEndIndicesForEdges,
    quadHeight = 0.1, quadWidth = 0.1) {
    this.vertices = [];
    this.indices = [];
    this.edgeVerts = [];
    this.edges = [];
    this.nodes = [];

    // create a quad for each position in array2DVertsForNodes
    for (let i = 0; i < array2DVertsForNodes.length; i++) {
      let firstIndex = 0;
      if (this.indices.length >= 6) {
        firstIndex = (this.indices[this.indices.length - 1]) + 1;
      }
      let quad = new Quad(array2DVertsForNodes[i][0],
        array2DVertsForNodes[i][1],
        quadWidth, quadHeight, firstIndex);
      this.vertices = this.vertices.concat(quad.vertices);
      this.indices = this.indices.concat(quad.indices);
      this.nodes.push(quad);
    }

    // create an Edge for each indicated edge in arrayBeginEndIndicesForEdges
    for (let j = 0; j < arrayBeginEndIndicesForEdges.length-1; j++) {
      let edge = new Edge(this.nodes[j].X, this.nodes[j].Y,
        this.nodes[j+1].X, this.nodes[j+1].Y);

      this.edges.push(edge);
      this.edgeVerts.push(edge.x1, edge.y1, edge.x2, edge.y2);
    }
  }

  /**
   * Compiles vertex and fragment shader programs.
   * @param {object} gl The OpenGL context.
   */
  createShaders(gl) {
    const vertexShaderSource =
      'attribute vec2 coordinates;                                    ' +
      'uniform mat4 uProjectionMatrix;                                 ' +
      'void main(void) {                                              ' +
      '  gl_Position = uProjectionMatrix * vec4(coordinates, 0.0, 1.0);' +
      '}                                           ';
    let vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShader, vertexShaderSource);
    gl.compileShader(vertexShader);

    webGLErrorCheck(gl);

    let fragmentShaderSource =
      'void main(void) {                           ' +
      '  gl_FragColor = vec4(0.8, 0.2, 0.2, 1.0);  ' +
      '}                                           ';
    let fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShader, fragmentShaderSource);
    gl.compileShader(fragmentShader);

    webGLErrorCheck(gl);

    let shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);
    gl.useProgram(shaderProgram);

    this.shaderProgram = shaderProgram;

    webGLErrorCheck(gl);
  }


  /**
   * Creates vertex buffers for dummy data.
   * @param {object} gl The OpenGL context.
   */
  createBuffers(gl) {
    // Quad buffers
    this.vertex_buffer = gl.createBuffer();
    this.vertex_array = new Float32Array(this.vertices);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.vertex_array, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    webGLErrorCheck(gl);

    // Edge buffers
    this.edgeVerts_buffer = gl.createBuffer();
    this.edgeVerts_array = new Float32Array(this.edgeVerts);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.edgeVerts_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.edgeVerts_array, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    webGLErrorCheck(gl);
  }

  /**
   * Updates vertex buffers with fresh data.
   */
  updateBuffers() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    this.vertex_array = new Float32Array(this.vertices);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.vertex_array, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    webGLErrorCheck(gl);

    this.edgeVerts_array = new Float32Array(this.edgeVerts);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.edgeVerts_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.edgeVerts_array, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    webGLErrorCheck(gl);
  }

  /**
   * Resize the canvas renderer to match the dom size.
   * @param {object} gl The OpenGL context.
   */
  resizeCanvas() {
    let canvas = this.refs.canvas;
    canvas.width = canvas.clientWidth;
    canvas.height = canvas.clientHeight;

    this.setupOrtho(canvas.width, canvas.height);
    requestAnimationFrame(this.drawScene.bind(this));
  }

  /**
   * Callback invoked before the React Component is rendered.
   */
  componentWillMount() {
    let { datasetId, k, persistenceLevel } = this.props.decomposition;
    console.log('dataset = ' + datasetId);
    console.log('k = ' + k);
    console.log('persistenceLevel = ' + persistenceLevel);
    this.client
      .fetchLayoutForPersistenceLevel(datasetId, k, persistenceLevel)
      .then(function(result) {
        console.dir(result);
        if (result.embedding && result.embedding.layout) {
          // let layout = [].concat(...result.embedding.layout);
          this.createGeometry(result.embedding.layout, [0, 1], 0.02, 0.02);
          this.updateBuffers();
        }
      }.bind(this));
  }

  /**
   * Callback invoked when the component is shutting down.
   */
  componentWillUnmount() {
    window.removeEventListener('resize', this.resizeCanvas);
    this.refs.canvas.removeEventListener('wheel', this.handleScrollEvent);
    this.refs.canvas.removeEventListener('mousedown', this.handleMouseDown);
    this.refs.canvas.removeEventListener('mouseup', this.handleMouseRelease);
  }

  /**
   * Callback invoked immediately after the component is mounted.
   */
  componentDidMount() {
    this.initGL();
    this.resizeCanvas();
    window.addEventListener('resize', this.resizeCanvas);
    this.refs.canvas.addEventListener('wheel', this.handleScrollEvent);
    this.refs.canvas.addEventListener('mousedown', this.handleMouseDown);
    this.refs.canvas.addEventListener('mouseup', this.handleMouseRelease);
    this.refs.canvas.addEventListener('contextmenu', function(e) {
      e.preventDefault();
    }, false);
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

    webGLErrorCheck(gl);

    let projectionMatrixLocation =
        gl.getUniformLocation(this.shaderProgram, 'uProjectionMatrix');
    gl.uniformMatrix4fv(projectionMatrixLocation, false, this.projectionMatrix);


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

    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.viewport(0, 0, canvas.width, canvas.height);


    gl.drawElements(gl.TRIANGLES, this.indices.length,
      gl.UNSIGNED_SHORT, indexBuffer);

    webGLErrorCheck(gl);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.edgeVerts_buffer);
    coordinateAttrib = gl.getAttribLocation(this.shaderProgram, 'coordinates');
    gl.vertexAttribPointer(coordinateAttrib, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coordinateAttrib);
    gl.drawArrays(gl.LINES, 0, this.edgeVerts.length / 2);

    webGLErrorCheck(gl);
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
