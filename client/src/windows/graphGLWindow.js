import * as d3 from 'd3';
import { Edge, Quad } from './primitives';
import { createShaderProgram, webGLErrorCheck } from './glUtils';
import EdgeFragmentShaderSource from '../shaders/edge.frag';
import EdgeVertexShaderSource from '../shaders/edge.vert';
import ErrorDialog from '../errorDialog.js';
import GLWindow from './glWindow.js';
import NodeFragmentShaderSource from '../shaders/node.frag';
import NodeVertexShaderSource from '../shaders/node.vert';
import Paper from '@material-ui/core/Paper';
import PickingFragmentShaderSource from '../shaders/picking.frag';
import PickingVertexShaderSource from '../shaders/picking.vert';
import PreviewTextureFragmentShaderSource from '../shaders/previewTexture.frag';
import PreviewTextureVertexShaderSource from '../shaders/previewTexture.vert';
import React from 'react';
import ThumbnailFragmentShaderSource from '../shaders/thumbnail.frag';
import ThumbnailVertexShaderSource from '../shaders/thumbnail.vert';
import Typography from '@material-ui/core/Typography';
import { mat4 } from 'gl-matrix';
import { withDSXContext } from '../dsxContext.js';

const zoomRate = 1.2;
const maxScale = 10;

const defaultEdgeThickness = 0.075 / 50;
const defaultEdgeSmoothness = 0.2;
const defaultEdgeOpacity = 0.05;

/**
 * A WebGL Window Component for rendering Graphs.
 */
class GraphGLWindow extends GLWindow {
  /**
   * GraphWebGLWindow constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.state = {
      hoverX: null,
      hoverY: null,
      hoverNode: null,
      hoverShow: false,
    };

    this.client = this.props.dsxContext.client;
    this.canvas = null;
    this.vertex_array = null;
    this.vertex_buffer = null;
    this.vertColor_array = null;
    this.vertColor_buffer = null;
    this.sampleIndex_array = null;
    this.sampleIndex_buffer = null;
    this.nodeShaderProgram = null;
    this.edgeShaderProgram = null;
    this.pickingShaderProgram = null;
    this.thumbnailHeight = 1;
    this.thumbnailWidth = 1;
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

    this.scale = 1;
    this.xOffset = 0;
    this.yOffset = 0;

    this.previousX = 0;
    this.previousY = 0;
    this.rightMouseDown = false;

    this.renderGL = this.renderGL.bind(this);
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

    this.currentHoverNode = null;
    this.currentHoverX = 0;
    this.currentHoverY = 0;

    this.showPreview = false;
    this.previewTextureShader = null;
    this.thumbnails = null;
    this.thumbnailsAtlasTexture = null;
    this.frameBuffer = null;
    this.renderBuffer = null;
    this.renderTexture = null;
  }

  /**
   * Event Handling for scroll wheel; zoom in and out of graph
   * @param {Event} event
   */
  handleScrollEvent(event) {
    if (event.deltaY < 0 && this.scale > -maxScale) {
      this.scale = this.scale / zoomRate;
    }
    if (event.deltaY > 0 && this.scale < maxScale) {
      this.scale = this.scale * zoomRate;
    }
    this.setupOrtho();
    requestAnimationFrame(this.renderGL);
  }

  /**
   * Event handling for mouse click down
   * @param {Event} event
   */
  handleMouseDown(event) {
    // Handle Right click
    if (event.button == 2) {
      this.rightMouseDown = true;
      this.previousX = event.offsetX;
      this.previousY = event.offsetY;
    }
  }

  /**
   * Event handling for mouse click up
   * @param {Event} event
   */
  handleMouseRelease(event) {
    // Handle left click release
    if (event.button == 0) {
      let x = event.offsetX;
      let y = event.offsetY;
      let id = this.getDesignUnderCursor(x, y);
      this.props.onDesignSelection(event, id);
    }

    // Handle right click release
    if (event.button == 2) {
      this.rightMouseDown = false;
    }
  }

