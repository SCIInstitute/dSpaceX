import * as THREE from 'three';
import { OrthographicTrackballControls } from 'three/examples/jsm/controls/OrthographicTrackballControls';
import Paper from '@material-ui/core/Paper';
import React from 'react';
import { withDSXContext } from '../dsxContext';

/**
 * Creates Morse-Smale decomposition
 */
class MorseSmaleWindow extends React.Component {
  /**
   * Creates Morse-Smale window object
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.client = this.props.dsxContext.client;

    this.init = this.init.bind(this);
    this.resizeCanvas = this.resizeCanvas.bind(this);
    this.mouseRelease = this.mouseRelease.bind(this);
    this.getCanvasPosition = this.getCanvasPosition.bind(this);
    this.getPickPosition = this.getPickPosition.bind(this);
    this.pick = this.pick.bind(this);
    this.getIndexOfNearestNeighbor = this.getIndexOfNearestNeighbor.bind(this);
    this.addRegressionCurvesToScene = this.addRegressionCurvesToScene.bind(this);
    this.addExtremaToScene = this.addExtremaToScene.bind(this);
    this.renderScene = this.renderScene.bind(this);
    this.animate = this.animate.bind(this);
    this.resetScene = this.resetScene.bind(this);
  }

  /**
   * Called by react when this component mounts.
   * Initializes Three.js for drawing and adds event listeners.
   */
  componentDidMount() {
    this.init();
    this.animate();
    window.addEventListener('resize', this.resizeCanvas);
    this.refs.msCanvas.addEventListener('mousedown', this.mouseRelease, { passive:true });
  }

