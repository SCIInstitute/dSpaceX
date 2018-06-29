﻿import * as d3 from 'd3';
import { Edge, Quad } from './primitives';
import { createShaderProgram, webGLErrorCheck } from './glUtils';
import EdgeFragmentShaderSource from '../shaders/edge.frag';
import EdgeVertexShaderSource from '../shaders/edge.vert';
import ErrorDialog from '../errorDialog.js';
import GLWindow from './glWindow.js';
import NodeFragmentShaderSource from '../shaders/node.frag';
import NodeVertexShaderSource from '../shaders/node.vert';
import Paper from 'material-ui/Paper';
import PickingFragmentShaderSource from '../shaders/picking.frag';
import PickingVertexShaderSource from '../shaders/picking.vert';
import PreviewTexturVertexeShaderSource from '../shaders/previewTexture.vert';
import PreviewTextureFragmentShaderSource from '../shaders/previewTexture.frag';
import React from 'react';
import ThumbnailFragmentShaderSource from '../shaders/thumbnail.frag';
import ThumbnailVertexShaderSource from '../shaders/thumbnail.vert';
import { mat4 } from 'gl-matrix';

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
    this.pickingShaderProgram = null;
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
   * Event Handling for scroll wheel
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
    this.pickGeometryUnderCursor(x, y);
  }

  /**
   * Looks up the texture value at the coordinate cooordinate to determine
   * what geometry is currently under the cursor.
   * @param {number} x
   * @param {number} y
   */
  pickGeometryUnderCursor(x, y) {
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
    console.log('Node index: ' + this.state.hoverNode);
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
          this.activeNodeShader = this.nodeShaderProgram;
        }
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
    this.createFrameBuffers(gl);

    webGLErrorCheck(gl);

    this.edgeThickness = defaultEdgeThickness;
    this.edgeSmoothness = defaultEdgeSmoothness;
    this.edgeOpacity = defaultEdgeOpacity;
  }

  /**
   * Set up an orthographic projection.
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
   * Compiles vertex and fragment shader programs.
   * @param {object} gl The OpenGL context.
   */
  createShaders(gl) {
    this.nodeShaderProgram = createShaderProgram(gl,
      NodeVertexShaderSource, NodeFragmentShaderSource);

    this.edgeShaderProgram = createShaderProgram(gl,
      EdgeVertexShaderSource, EdgeFragmentShaderSource);

    this.thumbnailShaderProgram = createShaderProgram(gl,
      ThumbnailVertexShaderSource, ThumbnailFragmentShaderSource);

    this.pickingShaderProgram = createShaderProgram(gl,
      PickingVertexShaderSource, PickingFragmentShaderSource);

    this.previewTextureShader = createShaderProgram(gl,
      PreviewTexturVertexeShaderSource, PreviewTextureFragmentShaderSource);

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
   * Creates render frame buffer and associated texture.
   * @param {object} gl The OpenGL context.
   */
  createFrameBuffers(gl) {
    let canvas = this.refs.canvas;
    this.frameBuffer = gl.createFramebuffer();
    this.frameBuffer.width = canvas.clientWidth;
    this.frameBuffer.height = canvas.clientHeight;
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffer);

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
  resizeFrameBuffers(gl) {
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

    this.resizeFrameBuffers();
    this.setupOrtho();
    requestAnimationFrame(this.renderGL);
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
    this.refs.canvas.addEventListener('wheel', this.handleScrollEvent);
    this.refs.canvas.addEventListener('mousedown', this.handleMouseDown);
    this.refs.canvas.addEventListener('mouseup', this.handleMouseRelease);
    this.refs.canvas.addEventListener('mousemove', this.handleMouseMove);
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
      }.bind(this));
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
      let vertexColorAttribute =
        gl.getAttribLocation(shader, 'vertexColor');
      if (vertexColorAttribute > 0) {
        gl.vertexAttribPointer(vertexColorAttribute,
          3, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(vertexColorAttribute);
      }
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, this.sampleIndex_buffer);
    let sampleIndexAttribute =
      gl.getAttribLocation(shader, 'sampleIndex');
    if (sampleIndexAttribute > 0) {
      gl.vertexAttribPointer(
        sampleIndexAttribute, 1, gl.UNSIGNED_SHORT, false, 0, 0);
      gl.enableVertexAttribArray(sampleIndexAttribute);
    }

    let nodeOutlineLocation =
        gl.getUniformLocation(shader, 'nodeOutline');
    gl.uniform1f(nodeOutlineLocation, this.nodeOutline);

    let nodeSmoothnessLocation =
        gl.getUniformLocation(shader, 'nodeSmoothness');
    gl.uniform1f(nodeSmoothnessLocation, this.nodeSmoothness);

    let projectionMatrixLocation =
        gl.getUniformLocation(shader, 'uProjectionMatrix');
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
  renderGL() {
    const canvas = this.refs.canvas;
    let gl = canvas.getContext('webgl');

    // draw scene offscreen for picking
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
    let style = {
      width: '100%',
      height: '100%',
      borderRight: '1px dashed gray',
      boxSizing: 'border-box',
    };

    let imageBase64 = null;
    if (this.thumbnails && this.state.hoverNode) {
      imageBase64 = this.thumbnails[this.state.hoverNode].rawData;
    }

    return (
      <React.Fragment>
        <canvas ref='canvas' className='glCanvas' style={style} />
        {
          this.state.hoverNode ? (<Paper style={{
            padding: '4px',
            position: 'absolute',
            top: (this.state.hoverY - 10) + 'px',
            left: (this.state.hoverX + 10) + 'px',
            width: '100px',
          }}>
            { 'Sample #' + this.state.hoverNode }
            { imageBase64 ?
              <img src={'data:image/png;base64, ' + imageBase64} /> :
              []
            }
          </Paper>) :
            []
        }
        <ErrorDialog ref='errorDialog' />
      </React.Fragment>
    );
  }
}

export default GraphGLWindow;
