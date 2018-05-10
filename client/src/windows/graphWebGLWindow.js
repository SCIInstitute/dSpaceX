import * as d3 from 'd3';
import { Edge, Quad } from './primitives';
import EdgeFragmentShaderSource from '../shaders/edge.frag';
import EdgeVertexShaderSource from '../shaders/edge.vert';
import NodeFragmentShaderSource from '../shaders/node.frag';
import NodeVertexShaderSource from '../shaders/node.vert';
import React from 'react';
import { mat4 } from 'gl-matrix';


const zoomRate = 1.2;
const maxScale = 10;

const constEdgeThickness = 0.0025;
const constEdgeSmoothness = 0.9;
const constEdgeOpacity = 0.95;

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
      ' in file:' + match[1] + ' Error#: ' + error;
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
    this.vertColor_array = null;
    this.vertColor_buffer = null;
    this.nodeShaderProgram = null;
    this.edgeShaderProgram = null;
    this.vertices = null;
    this.vertColors = null;
    this.nodes = null;
    this.edges = null;
    this.nodeEdgeCount = null;
    this.edgeVerts = null;
    this.edgeVerts_array = null;
    this.edgeVerts_buffer = null;

    this.projectionMatrix = mat4.create();
    this.bDrawEdgesAsQuads = null;

    this.scale = 1;
    this.xOffset = 0;
    this.yOffset = 0;

    this.previousX = 0;
    this.previousY = 0;
    this.netPanX = 0;
    this.netPanY = 0;
    this.rightMouseDown = false;

    this.handleScrollEvent = this.handleScrollEvent.bind(this);
    this.handleMouseDown = this.handleMouseDown.bind(this);
    this.handleMouseRelease = this.handleMouseRelease.bind(this);
    this.handleMouseMove = this.handleMouseMove.bind(this);
    this.resizeCanvas = this.resizeCanvas.bind(this);
    this.handleKeyDown = this.handleKeyDown.bind(this);

    this.edgeThickness = 0.0;
    this.edgeSmoothness = 0.0;
    this.edgeOpacity = 0.0;
  }

  /**
   * Event Handling for scroll wheel
   * @param {Event} evt
   */
  handleScrollEvent(evt) {
    if (evt.deltaY < 0 && this.scale > -maxScale) {
      this.scale = this.scale / zoomRate;
    }
    if (evt.deltaY > 0 && this.scale < maxScale) {
      this.scale = this.scale * zoomRate;
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
      this.rightMouseDown = true;
      this.previousX = evt.offsetX;
      this.previousY = evt.offsetY;
    }
  }

  /**
   * Event handling for mouse click up
   * @param {Event} evt
   */
  handleMouseRelease(evt) {
    if (evt.button == 2) {
      this.rightMouseDown = false;
    }
  }

  /**
   * Event handling for mouse movement
   * @param {Event} evt
   */
  handleMouseMove(evt) {
    if (this.rightMouseDown) {
      let x = evt.offsetX;
      let y = evt.offsetY;

      let dx = (x - this.previousX);
      let dy = (y - this.previousY);

      this.previousX = x;
      this.previousY = y;

      this.xOffset -= this.pixelToGeometryRatioX * dx;
      this.yOffset += this.pixelToGeometryRatioY * dy;

      this.resizeCanvas();
    }
  }

  /**
   * Event handling for keyboard input
   * @param {Event} evt
   */
  handleKeyDown(evt) {
    const keyName = evt.key;
    switch (keyName) {
    case 'ArrowUp':
      console.log('Up arrow pressed');
      break;
    case 'ArrowDown':
      console.log('Down arrow pressed');
      break;
    case 'ArrowLeft':
      console.log('Left arrow presses');
      break;
    case 'ArrowRight':
      console.log('Right arrow pressed');
      break;
    default:
      console.log('*Input Key Value Not Handled!');
    }
  }

  /**
   * Initial setup for the webgl canvas.
   */
  initGL() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    gl.clearColor(1.0, 1.0, 1.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.depthMask(false);
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

    // Create fake data if there's no decomposition information.
    if (!this.props.decomposition) {
      let fakeNodePositions = this.createFakeNodePositions();
      let fakeEdgesIndices = this.createFakeEdges(fakeNodePositions);
      this.createGeometry(fakeNodePositions, fakeEdgesIndices);
      let fakeNodeColors = this.createFakeNodeColors();
      this.addVertexColors(fakeNodeColors);
    }
    this.createShaders(gl);
    this.createBuffers(gl);

    webGLErrorCheck(gl);

    this.edgeThickness = constEdgeThickness;
    this.edgeSmoothness = constEdgeSmoothness;
    this.edgeOpacity = constEdgeOpacity;
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

    let geomLeft = -1 * sx * this.scale + this.xOffset;
    let geomRight = +1 * sx * this.scale + this.xOffset;
    let geomBottom = -1 * sy * this.scale + this.yOffset;
    let geomTop = +1 * sy * this.scale + this.yOffset;

    let geomWidth = Math.abs(geomRight - geomLeft);
    let geomHeight = Math.abs(geomTop - geomBottom);

    let canvas = this.refs.canvas;
    this.pixelToGeometryRatioX = geomWidth / canvas.clientWidth;
    this.pixelToGeometryRatioY = geomHeight / canvas.clientHeight;

    mat4.ortho(
      this.projectionMatrix,
      geomLeft,
      geomRight,
      geomBottom,
      geomTop,
      +1 /* near */,
      -1 /* far */);
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
   * Creates a fake set of node [x,y] positions for testing.
   * @return {Array}
   */
  createFakeNodePositions() {
    let fakeNodesPositions = [];
    let radius = 0.5;
    let pointCount = 16;
    let angle = Math.PI * 2 / pointCount;
    for (let i = 0; i < pointCount; i++) {
      let pX = radius * Math.cos(angle * i);
      let pY = radius * Math.sin(angle * i);
      fakeNodesPositions.push([pX, pY]);
    }
    return fakeNodesPositions;
  }

  /**
   * Creates a fake set of edge indices for testing.
   * @param {Array} nodePositions
   * @return {Array}
   */
  createFakeEdges(nodePositions) {
    let fakeEdgesIndices = [];

    for (let i = 0; i < nodePositions.length; i++) {
      let min = 0;
      let max = nodePositions.length - 1;
      min = Math.ceil(min);
      max = Math.floor(max);
      let randomNodeIndex = Math.floor(Math.random() * (max - min)) + min;
      fakeEdgesIndices.push([i, randomNodeIndex]);
    }

    for (let j = 0; j < 16; j++) {
      let min = 0;
      let max = nodePositions.length - 1;
      min = Math.ceil(min);
      max = Math.floor(max);
      let randomFirst = Math.floor(Math.random() * (max - min)) + min;
      let randomSecond = Math.floor(Math.random() * (max - min)) + min;
      while (randomSecond == randomFirst) {
        randomSecond = Math.floor(Math.random() * (max - min)) + min;
      }
      fakeEdgesIndices.push([randomFirst, randomSecond]);
    }
    return fakeEdgesIndices;
  }

  /**
   * Creates fakeNodeColors for proof of concept
   * @return {Array}
   */
  createFakeNodeColors() {
    let fakeNodeColors = [];
    for (let i = 0; i < this.nodes.length; i++) {
      let r = 1.0 - (1.0 / this.nodes.length * i);
      let g = 0.8;
      let b = 0.0 + (1.0 / this.nodes.length * i);
      fakeNodeColors.push(r, g, b);
    }
    return fakeNodeColors;
  }

  /**
   * Creates the geometry to be rendered.
   * @param {array} array2DVertsForNodes
   * @param {array} arrayBeginEndIndicesForEdges
   * @param {number} quadHeight
   * @param {number} quadWidth
   * @param {bool} drawEdgesAsQuads
   */
  createGeometry(array2DVertsForNodes, arrayBeginEndIndicesForEdges,
    quadHeight = 0.1, quadWidth = 0.1, drawEdgesAsQuads = true) {
    this.vertices = [];
    this.vertColors = [];
    this.edgeVerts = [];
    this.edges = [];
    this.nodes = [];
    this.bDrawEdgesAsQuads = drawEdgesAsQuads;

    // create a quad for each position in array2DVertsForNodes
    for (let i = 0; i < array2DVertsForNodes.length; i++) {
      let quad = new Quad(array2DVertsForNodes[i][0],
        array2DVertsForNodes[i][1],
        quadWidth, quadHeight);
      this.vertices = this.vertices.concat(quad.vertices);
      this.nodes.push(quad);
    }

    // create an Edge for each indicated edge in arrayBeginEndIndicesForEdges
    for (let i = 0; i < arrayBeginEndIndicesForEdges.length; i++) {
      let index1 = arrayBeginEndIndicesForEdges[i][0];
      let index2 = arrayBeginEndIndicesForEdges[i][1];

      let node1 = this.nodes[index1];
      let node2 = this.nodes[index2];

      let edge = new Edge(node1.X, node1.Y, node2.X, node2.Y);

      this.edges.push(edge);
      if (this.bDrawEdgesAsQuads) {
        this.edgeVerts = this.edgeVerts.concat(edge.vertices);
      } else {
        // push back xy1, uv1, xy2, uv2
        this.edgeVerts.push(edge.x1, edge.y1, 0.0, edge.x2, edge.y2, 1.0);
      }
    }
  }

  /**
  * Adds colors to go with the nodes
  * @param {array} arrayRGBColors
  */
  addVertexColors(arrayRGBColors) {
    this.vertColors = [];
    for (let i = 0; i < arrayRGBColors.length-2; i+=3) {
      for (let j = 0; j < 6; j++) {
        this.vertColors.push(arrayRGBColors[i], arrayRGBColors[i+1],
          arrayRGBColors[i+2]);
      }
    }
  }

  /**
   * Compiles vertex and fragment shader programs.
   * @param {object} gl The OpenGL context.
   */
  createShaders(gl) {
    let nodeVertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(nodeVertexShader, NodeVertexShaderSource);
    gl.compileShader(nodeVertexShader);

    webGLErrorCheck(gl);

    let nodeFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(nodeFragmentShader, NodeFragmentShaderSource);
    gl.compileShader(nodeFragmentShader);

    webGLErrorCheck(gl);

    let shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, nodeVertexShader);
    gl.attachShader(shaderProgram, nodeFragmentShader);
    gl.linkProgram(shaderProgram);
    gl.useProgram(shaderProgram);

    this.nodeShaderProgram = shaderProgram;

    webGLErrorCheck(gl);

    let edgeVertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(edgeVertexShader, EdgeVertexShaderSource);
    gl.compileShader(edgeVertexShader);

    webGLErrorCheck(gl);

    let edgeFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(edgeFragmentShader, EdgeFragmentShaderSource);
    gl.compileShader(edgeFragmentShader);

    webGLErrorCheck(gl);

    let edgeShaderProgram = gl.createProgram();
    gl.attachShader(edgeShaderProgram, edgeVertexShader);
    gl.attachShader(edgeShaderProgram, edgeFragmentShader);
    gl.linkProgram(edgeShaderProgram);
    gl.useProgram(edgeShaderProgram);

    webGLErrorCheck(gl);

    this.edgeShaderProgram = edgeShaderProgram;
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

    // vertex Color buffers
    this.vertColor_buffer = gl.createBuffer();
    this.vertColor_array = new Float32Array(this.vertColors);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertColor_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.vertColor_array, gl.STATIC_DRAW);
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

    this.vertColor_array = new Float32Array(this.vertColors);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertColor_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.vertColor_array, gl.STATIC_DRAW);
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
    if (!this.props.decomposition) {
      return;
    }
    let { datasetId, k, persistenceLevel } = this.props.decomposition;
    this.client
      .fetchLayoutForPersistenceLevel(datasetId, k, persistenceLevel)
      .then(function(result) {
        if (result.embedding && result.embedding.layout) {
          // let layout = [].concat(...result.embedding.layout);
          let layout = result.embedding.layout;
          let adjacency = result.embedding.adjacency;
          this.createGeometry(layout, adjacency, 0.02, 0.02);

          if (this.props.qoi) {
            let min = Math.min(...this.props.qoi);
            let max = Math.max(...this.props.qoi);
            let color = d3.scaleLinear()
              .domain([min, 0.5*(min+max), max])
              .range(['blue', 'white', 'red']);
            let colorsArray = [];
            for (let i = 0; i < this.props.qoi.length; i++) {
              let colorString = color(this.props.qoi[i]);
              let colorTriplet = colorString.match(/([0-9]+\.?[0-9]*)/g);
              colorTriplet[0] /= 255;
              colorTriplet[1] /= 255;
              colorTriplet[2] /= 255;
              colorsArray.push(...colorTriplet);
            }
            this.addVertexColors(colorsArray);
          } else {
            let colorsArray = [];
            for (let i=0; i < layout.length; i++) {
              colorsArray.push(1.0, 1.0, 1.0);
            }
            this.addVertexColors(colorsArray);
          }
        } else {
          // For now, if server fails. Render fake data.
          if (this.props.decomposition) {
            let fakeNodePositions = this.createFakeNodePositions();
            let fakeEdgeIndices = this.createFakeEdges(fakeNodePositions);
            this.createGeometry(fakeNodePositions, fakeEdgeIndices);
            let fakeNodeColors = this.createFakeNodeColors();
            this.addVertexColors(fakeNodeColors);
          }
        }
        this.updateBuffers();
        requestAnimationFrame(this.drawScene.bind(this));
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
    this.refs.canvas.removeEventListener('mousemove', this.handleMouseMove);
    this.refs.canvas.removeEventListener('keypress', this.handleKeyDown);
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
    this.refs.canvas.addEventListener('mousemove', this.handleMouseMove);
    this.refs.canvas.addEventListener(
      'contextmenu', (e) => e.preventDefault(), false);
    this.refs.canvas.addEventListener('keypress', this.handleKeyDown);
    requestAnimationFrame(this.drawScene.bind(this));
  }

  /**
   * React to new props. Reset views if dataset changes.
   * @param {object} nextProps
   */
  componentWillReceiveProps(nextProps) {
    // TODO:  Simplify and remove code duplication.
    if (!nextProps.decomposition) {
      return;
    }
    let { datasetId, k, persistenceLevel } = nextProps.decomposition;
    let { qoi } = nextProps;

    if ( datasetId == this.props.decomposition.datasetId &&
         k == this.props.decomposition.k &&
         persistenceLevel == this.props.decomposition.persistenceLevel &&
         qoi == this.props.qoi ) {
      return;
    }

    this.client
      .fetchLayoutForPersistenceLevel(datasetId, k, persistenceLevel)
      .then(function(result) {
        if (result.embedding && result.embedding.layout) {
          // let layout = [].concat(...result.embedding.layout);
          let layout = result.embedding.layout;
          let adjacency = result.embedding.adjacency;
          this.createGeometry(layout, adjacency, 0.02, 0.02);

          if (nextProps.qoi) {
            let min = Math.min(...nextProps.qoi);
            let max = Math.max(...nextProps.qoi);
            let color = d3.scaleLinear()
              .domain([min, 0.5*(min+max), max])
              .range(['blue', 'white', 'red']);
            let colorsArray = [];
            for (let i = 0; i < nextProps.qoi.length; i++) {
              let colorString = color(nextProps.qoi[i]);
              let colorTriplet = colorString.match(/([0-9]+\.?[0-9]*)/g);
              colorTriplet[0] /= 255;
              colorTriplet[1] /= 255;
              colorTriplet[2] /= 255;
              colorsArray.push(...colorTriplet);
            }
            this.addVertexColors(colorsArray);
          } else {
            let colorsArray = [];
            for (let i=0; i < layout.length; i++) {
              colorsArray.push(1.0, 1.0, 1.0);
            }
            this.addVertexColors(colorsArray);
          }
        } else {
          // For now, if server fails. Render fake data.
          if (nextProps.decomposition) {
            let fakeNodePositions = this.createFakeNodePositions();
            let fakeEdgeIndices = this.createFakeEdges(fakeNodePositions);
            this.createGeometry(fakeNodePositions, fakeEdgeIndices);
            let fakeNodeColors = this.createFakeNodeColors();
            this.addVertexColors(fakeNodeColors);
          }
        }
        this.updateBuffers();
        requestAnimationFrame(this.drawScene.bind(this));
      }.bind(this));
  }

  /**
   * Render Graph Nodes
   * @param {object} gl
   */
  drawNodes(gl) {
    gl.enable(gl.CULL_FACE);
    gl.frontFace(gl.CW);
    gl.cullFace(gl.BACK);
    gl.useProgram(this.nodeShaderProgram);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);
    let coordinateAttrib =
        gl.getAttribLocation(this.nodeShaderProgram, 'coordinates');
    gl.vertexAttribPointer(coordinateAttrib, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coordinateAttrib);

    if (this.vertColors && this.vertColors.length == this.vertices.length) {
      gl.bindBuffer(gl.ARRAY_BUFFER, this.vertColor_buffer);
      this.nodeShaderProgram.vertexColorAttribute =
        gl.getAttribLocation(this.nodeShaderProgram, 'vertexColor');
      gl.vertexAttribPointer(this.nodeShaderProgram.vertexColorAttribute,
        3, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(this.nodeShaderProgram.vertexColorAttribute);
    }

    let projectionMatrixLocation =
        gl.getUniformLocation(this.nodeShaderProgram, 'uProjectionMatrix');
    gl.uniformMatrix4fv(projectionMatrixLocation, false, this.projectionMatrix);

    gl.drawArrays(gl.TRIANGLES, 0, this.vertices.length / 3);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    webGLErrorCheck(gl);
  }

  /**
   * Render Graph Edges
   * @param {object} gl
   */
  drawEdges(gl) {
    gl.disable(gl.CULL_FACE);
    gl.useProgram(this.edgeShaderProgram);
    let projectionMatrixLocation =
        gl.getUniformLocation(this.edgeShaderProgram, 'uProjectionMatrix');
    gl.uniformMatrix4fv(projectionMatrixLocation, false, this.projectionMatrix);

    let edgeParamsLocation =
      gl.getUniformLocation(this.edgeShaderProgram, 'edgeParams');
    gl.uniform3f(edgeParamsLocation, this.edgeThickness,
      this.edgeSmoothness, this.edgeOpacity);

    let coordinateAttrib =
        gl.getAttribLocation(this.edgeShaderProgram, 'coordinates');
    gl.bindBuffer(gl.ARRAY_BUFFER, this.edgeVerts_buffer);
    gl.vertexAttribPointer(coordinateAttrib, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coordinateAttrib);

    if (this.bDrawEdgesAsQuads) {
      gl.drawArrays(gl.TRIANGLES, 0, this.edgeVerts.length / 3);
    } else {
      gl.drawArrays(gl.LINES, 0, this.edgeVerts.length / 3);
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    webGLErrorCheck(gl);
  }

  /**
   * Renders the OpenGL Content to the canvas.
   */
  drawScene() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.viewport(0, 0, canvas.width, canvas.height);

    // TODO: Replace with a safer check. Maybe add boolean to class.
    if (this.vertices) {
      this.drawEdges(gl);
      this.drawNodes(gl);
    }
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
