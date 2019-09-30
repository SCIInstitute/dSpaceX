import * as THREE from 'three';
import Paper from '@material-ui/core/Paper';
import React from 'react';
import { withDSXContext } from '../dsxContext'
import {breakStatement} from "@babel/types";

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

    this.client = this.props.dsxContext.client;

    this.init = this.init.bind(this);
    this.resizeCanvas = this.resizeCanvas.bind(this);
    this.addNodesToScene = this.addNodesToScene.bind(this);
    this.renderScene = this.renderScene.bind(this);
    this.animate = this.animate.bind(this);
  }

  /**
   * Called by React when this component mounts
   */
  componentDidMount() {
    this.init();
    this.animate();
    window.addEventListener('resize', this.resizeCanvas);
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

    if (this.props.numberOfWindows !== prevProps.numberOfWindows) {
      this.resizeCanvas();
    }

    if (prevProps.decomposition === null
      || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)) {
      const { datasetId, k } = this.props.decomposition;
      const qoiName = this.props.decomposition.decompositionField;
      Promise.all([
        this.client.fetchGraphEmbedding(datasetId, k),
        this.client.fetchQoi(datasetId, qoiName),
      ]).then((results) => {
        const [embeddingResult, qoiResult] = results;
        this.addNodesToScene(embeddingResult.embedding.layout);
        this.renderScene();
      });
    }
  }

  /**
   * Called by React when this component is removed from the DOM.
   */
  componentWillUnmount() {
    window.removeEventListener('resize', this.resizeCanvas);
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

    // light
    // TODO add light once you have basic idea working - if it needs light you will also have to change the mesh type

    // world
    this.scene = new THREE.Scene();

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight, false);

    this.renderScene();
  }

  /**
   * Called when the canvas is resized.
   * This can happen on a window resize or when another window is added to dSpaceX.
   */
  resizeCanvas() {
    let width = this.refs.embeddingCanvas.clientWidth;
    let height = this.ref.embeddingCanvas.clientHeight;

    this.refs.embeddingCanvas.width = width;
    this.refs.embeddingCanvs.height = height;

    // Resize renderer
    this.renderer.setSize(width, height, false);

    // Resize camera
    this.sizeCamera(width, height);
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
   * Add the sample nodes to the scene
   * @param {array} nodeCoordinates
   */
  addNodesToScene(nodeCoordinates) {
    // console.log('Add Node to Scene Called');
    // let geometry = new THREE.CircleGeometry(100, 32);
    // let material = new THREE.MeshBasicMaterial({ color:0x44aa88 });
    // let mesh = new THREE.Mesh(geometry, material);
    // this.scene.add(mesh);
    nodeCoordinates.forEach((coord, index) => {
      // Add Circle
      let nodeGeometry = new THREE.CircleGeometry(0.01, 32);
      let nodeMaterial = new THREE.MeshBasicMaterial({ color:0x44aa88 });
      let nodeMesh = new THREE.Mesh(nodeGeometry, nodeMaterial);
      nodeMesh.translateX(coord[0]);
      nodeMesh.translateY(coord[1]);
      nodeMesh.name = index;

      // Outline Circle
      let edges = new THREE.EdgesGeometry(nodeGeometry);
      let line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color:0xffffff }));
      line.translateX(coord[0]);
      line.translateY(coord[1]);

      this.scene.add(nodeMesh);
      this.scene.add(line);
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
