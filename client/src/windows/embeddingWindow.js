import * as THREE from 'three';
import React from 'react';
import { withDSXContext } from '../dsxContext';
import _ from 'lodash';

/**
 * Create Graph Window
 */
class EmbeddingWindow extends React.Component {
  /**
   * Creates Embedding window object
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.state = {
      drawerAdded: false,               // when parent component adds a drawer, resize isn't called, so force it
      renderEdges: false,
      renderThumbnails: false,
      colorThumbnails: false,
      thumbnailScale: 0.25,
      nodeSize: 0.01,
      selectingDesign: false,
    };

    // Used to zoom and translate embedding
    this.maxScale = 10;
    this.minScale = 0.05;
    this.zoomRate = 1.1;
    this.previousX = 0;
    this.previousY = 0;
    this.adjacency = null;

    this.client = this.props.dsxContext.client;

    this.init = this.init.bind(this);
    this.buildTexture = this.buildTexture.bind(this);
    this.addEdgesToScene = this.addEdgesToScene.bind(this);
    this.addNodesOrThumbnailsToScene = this.addNodesOrThumbnailsToScene.bind(this);
    this.addNodesToScene = this.addNodesToScene.bind(this);
    this.addThumbnailsToScene = this.addThumbnailsToScene.bind(this);
    this.renderScene = this.renderScene.bind(this);
    this.resetScene = this.resetScene.bind(this);
    this.handleMouseScrollEvent = this.handleMouseScrollEvent.bind(this);
    this.handleMouseDownEvent = this.handleMouseDownEvent.bind(this);
    this.catchMouseMoveEvent = this.catchMouseMoveEvent.bind(this);
    this.handleMouseMoveForPanning = this.handleMouseMoveForPanning.bind(this);
    this.handleMouseReleaseEvent = this.handleMouseReleaseEvent.bind(this);
    this.handleKeyDownEvent = this.handleKeyDownEvent.bind(this);
    this.getPickPosition = this.getPickPosition.bind(this);
  }

  /**
   * Called by React when this component mounts
   */
  componentDidMount() {
    this.init();
    window.addEventListener('resize', _.debounce(this.resizeCanvas, 200));
    window.addEventListener('keydown', this.handleKeyDownEvent);
    this.refs.embeddingCanvas.addEventListener('wheel', this.handleMouseScrollEvent); // intentionally non-passive
    this.refs.embeddingCanvas.addEventListener('mousedown', this.handleMouseDownEvent);
    this.refs.embeddingCanvas.addEventListener('contextmenu', (e) => e.preventDefault(), false);
  }

  /**
   * If any of the decomposition settings have changed returns true
   * for new decomposition
   * @param {object} prevDecomposition - the previous decomposition
   * @param {object} currentDecomposition - the current decomposition
   * @return {boolean} true if any of the decomposition settings have changed.
   * 
   * WARNING: CUT AND PASTED FUNCTION ALSO IN MorseSmaleWindow
   */
  isNewDecomposition(prevDecomposition, currentDecomposition) {
    return (prevDecomposition.datasetId !== currentDecomposition.datasetId
            || prevDecomposition.category !== currentDecomposition.category
            || prevDecomposition.fieldname !== currentDecomposition.fieldname
            || prevDecomposition.modelname !== currentDecomposition.modelname
            || prevDecomposition.decompositionMode !== currentDecomposition.decompositionMode
            || prevDecomposition.k !== currentDecomposition.k
            || prevDecomposition.persistenceLevel !== currentDecomposition.persistenceLevel
            || prevDecomposition.ms !== currentDecomposition.ms);
  }

