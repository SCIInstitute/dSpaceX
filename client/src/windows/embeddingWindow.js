import * as THREE from 'three';
import Paper from '@material-ui/core/Paper';
import React from 'react';
import { withDSXContext } from '../dsxContext';

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

    this.state = { renderEdges:false };

    // Used to zoom and translate embedding
    this.maxScale = 10;
    this.zoomRate = 1.1;
    this.rightMouseDown = false;
    this.previousX = 0;
    this.previousY = 0;

    this.client = this.props.dsxContext.client;

    this.init = this.init.bind(this);
    this.addNodesToScene = this.addNodesToScene.bind(this);
    this.addEdgesToScene = this.addEdgesToScene.bind(this);
    this.renderScene = this.renderScene.bind(this);
    this.animate = this.animate.bind(this);
    this.resetScene = this.resetScene.bind(this);
    this.resizeCanvas = this.resizeCanvas.bind(this);
    this.handleMouseScrollEvent = this.handleMouseScrollEvent.bind(this);
    this.handleMouseDownEvent = this.handleMouseDownEvent.bind(this);
    this.handleMouseMoveEvent = this.handleMouseMoveEvent.bind(this);
    this.handleMouseReleaseEvent = this.handleMouseReleaseEvent.bind(this);
    this.handleKeyDownEvent = this.handleKeyDownEvent.bind(this);
  }

  /**
   * Called by React when this component mounts
   */
  componentDidMount() {
    this.init();
    this.animate();
    window.addEventListener('resize', this.resizeCanvas);
    window.addEventListener('keydown', this.handleKeyDownEvent);
    this.refs.embeddingCanvas.addEventListener('wheel', this.handleMouseScrollEvent, { passive:true });
    this.refs.embeddingCanvas.addEventListener('mousedown', this.handleMouseDownEvent, { passive:true });
    this.refs.embeddingCanvas.addEventListener('mousemove', this.handleMouseMoveEvent, { passive:true });
    this.refs.embeddingCanvas.addEventListener('mouseup', this.handleMouseReleaseEvent, { passive:true });
    this.refs.embeddingCanvas.addEventListener('contextmenu', (e) => e.preventDefault(), false);
  }

  /**
   * Called by react when this component receives new proprs or context or
   * when the state changes.
   * The data needed to draw teh embedding is fetched here.
   * @param {object} prevProps
   * @param {object} prevState
   * @param {object} prevContext
   */
  componentDidUpdate(prevProps, prevState, prevContext) {
    if (this.props.decomposition === null) {
      return;
    }

    if (this.props.decomposition.decompositionField === null) {
      return;
    }

    // New window has been added to application
    if (this.props.numberOfWindows !== prevProps.numberOfWindows) {
      this.resizeCanvas();
    }

    // Decomposition is loaded for the first time
    // Or has been updated
    if (prevProps.decomposition === null
      || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)) {
      this.resetScene();
      const { datasetId, k, persistenceLevel } = this.props.decomposition;
      const qoiName = this.props.decomposition.decompositionField;
      this.client.fetchGraphEmbedding(datasetId, k, persistenceLevel, qoiName).then((result) => {
        this.adjacency = result.embedding.adjacency;
        this.layout = result.embedding.layout;
        this.colors = result.colors;
        this.addEdgesToScene(this.adjacency, this.layout);

        this.addNodesToScene(this.layout, this.colors);
        this.renderScene();
      });
    }

    // Selected designs changed
    if (prevProps.selectedDesigns !== this.props.selectedDesigns) {
      this.resetScene();
      this.addEdgesToScene(this.adjacency, this.layout);
      this.addNodesToScene(this.layout, this.colors);
      this.renderScene();
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
   * If any of the decomposition setting have changed returns true for
   * new decomposition
   * @param {object} prevDecomposition
   * @param {object} currentDecomposition
   * @return {boolean} true if any of the decomposition setting have chagned
   */
  isNewDecomposition(prevDecomposition, currentDecomposition) {
    return (prevDecomposition.datasetId !== currentDecomposition.datasetId
        || prevDecomposition.decompositionCategory !== currentDecomposition.decompositionCategory
        || prevDecomposition.decompositionField !== currentDecomposition.decompositionField
        || prevDecomposition.decompositionMode !== currentDecomposition.decompositionMode
        || prevDecomposition.k !== currentDecomposition.k
        || prevDecomposition.persistenceLevel !== currentDecomposition.persistenceLevel);
  }

  /**
   * Initialized the renderer, camera, and scene for Three.js
   */
  init() {
    // canvas
    let canvas = this.refs.embeddingCanvas;
    let gl = canvas.getContext('webgl');

    // camera
    let width = canvas.clientWidth;
    let height = canvas.clientHeight;
    let sx = 1;
    let sy = 1;
    if (width > height) {
      sx = width / height;
    } else {
      sy = height / width;
    }
    this.camera = new THREE.OrthographicCamera(-1*sx, 1*sx, 1*sy, -1*sy, -1, 1);
    this.camera.position.set(0, 0, -1);

    // world
    this.scene = new THREE.Scene();

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight, false);
    this.renderer.sortObjects = false;

    this.renderScene();
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
   * Add the sample nodes to the scene
   * @param {array} nodeCoordinates
   * @param {array} nodeColors
   */
  addNodesToScene(nodeCoordinates, nodeColors) {
    nodeCoordinates.forEach((coord, index) => {
      // Add Circle
      let nodeGeometry = new THREE.CircleGeometry(0.01, 32);

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
    });
  }

  /**
   * Draw the scene to the canvas
   */
  renderScene() {
    this.renderer.render(this.scene, this.camera);
  }

  /**
   * Animated the scene.
   * This is necessary for interactivity.
   */
  animate() {
    requestAnimationFrame(this.animate);
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
   */
  resizeCanvas() {
    let width = this.refs.embeddingCanvas.clientWidth;
    let height = this.refs.embeddingCanvas.clientHeight;

    this.refs.embeddingCanvas.width = width;
    this.refs.embeddingCanvas.height = height;

    // Resize renderer
    this.renderer.setSize(width, height, false);

    // Resize camera
    // this.sizeCamera(width, height);
    let sx = 1;
    let sy = 1;
    if (width > height) {
      sx = width / height;
    } else {
      sy = height / width;
    }
    this.camera.left = -1*sx;
    this.camera.right = 1*sx;
    this.camera.top = 1*sy;
    this.camera.bottom = -1*sy;
    this.camera.updateProjectionMatrix();

    // Redraw scene
    this.renderScene();
  }

  /**
   * Handles when the mouse is scrolled for increasing and decreasing
   * the embedding
   * @param {object} event
   */
  handleMouseScrollEvent(event) {
    if (event.deltaY < 0 && this.camera.zoom > -this.maxScale) {
      this.camera.zoom = this.camera.zoom / this.zoomRate;
    }
    if (event.deltaY > 0 && this.camera.zoom < this.maxScale) {
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
    let rightClick = 2;
    if (event.button === rightClick) {
      this.rightMouseDown = true;
      this.previousX = event.offsetX;
      this.previousY = event.offsetY;
    }
  }

  /**
   * Handles mouse move event.
   * Part of the embedding translation pipeline.
   * @param {object} event
   */
  handleMouseMoveEvent(event) {
    let canvas = this.refs.embeddingCanvas;
    let x = event.offsetX;
    let y = event.offsetY;

    if (this.rightMouseDown) {
      let dx = this.previousX - x;
      let dy = y - this.previousY;
      let scaleX = (this.camera.right - this.camera.left) / this.camera.zoom / canvas.clientWidth;
      let scaleY = (this.camera.top - this.camera.bottom) / this.camera.zoom / canvas.clientHeight;
      this.camera.translateX(scaleX * dx);
      this.camera.translateY(scaleY * dy);

      this.camera.updateProjectionMatrix();
      this.renderScene();
    }

    this.previousX = x;
    this.previousY = y;
  }

  /**
   * Handles mouse release event.
   * Part of the embedding translation pipeline.
   * @param {object} event
   */
  handleMouseReleaseEvent(event) {
    let rightClick = 2;
    if (event.button === rightClick) {
      this.rightMouseDown = false;
    }
  }

  /**
   * Handles hotkey events.
   * @param {object} event
   */
  handleKeyDownEvent(event) {
    // Currently not a switch statement but there are other options that need to be added
    switch (event.key) {
      case 'e':
        this.setState({ renderEdges:!this.state.renderEdges });
        this.resetScene();
        this.addEdgesToScene(this.adjacency, this.layout);
        this.addNodesToScene(this.layout, this.colors);
    }
    this.renderScene();
  }

  /**
   * Renders Embedding
   * @return {JSX}
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
    return (
      <Paper style={paperStyle}>
        <canvas ref='embeddingCanvas' style={canvasStyle}/>
      </Paper>
    );
  }
}

export default withDSXContext(EmbeddingWindow);
