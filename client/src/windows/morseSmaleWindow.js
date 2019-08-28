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
   * Create Morse-Smale window object
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.client = this.props.dsxContext.client;

    this.resizeCanvas = this.resizeCanvas.bind(this);
    this.init = this.init.bind(this);
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
  }

  /**
   * Called by react when this component receives new props or context or
   * when the state changes.
   * The data needed to draw the Morse-Smale decomposition it located here.
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
    this.camera.position.z = 1;

    // light
    this.ambientLight = new THREE.AmbientLight( 0x404040 ); // soft white light
    this.frontDirectionalLight = new THREE.DirectionalLight(0xffffff);
    this.frontDirectionalLight.position.set(0, 5, 5);
    this.backDirectionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    this.backDirectionalLight.position.set(0, -5, -5);

    // world
    this.scene = new THREE.Scene();
    this.scene.add(this.ambientLight);
    this.scene.add(this.frontDirectionalLight);
    this.scene.add(this.backDirectionalLight);

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight, false );

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
   * Adds the regression curves to the scene
   * @param {object} regressionData
   */
  addRegressionCurvesToScene(regressionData) {
    regressionData.curves.forEach((regressionCurve) => {
      let curvePoints = [];
      regressionCurve.points.forEach((regressionPoint) => {
        curvePoints.push(new THREE.Vector3(regressionPoint[0], regressionPoint[1], regressionPoint[2]));
      });
      // Create curve
      let curve = new THREE.CatmullRomCurve3(curvePoints);
      let curveGeometry = new THREE.TubeBufferGeometry(curve, 50, .02, 50, false);
      let count = curveGeometry.attributes.position.count;
      curveGeometry.addAttribute('color', new THREE.BufferAttribute(new Float32Array(count * 3), 3));
      let colors = regressionCurve.colors;
      let colorAttribute = curveGeometry.attributes.color;
      let color = new THREE.Color();
      for (let i = 0; i < 52; ++i) {
        color.setRGB(colors[i][0], colors[i][1], colors[i][2]);
        for (let j = 0; j < 50; ++j) {
          colorAttribute.setXYZ(i*50+j, color.r, color.g, color.b);
        }
      }
      let curveMaterial = new THREE.MeshLambertMaterial({
        color: 0xffffff,
        flatShading: true,
        vertexColors: THREE.VertexColors });
      let curveMesh = new THREE.Mesh(curveGeometry, curveMaterial);
      curveMesh.rotateX(-90);
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
      extremaMesh.rotateX(-90);
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
      <Paper style={ paperStyle }>
        <canvas ref='msCanvas' style={canvasStyle} />
      </Paper>
    );
  }
}

export default withDSXContext(MorseSmaleWindow);