  /**
   * Called by react when this component receives new props or context or
   * when the state changes.
   * The data needed to draw the Morse-Smale decomposition it fetched here.
   * @param {object} prevProps
   * @param {object} prevState
   * @param {object} prevContext
   */
  componentDidUpdate(prevProps, prevState, prevContext) {
    if (this.props.decomposition === null) {
      return;
    }

    if (this.props.numberOfWindows !== prevProps.numberOfWindows) {
      this.resizeCanvas();
    }

    if (prevProps.decomposition === null
      || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)) {
      this.resetScene();
      const { datasetId, k, persistenceLevel } = this.props.decomposition;
      Promise.all([
        this.client.fetchMorseSmaleRegression(datasetId, k, persistenceLevel),
        this.client.fetchMorseSmaleExtrema(datasetId, k, persistenceLevel),
      ]).then((response) => {
        const [regressionResponse, extremaResponse] = response;
        this.regressionCurves = regressionResponse;
        this.addRegressionCurvesToScene(regressionResponse);
        this.addExtremaToScene(extremaResponse.extrema);
        this.renderScene();
      });
    }
  }

  /**
   * Called by React when this component is removed from the DOM.
   */
  componentWillUnmount() {
    window.removeEventListener('resize', this.resizeCanvas);
    this.refs.msCanvas.removeEventListener('mousedown', this.mouseRelease);
  }

  /**
   * If any of the decomposition settings have changed returns true
   * for new decomposition
   * @param {object} prevDecomposition - the previous decomposition
   * @param {object} currentDecomposition - the current decomposition
   * @return {boolean} true if any of the decomposition settings have changed.
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
   * Initializes the renderer, camera, and scene for Three.js.
   */
  init() {
    // canvas
    let canvas = this.refs.msCanvas;
    let gl = canvas.getContext('webgl');

    // camera
    let width = canvas.clientWidth;
    let height = canvas.clientHeight;
    let sx = 1;
    let sy = 1;
    if (width > height) {
      sx = width/height;
    } else {
      sy = height/width;
    }
    this.camera = new THREE.OrthographicCamera(-4*sx, 4*sx, 4*sy, -4*sy, -16, 16);
    this.camera.position.set(0, -1, 0);
    this.camera.up.set(0, 0, 1);
    this.camera.zoom = 2;

    // light
    this.ambientLight = new THREE.AmbientLight( 0x404040 ); // soft white light
    this.frontDirectionalLight = new THREE.DirectionalLight(0xffffff);
    this.frontDirectionalLight.position.set(5, -1, 5);
    this.backDirectionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    this.backDirectionalLight.position.set(-5, -1, -5);

    // world
    this.scene = new THREE.Scene();
    this.scene.add(this.ambientLight);
    this.scene.add(this.frontDirectionalLight);
    this.scene.add(this.backDirectionalLight);

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight, false);

    // controls
    this.controls = new OrthographicTrackballControls(this.camera, this.renderer.domElement);
    this.controls.rotateSpeed = 0.75;
    this.controls.zoomSpeed = 0.1;
    this.controls.panSpeed = 0.5;
    this.controls.noZoom = false;
    this.controls.noPan = false;
    this.controls.staticMoving = true;
    this.controls.keys = [65, 83, 68];
    this.controls.addEventListener( 'change', this.renderScene );

    // picking
    this.raycaster = new THREE.Raycaster();

    this.renderScene();
  }

  /**
   * Called when the canvas is resized.
   * This can happen on a window resize or when another window is added to dSpaceX.
   */
  resizeCanvas() {
    let width = this.refs.msCanvas.clientWidth;
    let height = this.refs.msCanvas.clientHeight;

    this.refs.msCanvas.width = width;
    this.refs.msCanvas.height = height;

    // Resize renderer
    this.renderer.setSize(width, height, false );

    // Resize Camera
    let sx = 1;
    let sy = 1;
    if (width > height) {
      sx = width/height;
    } else {
      sy = height/width;
    }
    this.camera.left = -4*sx;
    this.camera.right = 4*sx;
    this.camera.top = 4*sy;
    this.camera.bottom = -4*sy;
    this.camera.updateProjectionMatrix();

    // Update controls
    this.controls = new OrthographicTrackballControls(this.camera, this.renderer.domElement);
    this.controls.rotateSpeed = 0.75;
    this.controls.zoomSpeed = 0.1;
    this.controls.panSpeed = 0.5;
    this.controls.noZoom = false;
    this.controls.noPan = false;
    this.controls.staticMoving = true;
    this.controls.keys = [65, 83, 68];
    this.controls.addEventListener( 'change', this.renderScene );

    // Redraw scene with updates
    this.renderScene();
  }

  /**
   * Event handling for mouse click release
   * @param {Event} event
   */
  mouseRelease(event) {
    // Handle left click release
    if (event.button === 0) {
      const position = this.getPickPosition(event);
      this.pick(position);
    }
  }

  /**
   * Gets the click coordinates on the canvas - used for picking
   * @param {Event} event
   * @return {{x: number, y: number}}
   */
  getCanvasPosition(event) {
    const rect = this.refs.msCanvas.getBoundingClientRect();
    return {
      x: event.clientX - rect.left,
      y: event.clientY - rect.top,
    };
  }

  /**
    * Converts pick position to clip space
    * @param {Event} event
    * @return {{x: number, y: number}}
    */
  getPickPosition(event) {
    const pos = this.getCanvasPosition(event);
    const canvas = this.refs.msCanvas;
    return {
      x: (pos.x / canvas.clientWidth) * 2 - 1,
      y: (pos.y / canvas.clientHeight) * -2 + 1,
    };
  }

  /**
   * Pick level set of decomposition
   * @param {object} normalizedPosition
   */
  pick(normalizedPosition) {
    this.raycaster.setFromCamera(normalizedPosition, this.camera);
    let intersectedObjects = this.raycaster.intersectObjects(this.scene.children);
    intersectedObjects = intersectedObjects.filter((io) => io.object.name !== '');
    if (intersectedObjects.length) {
      let cell = intersectedObjects[0].object.name;
      let point = this.getIndexOfNearestNeighbor(intersectedObjects[0]);
    }
  }

  /**
   * Finds the index of the nearest neighbor to point
   * @param {object} mesh
   * @return {number} index of nearest neighbor
   */
  getIndexOfNearestNeighbor(mesh) {
    const regressionPoints = [...this.regressionCurves.curves[mesh.object.name].points];
    // Remove first and last elements from array - these are for drawing the regression lines
    regressionPoints.shift();
    regressionPoints.pop();
    let smallestDistance = Infinity;
    let nearestIndex = Infinity;
    regressionPoints.forEach((regPoint, index) => {
      let euclidDistance = this.euclideanDistance(mesh.point, regPoint);
      if (euclidDistance < smallestDistance) {
        smallestDistance = euclidDistance;
        nearestIndex = index;
      }
    });
    return nearestIndex;
  }

  /**
   * Calculates the euclidean distance between two points.
   * @param {object} meshPoint
   * @param {array<number>}regPoint
   * @return {number} euclidean distance between two points
   */
  euclideanDistance(meshPoint, regPoint) {
    let x = meshPoint.x - regPoint[0];
    let y = meshPoint.y - regPoint[1];
    let z = meshPoint.z - regPoint[2];
    return Math.sqrt(x**2 + y**2 + z**2);
  }
  /**
   * Adds the regression curves to the scene
   * @param {object} regressionData
   */
  addRegressionCurvesToScene(regressionData) {
    regressionData.curves.forEach((regressionCurve, index) => {
      let curvePoints = [];
      regressionCurve.points.forEach((regressionPoint) => {
        curvePoints.push(new THREE.Vector3(regressionPoint[0], regressionPoint[1], regressionPoint[2]));
      });
      // Create curve
      let curve = new THREE.CatmullRomCurve3(curvePoints);
      let tubularSegments = 50;
      let curveGeometry = new THREE.TubeBufferGeometry(curve, tubularSegments, .02, 50, false);
      let count = curveGeometry.attributes.position.count;
      curveGeometry.addAttribute('color', new THREE.BufferAttribute(new Float32Array(count * 3), 3));
      let colors = regressionCurve.colors;
      let colorAttribute = curveGeometry.attributes.color;
      let color = new THREE.Color();
      for (let i = 0; i < curvePoints.length; ++i) {
        color.setRGB(colors[i][0], colors[i][1], colors[i][2]);
        for (let j = 0; j < tubularSegments; ++j) {
          colorAttribute.setXYZ(i*tubularSegments+j, color.r, color.g, color.b);
        }
      }
      let curveMaterial = new THREE.MeshLambertMaterial({
        color: 0xffffff,
        flatShading: true,
        vertexColors: THREE.VertexColors });
      let curveMesh = new THREE.Mesh(curveGeometry, curveMaterial);
      curveMesh.name = index;
      this.scene.add(curveMesh);
    });
  }

  /**
   * Adds the extrema to the scene.
   * @param {object} extrema
   */
  addExtremaToScene(extrema) {
    extrema.forEach((extreme) => {
      let extremaGeometry = new THREE.SphereBufferGeometry(0.05, 32, 32);
      let color = new THREE.Color(extreme.color[0], extreme.color[1], extreme.color[2]);
      let extremaMaterial = new THREE.MeshLambertMaterial({ color:color });
      let extremaMesh = new THREE.Mesh(extremaGeometry, extremaMaterial);
      // extremaMesh.rotateX(-90);
      extremaMesh.translateX(extreme.position[0]);
      extremaMesh.translateY(extreme.position[1]);
      extremaMesh.translateZ(extreme.position[2]);
      this.scene.add(extremaMesh);
    });
  }

  /**
   * Draws the scene to the canvas.
   */
  renderScene() {
    this.renderer.render(this.scene, this.camera);
  }

  /**
   * Animates the scene.
   * This is necessary for the Trackball Controls or any other interactivity.
   */
  animate() {
    requestAnimationFrame(this.animate);
    this.controls.update();
  }

  /**
   * Resets the scene when there is new data by removing
   * the old scene children and adding back the lights.
   */
  resetScene() {
    while (this.scene.children.length > 0) {
      this.scene.remove(this.scene.children[0]);
    }
    this.scene.add(this.ambientLight);
    this.scene.add(this.frontDirectionalLight);
    this.scene.add(this.backDirectionalLight);
  }

  /**
   * Renders Morse-Smale Decomposition
   * @return {JSX} Morse-Smale JSX component
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
        <canvas ref='msCanvas' style={canvasStyle} />
      </Paper>
    );
  }
}

export default withDSXContext(MorseSmaleWindow);