  /**
   * Called by react when this component receives new proprs or context or
   * when the state changes.
   * The data needed to draw the embedding is fetched here.
   * @param {object} prevProps
   * @param {object} prevState
   * @param {object} prevContext
   */
  componentDidUpdate(prevProps, prevState, prevContext) {
    if (this.props.decomposition === null) {
      return;
    }

    if (this.props.decomposition.fieldname === null) {
      return;
    }

    if (this.props.embedding === undefined) {
      return;
    }

    // Decomposition is loaded for the first time
    // Or has been updated so we need to load the data
    if (prevProps.decomposition === null
      || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)
      || prevProps.embedding !== this.props.embedding
      || prevProps.distanceMetric !== this.props.distanceMetric) {
      this.resetScene();
      const { datasetId, k, persistenceLevel, category, fieldname } = this.props.decomposition;
      const { embedding, distanceMetric } = this.props;
      Promise.all([
        this.client.fetchSingleEmbedding(datasetId, distanceMetric, embedding.id, k,
          persistenceLevel, category, fieldname),
        this.client.fetchThumbnails(datasetId),
      ]).then((results) => {
        const [graphResult, thumbnailResult] = results;
        this.adjacency = graphResult.embedding.adjacency;
        this.layout = graphResult.embedding.layout;
        this.colors = graphResult.colors;
        this.thumbnails = thumbnailResult.thumbnails;
        this.textures = this.buildTexture(this.thumbnails);
        this.addEdgesToScene(this.adjacency, this.layout);
        this.addNodesOrThumbnailsToScene(this.layout, this.colors, this.thumbnails, this.textures);
        this.renderScene();
      });
    }
    // Selected designs changed or state has changed
    else if (prevProps.selectedDesigns !== this.props.selectedDesigns || prevState !== this.state) {
      this.resetScene();
      this.addEdgesToScene(this.adjacency, this.layout);
      this.addNodesOrThumbnailsToScene(this.layout, this.colors, this.thumbnails, this.textures);
      this.renderScene();
    }