  /**
   * Event handling for mouse movement
   * @param {Event} event
   */
  handleMouseMove(event) {
    let x = event.offsetX;
    let y = event.offsetY;

    if (this.rightMouseDown) {
      let dx = (x - this.previousX);
      let dy = (y - this.previousY);

      this.xOffset -= this.pixelToGeometryRatioX * dx;
      this.yOffset += this.pixelToGeometryRatioY * dy;

      this.setupOrtho();
      requestAnimationFrame(this.renderGL);
    }

    this.previousX = x;
    this.previousY = y;
    this.setHoverUnderCursor(x, y);
  }

  /**
   * Looks up the texture value at the coordinate  to determine
   * what geometry is currently under the cursor and sets the state for hover.
   * @param {number} x
   * @param {number} y
   */
  setHoverUnderCursor(x, y) {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    // Read one pixel
    let readout = new Uint8Array(4);
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffer);
    gl.readPixels(x, canvas.height-y, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, readout);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    let range = 255*255;
    let n = 1000;

    let r = readout[0];
    let g = readout[1];
    let b = readout[2];

    if (r != 255) {
      let mappedIndex = 255*g + b;
      let baseIndex = Math.floor(mappedIndex / Math.floor(range/n));
      this.setState({
        hoverNode: baseIndex,
        hoverX: x,
        hoverY: y,
        hoverShow: baseIndex == this.state.hoverNode,
      });
    } else {
      this.setState({
        hoverNode: null,
        hoverShow: false,
      });
    }
  }

  /**
   * Looks up the texture value at the coordinate to determine
   * what design is currently under the cursor for left click to select.
   * @param {number} x
   * @param {number} y
   * @return {number} index of design
   */
  getDesignUnderCursor(x, y) {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    // Read one pixel
    let readout = new Uint8Array(4);
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffer);
    gl.readPixels(x, canvas.height-y, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, readout);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    let range = 255*255;
    let n = 1000;
    let r = readout[0];
    let g = readout[1];
    let b = readout[2];

    let baseIndex = null;
    if (r != 255) {
      let mappedIndex = 255*g + b;
      baseIndex = Math.floor(mappedIndex / Math.floor(range/n));
    }
    return baseIndex;
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
      case 'p':
        this.showPreview = !this.showPreview;
        break;
      case 't':
        if (this.activeNodeShader == this.nodeShaderProgram) {
          console.log('Switching to thumbnail node shader.');
          if (!this.thumbnails) {
            this.client.fetchThumbnails(this.props.dataset.datasetId)
              .then((result) => {
                this.thumbnails = result.thumbnails;
                this.createTextureAtlas();
                requestAnimationFrame(this.renderGL);
              });
          }
          this.activeNodeShader = this.thumbnailShaderProgram;
        } else {
          console.log('Switching to default node shader.');
          for (let i = 0; i < this.nodes.length; i++) {
            this.nodes[i].resetRadius();
            let count = this.nodes[i].vertices.length;
            for (let j = 0; j < count; j++) {
              this.vertices[i*count+j] = this.nodes[i].vertices[j];
            }
          }
          this.activeNodeShader = this.nodeShaderProgram;
        }
        this.updateBuffers();
        break;
    }
    requestAnimationFrame(this.renderGL);
  }

  /**
   * Initial setup for the webgl canvas.
   */
  initGL() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    // Clear the canvas
    gl.clearColor(1.0, 1.0, 1.0, 1.0);
    gl.clear(gl.DEPTH_BUFFER_BIT | gl.COLOR_BUFFER_BIT);

    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    gl.depthMask(false);
    gl.enable(gl.BLEND);
    gl.blendFuncSeparate(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA,
      gl.ONE, gl.ONE_MINUS_SRC_ALPHA);

    this.createShaders(gl);
    this.createBuffers(gl);
    this.createSelectionFrameBuffer(gl);

    webGLErrorCheck(gl);

    this.edgeThickness = defaultEdgeThickness;
    this.edgeSmoothness = defaultEdgeSmoothness;
    this.edgeOpacity = defaultEdgeOpacity;
  }

  /**
   * Set up an orthographic projection.
   * This is used to position the graph correctly in space.
   */
  setupOrtho() {
    const canvas = this.refs.canvas;
    let width = canvas.clientWidth;
    let height = canvas.clientHeight;
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
   * Creates the geometry to be rendered.
   * TODO: Refactor this routine for maintainability and readablity.
   * @param {array} array2DVertsForNodes
   * @param {array} arrayBeginEndIndicesForEdges
   * @param {number} quadHeight
   * @param {number} quadWidth
   * @param {bool} drawEdgesAsQuads
   */
  createGeometry(array2DVertsForNodes, arrayBeginEndIndicesForEdges, quadHeight = 0.1, quadWidth = 0.1) {
    this.sampleIndexes = [];
    this.vertColors = [];
    // this.bDrawEdgesAsQuads = true;

    // Graph Vertex Geometry
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

    // Graph Edge Geometry
    arrLength = arrayBeginEndIndicesForEdges.length;
    this.edges = new Array(arrLength);
    this.edgeVerts = new Array(this.edges.length * 18);
    // create an Edge for each indicated edge in arrayBeginEndIndicesForEdges
    for (let i = 0; i < arrLength; i++) {
      let index1 = arrayBeginEndIndicesForEdges[i][0];
      let index2 = arrayBeginEndIndicesForEdges[i][1];

      let node1 = this.nodes[index1];
      let node2 = this.nodes[index2];

      let edge = new Edge(node1.X, node1.Y, node2.X, node2.Y, defaultEdgeThickness);

      this.edges[i] = edge;
      for (let j = 0; j < 18; j++) {
        this.edgeVerts[i*18+j] = edge.vertices[j];
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
    this.nodeShaderProgram = createShaderProgram(gl, NodeVertexShaderSource, NodeFragmentShaderSource);

    this.edgeShaderProgram = createShaderProgram(gl, EdgeVertexShaderSource, EdgeFragmentShaderSource);

    this.thumbnailShaderProgram = createShaderProgram(gl, ThumbnailVertexShaderSource, ThumbnailFragmentShaderSource);

    this.pickingShaderProgram = createShaderProgram(gl, PickingVertexShaderSource, PickingFragmentShaderSource);

    this.previewTextureShader = createShaderProgram(gl,
      PreviewTextureVertexShaderSource, PreviewTextureFragmentShaderSource);

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

    // TODO: Refactor to support thumbnails of various/heterogeneous sizes.
    this.thumbnailWidth = this.thumbnails[0].width;
    this.thumbnailHeight = this.thumbnails[0].height;
    const MAX_TEXTURE_SIZE = 2048;
    let thumbnailsPerTextureRow = Math.floor(MAX_TEXTURE_SIZE / this.thumbnailWidth);
    let atlasBuffer = new Uint8Array(4*MAX_TEXTURE_SIZE*MAX_TEXTURE_SIZE);

    for (let i=0; i < this.thumbnails.length; i++) {
      let data = this._base64ToUint8Array(this.thumbnails[i].data);

      // copy the thumbnail into the atlas.
      let atlasOffsetY = Math.floor(i / thumbnailsPerTextureRow);
      let atlasOffsetX = i % thumbnailsPerTextureRow;
      let y = (atlasOffsetY * this.thumbnailHeight);
      let x = (atlasOffsetX * this.thumbnailWidth);

      for (let h=0; h < this.thumbnailHeight; h++) {
        for (let w=0; w < this.thumbnailWidth; w++) {
          let sourceIndex = (this.thumbnailWidth*h + w);
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
   * Creates render frame buffer and associated texture.
   * The picking shader program is drawn to this framebuffer and
   * used when hovering over or selecting designs.
   * @param {object} gl The OpenGL context.
   */
  createSelectionFrameBuffer(gl) {
    // Create FrameBuffer
    let canvas = this.refs.canvas;
    this.frameBuffer = gl.createFramebuffer();
    this.frameBuffer.width = canvas.clientWidth;
    this.frameBuffer.height = canvas.clientHeight;
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffer);

    // TODO better comment than -> Create Texture - empty by default, this is the target of a render.
    this.renderTexture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, this.renderTexture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.frameBuffer.width,
      this.frameBuffer.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

    console.log('Created RenderTexture of size ',
      this.frameBuffer.width, this.frameBuffer.height);

    this.renderBuffer = gl.createRenderbuffer();
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderBuffer);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16,
      this.frameBuffer.width, this.frameBuffer.height);

    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0,
      gl.TEXTURE_2D, this.renderTexture, 0);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT,
      gl.RENDERBUFFER, this.renderBuffer);

    let status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
    switch (status) {
      case gl.FRAMEBUFFER_COMPLETE:
        console.log('The framebuffer is ready to display.');
        break;
      case gl.FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        console.log('The attachment types are mismatched or not all ' +
          'framebuffer attachment points are framebuffer attachment complete.');
        break;
      case gl.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        console.log('There is no attachment.');
        break;
      case gl.FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        console.log('Height and width of the attachment are not the same.');
        break;
      case gl.FRAMEBUFFER_UNSUPPORTED:
        console.log('The format of the attachment is not supported or if ' +
          'depth and stencil attachments are not the same renderbuffer.');
        break;
      default:
        console.log('FramebufferStatus reported unknown status: ' + status);
        break;
    }

    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }

  /**
   * Recreates render frame buffer and associated texture with new sizes.
   * @param {object} gl The OpenGL context.
   */
  resizeSelectionFrameBuffer(gl) {
    // TODO: Tear down existing frame buffer and create new one with current
    //       size. Otherwise, node picking may not work after window resize.
  }

  /**
   * Creates vertex buffers for geometry data.
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

    // preview box buffer
    // TODO: Move logic into new function
    this.previewBoxVertexBuffer = gl.createBuffer();
    let dataArray = new Float32Array([
      0, 0, 0, // v1
      1, 0, 0, // v2
      1, 1, 0, // v3
      0, 1, 0, // v4
    ]);
    gl.bindBuffer(gl.ARRAY_BUFFER, this.previewBoxVertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, dataArray, gl.STATIC_DRAW);
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
    // Resize the webgl render resolution to match browser canvas size.
    let canvas = this.refs.canvas;
    canvas.width = canvas.clientWidth;
    canvas.height = canvas.clientHeight;

    this.resizeSelectionFrameBuffer();
    this.setupOrtho();
    requestAnimationFrame(this.renderGL);
  }

  /**
   * Callback invoked before the React Component is rendered.
   */
  componentWillMount() {
    const { selectedDesigns, activeDesigns } = this.props;
    if (!this.props.decomposition) {
      return;
    }
    let { datasetId, k, persistenceLevel } = this.props.decomposition;
    this.client
      .fetchLayoutForPersistenceLevel(datasetId, k, persistenceLevel)
      .then(function(result) {
        if (result.embedding && result.embedding.layout) {
          let layout = result.embedding.layout;
          let adjacency = result.embedding.adjacency;
          this.createGeometry(layout, adjacency, 0.02, 0.02);

          if (this.props.qoi) {
            let min = Math.min(...this.props.qoi);
            let max = Math.max(...this.props.qoi);
            let color = d3.scaleLinear()
              .domain([min, 0.5*(min+max), max])
              .range(['white', '#b53f51']);
            let colorsArray = [];
            for (let i = 0; i < this.props.qoi.length; i++) {
              if (selectedDesigns.has(i)) {
                colorsArray.push((63/255), (81/255), (181/255));
              } else if (activeDesigns.has(i)) {
                let colorString = color(this.props.qoi[i]);
                let colorTriplet = colorString.match(/([0-9]+\.?[0-9]*)/g);
                colorTriplet[0] /= 255;
                colorTriplet[1] /= 255;
                colorTriplet[2] /= 255;
                colorsArray.push(...colorTriplet);
              } else {
                colorsArray.push((211/255), (211/255), (211/255));
              }
            }
            this.addVertexColors(colorsArray);
          } else {
            let colorsArray = [];
            for (let i=0; i < layout.length; i++) {
              if (selectedDesigns.has(i)) {
                colorsArray.push((63/255), (81/255), (181/255));
              } else if (activeDesigns.has(i)) {
                colorsArray.push(1.0, 1.0, 1.0);
              } else {
                colorsArray.push((211/255), (211/255), (211/255));
              }
            }
            this.addVertexColors(colorsArray);
          }
        } else {
          if (this.props.decomposition) {
            let errorMessage = 'No decomposition layout provided.';
            this.refs.errorDialog.reportError(errorMessage);
          } else {
            let errorMessage = 'No decomposition provided.';
            this.refs.errorDialog.reportError(errorMessage);
          }
        }
        this.updateBuffers();
        requestAnimationFrame(this.renderGL);
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
    this.refs.canvas.addEventListener(
      'wheel', this.handleScrollEvent, { passive:true });
    this.refs.canvas.addEventListener(
      'mousedown', this.handleMouseDown, { passive:true });
    this.refs.canvas.addEventListener(
      'mouseup', this.handleMouseRelease, { passive:true });
    this.refs.canvas.addEventListener(
      'mousemove', this.handleMouseMove, { passive:true });
    this.refs.canvas.addEventListener(
      'contextmenu', (e) => e.preventDefault(), false);
    window.addEventListener('keydown', this.handleKeyDown);
    requestAnimationFrame(this.renderGL);
  }

  /**
   * React to new props. Reset views if dataset changes.
   * @param {object} nextProps
   */
  componentWillReceiveProps(nextProps) {
    // Make sure canvas is re-sized so hover and selection work
    this.resizeCanvas();

    // TODO:  Simplify and remove code duplication.
    if (!nextProps.decomposition) {
      return;
    }
    const { selectedDesigns, activeDesigns } = nextProps;
    const { datasetId, k, persistenceLevel } = nextProps.decomposition;
    const qoiName = nextProps.decomposition.decompositionField;

    if (this.props.decomposition &&
        datasetId === this.props.decomposition.datasetId &&
        k === this.props.decomposition.k &&
        persistenceLevel === this.props.decomposition.persistenceLevel &&
        qoiName === this.props.decomposition.decompositionField &&
        selectedDesigns === this.props.selectedDesigns) {
      return;
    } else if (this.props.decomposition &&
        datasetId === this.props.decomposition.datasetId &&
        k === this.props.decomposition.k &&
        persistenceLevel === this.props.decomposition.persistenceLevel &&
        qoiName === this.props.decomposition.decompositionField &&
        (selectedDesigns !== this.props.selectedDesigns ||
        activeDesigns !== this.props.activeDesigns)) {
      if (this.layout && this.adjacency) {
        this.createGeometry(this.layout, this.adjacency, 0.02, 0.02);
        if (this.qoi) {
          let min = Math.min(...this.qoi);
          let max = Math.max(...this.qoi);
          let color = d3.scaleLinear()
            .domain([min, 0.5*(min+max), max])
            .range(['white', '#b53f51']);
          let colorsArray = [];
          for (let i = 0; i < this.qoi.length; i++) {
            if (selectedDesigns.has(i)) {
              colorsArray.push((63/255), (81/255), (181/255));
            } else if (activeDesigns.has(i)) {
              let colorString = color(this.qoi[i]);
              let colorTriplet = colorString.match(/([0-9]+\.?[0-9]*)/g);
              colorTriplet[0] /= 255;
              colorTriplet[1] /= 255;
              colorTriplet[2] /= 255;
              colorsArray.push(...colorTriplet);
            } else {
              colorsArray.push((211/255), (211/255), (211/255));
            }
          }
          this.addVertexColors(colorsArray);
        } else {
          let colorsArray = [];
          for (let i=0; i < this.layout.length; i++) {
            if (selectedDesigns.has(i)) {
              colorsArray.push((63/255), (81/255), (181/255));
            } else if (activeDesigns.has(i)) {
              colorsArray.push(1.0, 1.0, 1.0);
            } else {
              colorsArray.push((211/255), (211/255), (211/255));
            }
          }
          this.addVertexColors(colorsArray);
        }
      }
    } else {
      Promise.all([
        this.client.fetchLayoutForPersistenceLevel(
          datasetId, k, persistenceLevel),
        this.client.fetchQoi(datasetId, qoiName),
      ]).then((results) => {
        const [result, qoiResult] = results;
        this.qoi = qoiResult.qoi;
        if (result.embedding && result.embedding.layout) {
          // let layout = [].concat(...result.embedding.layout);
          this.layout = result.embedding.layout;
          this.adjacency = result.embedding.adjacency;
          this.createGeometry(this.layout, this.adjacency, 0.02, 0.02);

          if (this.qoi) {
            let min = Math.min(...this.qoi);
            let max = Math.max(...this.qoi);
            let color = d3.scaleLinear()
              .domain([min, 0.5*(min+max), max])
              .range(['white', '#b53f51']);
            let colorsArray = [];
            for (let i = 0; i < this.qoi.length; i++) {
              if (selectedDesigns.has(i)) {
                colorsArray.push((63/255), (81/255), (181/255));
              } else if (activeDesigns.has(i)) {
                let colorString = color(this.qoi[i]);
                let colorTriplet = colorString.match(/([0-9]+\.?[0-9]*)/g);
                colorTriplet[0] /= 255;
                colorTriplet[1] /= 255;
                colorTriplet[2] /= 255;
                colorsArray.push(...colorTriplet);
              } else {
                colorsArray.push((211/255), (211/255), (211/255));
              }
            }
            this.addVertexColors(colorsArray);
          } else {
            let colorsArray = [];
            for (let i=0; i < this.layout.length; i++) {
              if (selectedDesigns.has(i)) {
                colorsArray.push((63/255), (81/255), (181/255));
              } else if (activeDesigns.has(i)) {
                colorsArray.push(1.0, 1.0, 1.0);
              } else {
                colorsArray.push((211/255), (211/255), (211/255));
              }
            }
            this.addVertexColors(colorsArray);
          }
        } else {
          if (nextProps.decomposition) {
            let errorMessage = 'No decomposition layout provided.';
            this.refs.errorDialog.reportError(errorMessage);
          } else {
            let errorMessage = 'No decomposition provided.';
            this.refs.errorDialog.reportError(errorMessage);
          }
        }
        this.updateBuffers();
        requestAnimationFrame(this.renderGL);
      });
    }
    this.updateBuffers();
    requestAnimationFrame(this.renderGL);
  }

  /**
   * React lifecycle method run after componenet updates
   * @param {object} prevProps previous props
   * @param {object} prevState previous state
   * @param {object} prevContext previous context
   */
  componentDidUpdate(prevProps, prevState, prevContext) {
    if (this.props.decomposition && !prevProps.decomposition) {
      if (!this.thumbnails) {
        this.client.fetchThumbnails(this.props.dataset.datasetId)
          .then((result) => {
            this.thumbnails = result.thumbnails;
            this.createTextureAtlas();
            requestAnimationFrame(this.renderGL);
          });
      }
    }

    if (this.props.numberOfWindows !== prevProps.numberOfWindows) {
      this.resizeCanvas();
    }

    if (!this.setsAreEqual(this.props.activeDesigns, prevProps.activeDesigns)) {
      this.renderGL();
    }
  }

  /**
   * Compares two array for equality
   * @param {array} first
   * @param {array} second
   * @return {boolean}
   */
  setsAreEqual(first, second) {
    if (first.size !== second.size) {
      return false;
    }

    first.forEach( (i) => {
      if (!second.has(i)) {
        return false;
      }
    });
    return true;
  }

  /**
   * Draw PreviewBox. Useful for debugging and
   * @param {object} gl
   */
  drawPreviewBox(gl) {
    let shader = this.previewTextureShader;

    gl.useProgram(shader);
    gl.bindTexture(gl.TEXTURE_2D, this.renderTexture);

    gl.bindBuffer(gl.ARRAY_BUFFER, this.previewBoxVertexBuffer);
    let vertsAttrib = gl.getAttribLocation(shader, 'verts');
    gl.vertexAttribPointer(vertsAttrib, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(vertsAttrib);

    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
  }


  /**
   * Render Graph Nodes
   * @param {object} gl
   * @param {object} shader
   */
  drawNodes(gl, shader) {
    gl.useProgram(shader);
    if (this.thumbnailsAtlasTexture) {
      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, this.thumbnailsAtlasTexture);
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, this.vertex_buffer);
    let coordinateAttrib = gl.getAttribLocation(shader, 'coordinates');
    gl.vertexAttribPointer(coordinateAttrib, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(coordinateAttrib);

    if (this.vertColors && this.vertColors.length == this.vertices.length) {
      gl.bindBuffer(gl.ARRAY_BUFFER, this.vertColor_buffer);
      let vertexColorAttribute = gl.getAttribLocation(shader, 'vertexColor');
      if (vertexColorAttribute > 0) {
        gl.vertexAttribPointer(vertexColorAttribute, 3, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(vertexColorAttribute);
      }
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, this.sampleIndex_buffer);
    let sampleIndexAttribute = gl.getAttribLocation(shader, 'sampleIndex');
    if (sampleIndexAttribute > 0) {
      gl.vertexAttribPointer(sampleIndexAttribute, 1, gl.UNSIGNED_SHORT, false, 0, 0);
      gl.enableVertexAttribArray(sampleIndexAttribute);
    }

    let nodeOutlineLocation = gl.getUniformLocation(shader, 'nodeOutline');
    gl.uniform1f(nodeOutlineLocation, this.nodeOutline);

    let nodeSmoothnessLocation = gl.getUniformLocation(shader, 'nodeSmoothness');
    gl.uniform1f(nodeSmoothnessLocation, this.nodeSmoothness);

    let projectionMatrixLocation = gl.getUniformLocation(shader, 'uProjectionMatrix');
    gl.uniformMatrix4fv(projectionMatrixLocation, false, this.projectionMatrix);

    let thumbnailWidthLocation = gl.getUniformLocation(shader, 'thumbnailWidth');
    gl.uniform1f(thumbnailWidthLocation, this.thumbnailWidth);

    let thumbnailHeightLocation = gl.getUniformLocation(shader, 'thumbnailHeight');
    gl.uniform1f(thumbnailHeightLocation, this.thumbnailHeight);

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

    gl.drawArrays(gl.TRIANGLES, 0, this.edgeVerts.length / 3);

    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    webGLErrorCheck(gl);
  }

  /**
   * Renders the OpenGL Content to the canvas.
   */
  renderGL() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    // draw scene offscreen for picking (selection)
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffer);
    gl.clear(gl.DEPTH_BUFFER_BIT | gl.COLOR_BUFFER_BIT);
    gl.viewport(0, 0, canvas.clientWidth, canvas.clientHeight);
    if (this.vertices) {
      this.drawNodes(gl, this.pickingShaderProgram);
    }

    // draw scene to screen
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.clear(gl.DEPTH_BUFFER_BIT | gl.COLOR_BUFFER_BIT);
    // gl.viewport(0, 0, canvas.width, canvas.height);
    this.drawScene(gl);

    if (this.showPreview) {
      this.drawPreviewBox(gl);
    }
  }

  /**
   * Draw the graph to be seen by the user.
   * @param {object} gl
   */
  drawScene(gl) {
    const canvas = this.refs.canvas;
    gl.clear(gl.DEPTH_BUFFER_BIT | gl.COLOR_BUFFER_BIT);
    gl.viewport(0, 0, canvas.width, canvas.height);

    // TODO: Replace with a safer check. Maybe add boolean to class.
    if (this.vertices) {
      this.drawEdges(gl);
      this.drawNodes(gl, this.activeNodeShader);
    }
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    let paperStyle = {
      position: 'relative',
      border: '1px solid gray',
      flexBasis: '50%',
    };

    let canvasStyle = {
      width: '100%',
      height: '100%',
      boxSizing: 'border-box',
      position: 'absolute',
    };

    let imageBase64 = null;
    let qoi = null;
    if (this.state.hoverNode && this.qoi) {
      qoi = (this.qoi[this.state.hoverNode]).toExponential(5);
    }

    if (this.state.hoverNode && this.thumbnails) {
      imageBase64 = this.thumbnails[this.state.hoverNode].rawData;
    }

    return (
      <Paper style={paperStyle}>
        <canvas ref='canvas' className='glCanvas' style={canvasStyle} />
        {
          this.state.hoverNode ? (<Paper style={{
            position: 'absolute',
            width: '120px',
            padding: '8px',
            display: 'flex',
            flexDirection: 'column',
            top: (this.state.hoverY - 10) + 'px',
            left: (this.state.hoverX + 10) + 'px',
            opacity: '0.75',
          }}>
            <Typography>
              { 'Sample: ' + (this.state.hoverNode + 1) }
            </Typography>
            { qoi ? <Typography> { 'Qoi: ' + qoi } </Typography> : [] }
            { imageBase64 ?
              <img src={'data:image/png;base64, ' + imageBase64}
                style = {{
                  display: 'block',
                  borderColor: '#ddd',
                  borderSize: '1px',
                  borderStyle: 'solid',
                  maxWidth: '115px',
                  width: 'auto',
                  height: 'auto',
                }} /> :
              []
            }
          </Paper>) :
            []
        }
        <ErrorDialog ref='errorDialog' />
      </Paper>
    );
  }
}

export default withDSXContext(GraphGLWindow);
