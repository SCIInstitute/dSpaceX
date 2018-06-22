import * as d3 from 'd3';
import { Edge, Quad } from './primitives';
import EdgeFragmentShaderSource from '../shaders/edge.frag';
import EdgeVertexShaderSource from '../shaders/edge.vert';
import NodeFragmentShaderSource from '../shaders/node.frag';
import NodeVertexShaderSource from '../shaders/node.vert';
import React from 'react';
import ThumbnailFragmentShaderSource from '../shaders/thumbnail.frag';
import ThumbnailVertexShaderSource from '../shaders/thumbnail.vert';
import { mat4 } from 'gl-matrix';
import { webGLErrorCheck } from './glUtils';

const zoomRate = 1.2;
const maxScale = 10;

const defaultEdgeThickness = 0.075 / 50;
const defaultEdgeSmoothness = 0.2;
const defaultEdgeOpacity = 0.05;


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
    this.canvas = null;
    this.vertex_array = null;
    this.vertex_buffer = null;
    this.vertColor_array = null;
    this.vertColor_buffer = null;
    this.sampleIndex_array = null;
    this.sampleIndex_buffer = null;
    this.nodeShaderProgram = null;
    this.edgeShaderProgram = null;
    this.thumbnailShaderProgram = null;
    this.activeNodeShader = null;
    this.vertices = null;
    this.vertColors = null;
    this.sampleIndexes = null;
    this.nodes = null;
    this.edges = null;
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
    this.tDown = false;
    this.sDown = false;
    this.oDown = false;

    this.handleScrollEvent = this.handleScrollEvent.bind(this);
    this.handleMouseDown = this.handleMouseDown.bind(this);
    this.handleMouseRelease = this.handleMouseRelease.bind(this);
    this.handleMouseMove = this.handleMouseMove.bind(this);
    this.resizeCanvas = this.resizeCanvas.bind(this);
    this.handleKeyDown = this.handleKeyDown.bind(this);

    this.edgeThickness = 0.0;
    this.edgeSmoothness = 0.0;
    this.edgeOpacity = 0.0;

    this.nodeOutline = 0.025;
    this.nodeSmoothness = 0.05;

    this.thumbnails = null;
    this.thumbnailsAtlasTexture = null;
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
   * Event handling for keydown event
   * @param {Event} event
   */
  handleKeyDown(event) {
    switch (event.key) {
    case '/':
      for (let i = 0; i < this.nodes.length; i++) {
        this.nodes[i].decreaseRadius(1.1);
        let count = this.nodes[i].vertices.length;
        for (let j = 0; j < count; j++) {
          this.vertices[i*count+j] = this.nodes[i].vertices[j];
        }
      }
      this.updateBuffers();
      break;
    case '\'':
      for (let i = 0; i < this.nodes.length; i++) {
        this.nodes[i].increaseRadius(1.1);
        let count = this.nodes[i].vertices.length;
        for (let j = 0; j < count; j++) {
          this.vertices[i*count+j] = this.nodes[i].vertices[j];
        }
      }
      this.updateBuffers();
      break;
    case '.':
      this.nodeOutline = Math.max(0.00002, this.nodeOutline/1.1);
      break;
    case ';':
      this.nodeOutline *= 1.1;
      break;
    case ',':
      this.nodeSmoothness = Math.max(0.0002, this.nodeSmoothness / 1.1);
      break;
    case 'l':
      this.nodeSmoothness *= 1.1;
      break;
    case 'm':
      for (let i = 0; i < this.edges.length; i++) {
        this.edges[i].decreaseThickness(1.1);
        let count = this.edges[i].vertices.length;
        for (let j = 0; j < count; j++) {
          this.edgeVerts[i*count+j] = this.edges[i].vertices[j];
        }
      }
      this.updateBuffers();
      this.edgeThickness = Math.max(0.0001, this.edgeThickness / 1.1);
      console.log('edgeThickness = ' + this.edgeThickness);
      break;
    case 'k':
      for (let i = 0; i < this.edges.length; i++) {
        this.edges[i].increaseThickness(1.1);
        let count = this.edges[i].vertices.length;
        for (let j = 0; j < count; j++) {
          this.edgeVerts[i*count+j] = this.edges[i].vertices[j];
        }
      }
      this.updateBuffers();
      this.edgeThickness *= 1.1;
      console.log('edgeThickness = ' + this.edgeThickness);
      break;
    case 'n':
      this.edgeOpacity = Math.max(0.0002, this.edgeOpacity / 1.1);
      console.log('edgeOpacity = ' + this.edgeOpacity);
      break;
    case 'j':
      this.edgeOpacity *= 1.1;
      console.log('edgeOpacity = ' + this.edgeOpacity);
      break;
    case 't':
      if (this.activeNodeShader == this.nodeShaderProgram) {
        console.log('Switching to thumbnail node shader.');
        if (!this.thumbnails) {
          this.client.fetchThumbnails(this.props.dataset.datasetId)
            .then((result) => {
              this.thumbnails = result.thumbnails;
              this.createTextureAtlas();
              requestAnimationFrame(this.drawScene.bind(this));
            });
        }
        this.activeNodeShader = this.thumbnailShaderProgram;
      } else {
        console.log('Switching to default node shader.');
        this.activeNodeShader = this.nodeShaderProgram;
      }
      break;
    }
    this.resizeCanvas();
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
    gl.blendFuncSeparate(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA,
      gl.ONE, gl.ONE_MINUS_SRC_ALPHA);

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

    this.edgeThickness = defaultEdgeThickness;
    this.edgeSmoothness = defaultEdgeSmoothness;
    this.edgeOpacity = defaultEdgeOpacity;
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
   * TODO: Refactor this routine for maintainability and readablity.
   * @param {array} array2DVertsForNodes
   * @param {array} arrayBeginEndIndicesForEdges
   * @param {number} quadHeight
   * @param {number} quadWidth
   * @param {bool} drawEdgesAsQuads
   */
  createGeometry(array2DVertsForNodes, arrayBeginEndIndicesForEdges,
    quadHeight = 0.1, quadWidth = 0.1, drawEdgesAsQuads = true) {
    this.sampleIndexes = [];
    this.vertColors = [];
    this.bDrawEdgesAsQuads = drawEdgesAsQuads;

    let arrLength = array2DVertsForNodes.length;
    this.nodes = new Array(arrLength);
    this.vertices = new Array(this.nodes.length * 18);
    this.sampleIndexes = new Array(this.nodes.length * 6);
    // create a quad for each position in array2DVertsForNodes
    for (let i = 0; i < arrLength; i++) {
      let quad = new Quad(array2DVertsForNodes[i][0],
        array2DVertsForNodes[i][1],
        quadWidth, quadHeight);
      let vertCount = quad.vertices.length;

      for (let j = 0; j < vertCount; j++) {
        this.vertices[i*vertCount + j] = quad.vertices[j];
      }
      for (let j = 0; j < 6; j++) {
        this.sampleIndexes[i*6 + j] = i;
      }

      this.nodes[i] = quad;
    }

    arrLength = arrayBeginEndIndicesForEdges.length;
    this.edges = new Array(arrLength);
    this.edgeVerts = new Array(this.edges.length * 18);
    // create an Edge for each indicated edge in arrayBeginEndIndicesForEdges
    for (let i = 0; i < arrLength; i++) {
      let index1 = arrayBeginEndIndicesForEdges[i][0];
      let index2 = arrayBeginEndIndicesForEdges[i][1];

      let node1 = this.nodes[index1];
      let node2 = this.nodes[index2];

      let edge = new Edge(node1.X, node1.Y, node2.X, node2.Y,
        defaultEdgeThickness);

      this.edges[i] = edge;
      if (this.bDrawEdgesAsQuads) {
        for (let j = 0; j < 18; j++) {
          this.edgeVerts[i*18+j] = edge.vertices[j];
        }
      } else {
        // push back xy1, uv1, xy2, uv2
        this.edgeVerts[i * 6] = edge.x1;
        this.edgeVerts[i * 6 + 1] = edge.y1;
        this.edgeVerts[i * 6 + 2] = 0.0;
        this.edgeVerts[i * 6 + 3] = edge.x2;
        this.edgeVerts[i * 6 + 4] = edge.y2;
        this.edgeVerts[i * 6 + 5] = 1.0;
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
   * Compile vertex and fragment shader into a shader program.
   * @param {object} gl The OpenGL context.
   * @param {string} vertexShaderSource
   * @param {string} fragmentShaderSource
   * @return {reference}
   */
  createShaderProgram(gl, vertexShaderSource, fragmentShaderSource) {
    let vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShader, vertexShaderSource);
    gl.compileShader(vertexShader);
    let compiled = gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS);
    if (compiled) {
      console.log('Vertex shader compiled successfully');
    } else {
      console.log('Vertex shader failed to compile.');
      let compilationLog = gl.getShaderInfoLog(vertexShader);
      console.log('Shader compiler log: ' + compilationLog);
    }

    let fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShader, fragmentShaderSource);
    gl.compileShader(fragmentShader);

    compiled = gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS);
    if (compiled) {
      console.log('Fragment shader compiled successfully');
    } else {
      console.log('Fragment shader failed to compile.');
      let compilationLog = gl.getShaderInfoLog(fragmentShader);
      console.log('Shader compiler log: ' + compilationLog);
    }

    let shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);
    gl.useProgram(shaderProgram);

    return shaderProgram;
  }

  /**
   * Compiles vertex and fragment shader programs.
   * @param {object} gl The OpenGL context.
   */
  createShaders(gl) {
    this.nodeShaderProgram = this.createShaderProgram(gl,
      NodeVertexShaderSource, NodeFragmentShaderSource);

    this.edgeShaderProgram = this.createShaderProgram(gl,
      EdgeVertexShaderSource, EdgeFragmentShaderSource);

    this.thumbnailShaderProgram = this.createShaderProgram(gl,
      ThumbnailVertexShaderSource, ThumbnailFragmentShaderSource);

    this.activeNodeShader = this.nodeShaderProgram;
  }


  /**
   * Decode a base64 array into a Uint8Array
   * @param {array} base64
   * @return {Uint8Array}
   */
  _base64ToUint8Array(base64) {
    let binaryString = atob(base64);
    let array = new Uint8Array(binaryString.length);
    for (let i = 0; i < binaryString.length; i++) {
      array[i] = binaryString.charCodeAt(i);
    }
    return array;
  }


  /**
   * Creates a texture atlas texture that can be used to render thumbnails
   * to the graph.
   */
  createTextureAtlas() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    // TODO: Refactor to support thumbnails of various/heterogenous sizes.
    let width = this.thumbnails[0].width;
    let height = this.thumbnails[0].height;
    const MAX_TEXTURE_SIZE = 2048;
    const THUMBNAIL_WIDTH = 80;
    const THUMBNAIL_HEIGHT = 40;
    let thumbnailsPerTextureRow =
      Math.floor(MAX_TEXTURE_SIZE / THUMBNAIL_WIDTH);
    let atlasBuffer = new Uint8Array(4*MAX_TEXTURE_SIZE*MAX_TEXTURE_SIZE);

    for (let i=0; i < this.thumbnails.length; i++) {
      let data = this._base64ToUint8Array(this.thumbnails[i].data);

      // copy the thumbnail into the atlas.
      let atlasOffsetY = Math.floor(i / thumbnailsPerTextureRow);
      let atlasOffsetX = i % thumbnailsPerTextureRow;
      let y = (atlasOffsetY * THUMBNAIL_HEIGHT);
      let x = (atlasOffsetX * THUMBNAIL_WIDTH);

      for (let h=0; h < height; h++) {
        for (let w=0; w < width; w++) {
          let sourceIndex = (width*h + w);
          let targetIndex = ((y+h)*MAX_TEXTURE_SIZE) + x + w;
          atlasBuffer[4*targetIndex + 0] = data[4*sourceIndex + 0];
          atlasBuffer[4*targetIndex + 1] = data[4*sourceIndex + 1];
          atlasBuffer[4*targetIndex + 2] = data[4*sourceIndex + 2];
          atlasBuffer[4*targetIndex + 3] = data[4*sourceIndex + 3];
        }
      }
    }

    let texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE,
      0, gl.RGBA, gl.UNSIGNED_BYTE, atlasBuffer);
    gl.bindTexture(gl.TEXTURE_2D, null);
    this.thumbnailsAtlasTexture = texture;
  }


  /**
   * Creates vertex buffers for dummy data.
   * @param {object} gl The OpenGL context.
   */
  createBuffers(gl) {
    // Quad buffer
    this.vertex_buffer = gl.createBuffer();
    this.vertex_array = new Float32Array(this.vertices);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.vertex_array, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    webGLErrorCheck(gl);

    // vertex Color buffer
    this.vertColor_buffer = gl.createBuffer();
    this.vertColor_array = new Float32Array(this.vertColors);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertColor_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.vertColor_array, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    webGLErrorCheck(gl);

    // sample index buffer
    this.sampleIndex_buffer = gl.createBuffer();
    this.sampleIndex_array = new Uint16Array(this.sampleIndexes);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.sampleIndex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.sampleIndex_array, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    webGLErrorCheck(gl);

    // Edge buffer
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

    this.sampleIndex_array = new Uint16Array(this.sampleIndexes);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.sampleIndex_buffer);
    gl.bufferData(gl.ARRAY_BUFFER, this.sampleIndex_array, gl.STATIC_DRAW);
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
    window.removeEventListener('keydown', this.handleKeyDown);
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
    window.addEventListener('keydown', this.handleKeyDown);
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
    gl.useProgram(this.activeNodeShader);

    if (this.thumbnailsAtlasTexture) {
      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, this.thumbnailsAtlasTexture);
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);
    let coordinateAttrib =
        gl.getAttribLocation(this.activeNodeShader, 'coordinates');
    gl.vertexAttribPointer(coordinateAttrib, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coordinateAttrib);

    if (this.vertColors && this.vertColors.length == this.vertices.length) {
      gl.bindBuffer(gl.ARRAY_BUFFER, this.vertColor_buffer);
      let vertexColorAttribute =
        gl.getAttribLocation(this.activeNodeShader, 'vertexColor');
      gl.vertexAttribPointer(vertexColorAttribute,
        3, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(vertexColorAttribute);
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, this.sampleIndex_buffer);
    let sampleIndexAttribute =
      gl.getAttribLocation(this.activeNodeShader, 'sampleIndex');
    if (sampleIndexAttribute > 0) {
      gl.vertexAttribPointer(
        sampleIndexAttribute, 1, gl.UNSIGNED_SHORT, false, 0, 0);
      gl.enableVertexAttribArray(sampleIndexAttribute);
    }

    let nodeOutlineLocation =
        gl.getUniformLocation(this.activeNodeShader, 'nodeOutline');
    gl.uniform1f(nodeOutlineLocation, this.nodeOutline);

    let nodeSmoothnessLocation =
        gl.getUniformLocation(this.activeNodeShader, 'nodeSmoothness');
    gl.uniform1f(nodeSmoothnessLocation, this.nodeSmoothness);

    let projectionMatrixLocation =
        gl.getUniformLocation(this.activeNodeShader, 'uProjectionMatrix');
    gl.uniformMatrix4fv(projectionMatrixLocation, false, this.projectionMatrix);

    gl.drawArrays(gl.TRIANGLES, 0, this.vertices.length / 3);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);

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