    // awkward, but force a resize if a drawer gets added (temporary fix for github issue #109)
    if (this.state.drawerAdded == false && this.props.drawerImages.length > 0) {
      this.resizeCanvas();
      this.state.drawerAdded = true;
    }
  }

  /**
   * Called by React when this component is removed from the DOM.
   */
  componentWillUnmount() {
    window.removeEventListener('resize', this.resizeCanvas);
    window.removeEventListener('keydown', this.handleKeyDownEvent);
    this.refs.embeddingCanvas.removeEventListener('wheel', this.handleMouseScrollEvent);
    this.refs.embeddingCanvas.removeEventListener('mousedown', this.handleMouseDownEvent);
    this.refs.embeddingCanvas.removeEventListener('mousemove', this.handleMouseMoveEvent);
    this.refs.embeddingCanvas.removeEventListener('mouseup', this.handleMouseReleaseEvent);
  }

  /**
   * Initialized the renderer, camera, and scene for Three.js
   */
  init() {
    // canvas
    let canvas = this.refs.embeddingCanvas;
    let gl = canvas.getContext('webgl');

    gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);

    // camera
    let width = canvas.clientWidth;
    let height = canvas.clientHeight;
    let sx = width / height;
    let sy = 1;
    this.camera = new THREE.OrthographicCamera(-1*sx, sx, sy, -1*sy, -1, 1);
    this.camera.position.set(0, 0, 1);

    // world
    this.scene = new THREE.Scene();

    // picking scene
    this.pickingScene = new THREE.Scene();
    this.pickingScene.background = new THREE.Color(0);
    this.pickHelper = new PickHelper();
    this.idToObject = {};

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl, antialias:true });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight, false);
    this.renderer.sortObjects = false;

    this.renderScene();
  }

  /**
   * Build texture for thumbnails
   * @param {array} thumbnails - raw thumbnail data
   * @return {array} textures - list of textures
   */
  buildTexture(thumbnails) {
    let textures = [];
    thumbnails.forEach((thumbnail) => {
      const image = new Image();
      image.src = 'data:image/jpeg;base64, ' + thumbnail.rawData;
      const texture = new THREE.Texture();
      texture.image = image;
      texture.needsUpdate = true;
      texture.wrapS = texture.wrapT = THREE.ClampToEdgeWrapping;
      textures.push(texture);
    });
    return textures;
  }

  /**
   * Add graph edges to scene
   * @param {array} adjacencyMatrix - sample indexes with edges between them
   * @param {array} sampleCoordinates - sample coordinates
   */
  addEdgesToScene(adjacencyMatrix, sampleCoordinates) {
    if (this.state.renderEdges) {
      adjacencyMatrix.forEach((edge) => {
        let endPoint1 = sampleCoordinates[edge[0]];
        let endPoint2 = sampleCoordinates[edge[1]];

        let lineMaterial = new THREE.LineBasicMaterial({ color:0x5C5C5C, linewidth:0.001 });
        let lineGeometry = new THREE.Geometry();
        lineGeometry.vertices.push(new THREE.Vector3(endPoint1[0], endPoint1[1], 0));
        lineGeometry.vertices.push(new THREE.Vector3(endPoint2[0], endPoint2[1], 0));
        let line = new THREE.Line(lineGeometry, lineMaterial);
        this.scene.add(line);
      });
    }
  }

  /**
   * Draws nodes or thumbnails to scene depending on
   * settings.
   * @param {array} nodeCoordinates - where the nodes should be placed
   * @param {array} nodeColors - color of nodes
   * @param {array} thumbnails - thumbnail information
   * @param {array} textures - texture for drawing thumbnails
   */
  addNodesOrThumbnailsToScene(nodeCoordinates, nodeColors, thumbnails, textures) {
    if (this.state.renderThumbnails) {
      this.addThumbnailsToScene(nodeCoordinates, nodeColors, thumbnails, textures);
    } else {
      this.addNodesToScene(nodeCoordinates, nodeColors);
    }
  }

  /**
   * Add the sample nodes to the scene
   * @param {array} nodeCoordinates
   * @param {array} nodeColors
   */
  addNodesToScene(nodeCoordinates, nodeColors) {
    if (nodeCoordinates)
      console.log("adding " + nodeCoordinates.length.toString() + " nodes to scene");
    else
      console.log("called addNodesToScene with null nodeCoordinates");

    nodeCoordinates.forEach((coord, index) => {
      // Add Circle
      let nodeGeometry = new THREE.CircleGeometry(this.state.nodeSize, 32);

      // If design is selected color, else grey
      let nodeMaterial;
      if (this.props.selectedDesigns.size === 0 || this.props.selectedDesigns.has(index)) {
        let color = new THREE.Color();
        color.setRGB(nodeColors[index][0], nodeColors[index][1], nodeColors[index][2]);
        nodeMaterial = new THREE.MeshBasicMaterial({ color:color });
      } else {
        nodeMaterial = new THREE.MeshBasicMaterial({ color:0xE9E9E9 });
      }

      let nodeMesh = new THREE.Mesh(nodeGeometry, nodeMaterial);
      nodeMesh.translateX(coord[0]);
      nodeMesh.translateY(coord[1]);
      nodeMesh.name = index;

      // Outline Circle
      let edges = new THREE.EdgesGeometry(nodeGeometry);
      let line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color:0x000000 }));
      line.translateX(coord[0]);
      line.translateY(coord[1]);
      line.name = 'line'+index;

      this.scene.add(line);
      this.scene.add(nodeMesh);

      // Add to picking Scene is is off by one so it is distinguishable from black background.
      let id = index + 1;
      this.idToObject[id] = nodeMesh;
      const pickingMaterial = new THREE.MeshPhongMaterial({
        emissive: new THREE.Color(id),
        color: new THREE.Color(0, 0, 0),
        specular: new THREE.Color(0, 0, 0),
        transparent: true,
        side: THREE.DoubleSide,
        alphaTest: 0.5,
        blending: THREE.NoBlending,
      });
      const pickingNode = new THREE.Mesh(nodeGeometry, pickingMaterial);
      this.pickingScene.add(pickingNode);
      pickingNode.position.copy(nodeMesh.position);
    });
  }

  /**
   * Adds thumbnails to scene as nodes of embedding
   * @param {array} nodeCoordinates - where each thumbnail should be placed
   * @param {array} nodeColors - color for reach node
   * @param {array} thumbnails - thumbnails
   * @param {array} textures - textures to be used
   */
  addThumbnailsToScene(nodeCoordinates, nodeColors, thumbnails, textures) {
    const canvas = this.refs.embeddingCanvas;
    const canvasWidth = canvas.clientWidth;
    const canvasHeight = canvas.clientHeight;
    const scale = this.state.thumbnailScale / canvasWidth;

    nodeCoordinates.forEach((coord, index) => {
      const thumbnailWidth = thumbnails[index].width;
      const thumbnailHeight = thumbnails[index].height;

      // todo: using a sprite here like in morseSmaleWindow might be faster
      const nodeGeometry = new THREE.BoxGeometry(
        scale * thumbnailWidth,
        scale * thumbnailHeight, 0);

      // Color based on QoI if enabled, if not enabled selected design is colored orange
      // to match the gallery view.
      let color;
      if (this.state.colorThumbnails
        && (this.props.selectedDesigns.size === 0|| this.props.selectedDesigns.has(index))) {
        color = new THREE.Color();
        color.setRGB(nodeColors[index][0], nodeColors[index][1], nodeColors[index][2]);
      } else if (!this.state.colorThumbnails && this.props.selectedDesigns.has(index)) {
        color = 0xFFA500;
      } else {
        color = 0xFFFFFF;
      }
      const nodeMaterial = new THREE.MeshBasicMaterial({ color:color, map:textures[index] });

      let nodeMesh = new THREE.Mesh(nodeGeometry, nodeMaterial);
      nodeMesh.translateX(coord[0]);
      nodeMesh.translateY(coord[1]);

      // Outline Rectangle
      let edges = new THREE.EdgesGeometry(nodeGeometry);
      let line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color:0x000000 }));
      line.translateX(coord[0]);
      line.translateY(coord[1]);

      this.scene.add(line);
      this.scene.add(nodeMesh);
    });
  }

  /**
   * Draw the scene to the canvas
   */
  renderScene() {
    this.renderer.render(this.scene, this.camera);
  }

  /**
   * Resets the scene when there is new data by removing
   * the old scene children.
   */
  resetScene() {
    while (this.scene.children.length > 0) {
      this.scene.remove(this.scene.children[0]);
    }
  }

  /**
   * Called when the canvas is resized.
   * This can happen on a window resize or when another window is added to dSpaceX.
   * @param {boolean} newWindowAdded
   */
  resizeCanvas = () => {
    let width = this.refs.embeddingCanvas.clientWidth;
    let height = this.refs.embeddingCanvas.clientHeight;

    // Resize renderer
    this.renderer.setSize(width, height, false);

    // Resize camera
    let sx = width / height;
    let sy = 1;         // setting sx/sy consistently has most consistent result, esp when window is *slowly* resized
    this.camera.left = -1*sx;
    this.camera.right = sx;
    this.camera.top = sy;
    this.camera.bottom = -1*sy;
    this.camera.updateProjectionMatrix();

    this.renderScene();
  };

  /**
   * Handles when the mouse is scrolled for increasing and decreasing
   * the embedding
   * @param {object} event
   */
  handleMouseScrollEvent(event) {
    event.preventDefault(); // combined with non-passive event binding, doesn't scroll window when zooming
    if (event.deltaY > 0 && this.camera.zoom > -this.maxScale) {
      this.camera.zoom = this.camera.zoom / this.zoomRate;
    }
    if (event.deltaY < 0 && this.camera.zoom < this.maxScale*10.0) {
      this.camera.zoom = this.camera.zoom * this.zoomRate;
    }
    this.camera.updateProjectionMatrix();
    this.renderScene();
  }

  /**
   * Handles mouse down event.
   * Part of the embedding translation pipeline.
   * @param {object} event
   */
  handleMouseDownEvent(event) {
    this.refs.embeddingCanvas.addEventListener('mousemove', this.catchMouseMoveEvent);
    this.refs.embeddingCanvas.addEventListener('mouseup', this.handleMouseReleaseEvent, { passive:true });

    // if mouse gets released without moving, we'll try to select a design
    this.state.selectingDesign = true;

    // if mouse moves, translate viewpoint until released
    this.previousX = event.offsetX;
    this.previousY = event.offsetY;
  }

  /**
   * Catches first mouse move event after mouse down to disable selection and enable panning
   * If mouse is moving with a button down, don't try to select a design on mouseup.
   */
  catchMouseMoveEvent(event) {
    // let some teeeeny movement slide
    const allowed = 2;
    let dx = this.previousX - event.offsetX;
    let dy = this.previousY - event.offsetY;
    if (Math.abs(dx + dy) > allowed) { 
      this.state.selectingDesign = false;
      this.refs.embeddingCanvas.removeEventListener('mousemove', this.catchMouseMoveEvent);

      // now we're panning, so let that function be used to (passively) handle mouse moves events after this
      this.refs.embeddingCanvas.addEventListener('mousemove', this.handleMouseMoveForPanning, { passive:true });
      this.handleMouseMoveForPanning(event);
    }
  }

  /**
   * Handles (passive) mouse move event for panning
   * Part of the embedding translation pipeline.
   * @param {object} event
   */
  handleMouseMoveForPanning(event) {
    let canvas = this.refs.embeddingCanvas;
    let x = event.offsetX;
    let y = event.offsetY;

    let dx = this.previousX - x;
    let dy = y - this.previousY;
    let scaleX = (this.camera.right - this.camera.left) / this.camera.zoom / canvas.clientWidth;
    let scaleY = (this.camera.top - this.camera.bottom) / this.camera.zoom / canvas.clientHeight;
    this.camera.translateX(scaleX * dx);
    this.camera.translateY(scaleY * dy);

    this.camera.updateProjectionMatrix();
    this.renderScene();

    this.previousX = x;
    this.previousY = y;
  }

  /**
   * Handles mouse release event.
   * Part of the embedding translation pipeline.
   * @param {object} event
   */
  handleMouseReleaseEvent(event) {
    this.refs.embeddingCanvas.removeEventListener('mouseup', this.handleMouseReleaseEvent);
    this.refs.embeddingCanvas.removeEventListener('mousemove', this.catchMouseMoveEvent);
    this.refs.embeddingCanvas.removeEventListener('mousemove', this.handleMouseMoveForPanning);

    // handle left click release if we're selecting
    const leftClick = 0;
    if (this.state.selectingDesign && event.button === leftClick) {
      this.state.selectingDesign = false;

      const pickPosition = this.getPickPosition(event);
      const pickedObject = this.pickHelper.pick(pickPosition,
        this.pickingScene,
        this.camera,
        this.renderer,
        this.idToObject);
      let selectedDesignId = -1;
      if (pickedObject) {
        selectedDesignId = pickedObject.name;
      }
      this.props.onDesignSelection(event, selectedDesignId);
    }
  }

  /**
   * Handles hotkey events.
   * @param {object} event
   */
  handleKeyDownEvent(event) {
    let newScale;
    switch (event.key) {
      case 'e': // Enable/disable edges
        this.setState({ renderEdges:!this.state.renderEdges });
        break;
      case 't': // Enable/disable thumbnails
        this.setState({ renderThumbnails:!this.state.renderThumbnails });
        break;
      case 'i': // show sample ids
        //this.setState({ renderIds:!this.state.renderIds });
      //todo: how do we draw text in the node?
      // https://threejs.org/docs/#api/en/geometries/TextGeometry (but this _can't_ be efficient)
      break;
      case '=': // Increase thumbnail size
      case '+':
        if (this.state.renderThumbnails) {
          newScale = this.state.thumbnailScale + 0.05;
          if (newScale > this.maxScale) {
            newScale = this.maxScale;
          }
          this.setState({ thumbnailScale:newScale });
        }
        else {
          this.setState({ nodeSize : this.state.nodeSize + 0.002 });
        }
        break;
      case '-': // decrease thumbnail size
        if (this.state.renderThumbnails) {
          newScale = this.state.thumbnailScale - 0.05;
          if (newScale < this.minScale) {
            newScale = this.minScale;
          }
          this.setState({ thumbnailScale:newScale });
        }
        else {
          this.setState({ nodeSize : Math.max(0.001, this.state.nodeSize - 0.002) });
        }
        break;
      case 'c': // enable color for thumbnail
        this.setState({ colorThumbnails:!this.state.colorThumbnails });
        break;
    }
    this.renderScene();
  }

  /**
   * Decode a base64 array into a Uint8Array - used to convert image
   * data to usable for by Three.js texture
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
   * Gets the picked position
   * @param {object} event
   * @return {{x: number, y: number}}
   */
  getPickPosition(event) {
    const rect = this.refs.embeddingCanvas.getBoundingClientRect();
    return {
      x: event.clientX - rect.left,
      y: event.clientY - rect.top,
    };
  }

  /**
   * Renders Embedding
   * @return {JSX}
   */
  render() {
    const style = {
      height: '100%',
      width: '100%',
    };
    return (
        <canvas ref='embeddingCanvas' style={style}/>);
  }
}

/**
 * Helper class to assist with picking
 */
class PickHelper {
  /**
   * Create PickHelper Object
   */
  constructor() {
    this.pickingTexture = new THREE.WebGLRenderTarget(1, 1);
    this.pixelBuffer = new Uint8Array(4);
    this.pickedObject = null;
  }

  /**
   * Returns object from canvas position
   * @param {object} canvasPosition
   * @param {object} scene
   * @param {object} camera
   * @param {object} renderer
   * @param {set} idToObject
   * @return {*} Selected object; null if nothing is selected
   */
  pick(canvasPosition, scene, camera, renderer, idToObject) {
    const { pickingTexture, pixelBuffer } = this;

    if (this.pickedObject) {
      this.pickedObject = null;
    }

    const pixelRatio = renderer.getPixelRatio();
    camera.setViewOffset(
      renderer.getContext().drawingBufferWidth,
      renderer.getContext().drawingBufferHeight,
      canvasPosition.x * pixelRatio | 0,
      canvasPosition.y * pixelRatio | 0,
      1,
      1,
    );

    renderer.setRenderTarget(pickingTexture);
    renderer.render(scene, camera);
    renderer.setRenderTarget(null);

    camera.clearViewOffset();

    renderer.readRenderTargetPixels(
      pickingTexture,
      0,
      0,
      1,
      1,
      pixelBuffer
    );
    const id = (pixelBuffer[0] << 16) |
               (pixelBuffer[1] << 8) |
               (pixelBuffer[2]);

    const intersectedObject = idToObject[id];
    if (intersectedObject) {
      this.pickedObject = intersectedObject;
    }
    return this.pickedObject;
  }
}

export default withDSXContext(EmbeddingWindow);
