import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import React from 'react';
import ReactResizeDetector from 'react-resize-detector';
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
    this.createCamerasAndControls = this.createCamerasAndControls.bind(this);
    this.updateCamera = this.updateCamera.bind(this);
    this.updatePerspCamera = this.updatePerspCamera.bind(this);
    this.updateOrthoCamera = this.updateOrthoCamera.bind(this);
    this.toggleCamera = this.toggleCamera.bind(this);

    this.handleKeyDownEvent = this.handleKeyDownEvent.bind(this);
    this.handleMouseRelease = this.handleMouseRelease.bind(this);

    this.pick = this.pick.bind(this);
    this.getPickPosition = this.getPickPosition.bind(this);
    this.getCanvasPosition = this.getCanvasPosition.bind(this);

    this.addRegressionCurvesToScene = this.addRegressionCurvesToScene.bind(this);
    this.addExtremaToScene = this.addExtremaToScene.bind(this);

    this.resetScene = this.resetScene.bind(this);
    this.resizeCanvas = this.resizeCanvas.bind(this);
    this.renderScene = this.renderScene.bind(this);
  }

  /**
   * Called by react when this component mounts.
   * Initializes Three.js for drawing and adds event listeners.
   */
  componentDidMount() {
    this.init();
    window.addEventListener('resize', this.resizeCanvas);
    window.addEventListener('keydown', this.handleKeyDownEvent);
    this.refs.msCanvas.addEventListener('mousedown', this.handleMouseRelease, { passive:true });
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
      this.resizeCanvas(null);
    }

    if (prevProps.decomposition === null
      || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)) {
      this.resetScene();
      // object unpacking (a javascript thing, props is inherited from the React component)
      const { datasetId, k, persistenceLevel } = this.props.decomposition;
      const category = this.props.decomposition.decompositionCategory;
      const field = this.props.decomposition.decompositionField;
      Promise.all([
        this.client.fetchMorseSmaleRegression(datasetId, category, field, k, persistenceLevel),
        this.client.fetchMorseSmaleExtrema(datasetId, category, field, k, persistenceLevel),
      ]).then((response) => {
        const [regressionResponse, extremaResponse] = response;
        this.regressionCurves = regressionResponse;
        this.addRegressionCurvesToScene(regressionResponse);
        this.addExtremaToScene(extremaResponse.extrema);
        this.renderScene();

        if (this.pickedObject) {
          let crystalID = this.pickedObject.name;
          this.client.fetchCrystalPartition(datasetId, persistenceLevel, crystalID).then((result) => {
            this.props.onCrystalSelection(result.crystalSamples);
          });
        }
      });
    }
  }

  /**
   * Called by React when this component is removed from the DOM.
   */
  componentWillUnmount() {
    // todo: do we need to dispose and remove event listeners for everything (i.e., a destructor)?
    // this.controls.removeEventListener( 'change', this.renderScene );
    // this.controls.dispose();

    window.removeEventListener('resize', this.resizeCanvas);
    window.removeEventListener('keydown', this.handleKeyDownEvent);
    this.refs.msCanvas.removeEventListener('mousedown', this.handleMouseRelease);
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

    // scene
    this.scene = new THREE.Scene();

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight, false);

    // camera and controls
    this.createCamerasAndControls();

    // light
    this.ambientLight = new THREE.AmbientLight( 0x404040 ); // soft white light
    this.frontDirectionalLight = new THREE.DirectionalLight(0xffffff);
    this.frontDirectionalLight.position.set(5, -1, 5);
    this.backDirectionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    this.backDirectionalLight.position.set(-5, -1, -5);

    // picking
    this.pickedObject = undefined;
    this.raycaster = new THREE.Raycaster();

    // reset and render
    this.resetScene();
    this.renderScene();
  }

  /**
   * Called when the canvas is resized.
   * This can happen on a window resize or when another window is added to dSpaceX.
   * @param {boolean} newWindowAdded
   */
  resizeCanvas(event) {
    let width = this.refs.msCanvas.clientWidth;
    let height = this.refs.msCanvas.clientHeight;

    this.refs.msCanvas.width = width;
    this.refs.msCanvas.height = height;

    // Resize renderer
    this.renderer.setSize(width, height, false);

    // update camera
    this.updateCamera(width, height);

    // Redraw scene with updates
    this.renderScene();
  }

  /**
   * Event handling for mouse click release
   * @param {Event} event
   */
  handleMouseRelease(event) {
    // Handle left click release
    if (event.button === 0) {
      const position = this.getPickPosition(event);
      this.pick(position, event.ctrlKey); // click w/ ctrl held down to produce model's original samples
    }
  }

  /**
   * Handles hotkey events.
   * @param {object} event
   */
  handleKeyDownEvent(event) {
    switch (event.key) {
      case 'v': // toggle orthogonal/perspective camera
        this.toggleCamera();
        break;
      case 'r': // reset view
        this.controls.reset();   // resets camera to original position
        break;
    }
    this.renderScene();
  }

  /**
   * Gets the click coordinates on the canvas - used for selecting crystal
   * @param {object} event
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
    * Converts pixel space to clip space
    * @param {object} event
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
   * @param {boolean} showOrig
   */
  pick(normalizedPosition, showOrig) {
    // Get intersected object
    const { datasetId, decompositionCategory, decompositionField, persistenceLevel } = this.props.decomposition;
    this.raycaster.setFromCamera(normalizedPosition, this.camera);
    let intersectedObjects = this.raycaster.intersectObjects(this.scene.children);
    intersectedObjects = intersectedObjects.filter((io) => io.object.name !== '');
    if (intersectedObjects.length) {
      // Update opacity to signify selected crystal
      if (this.pickedObject) {
        // Make sure have object in current scene
        this.pickedObject = this.scene.getObjectByName(this.pickedObject.name);
        this.pickedObject.material.opacity = 0.75;
        this.pickedObject = undefined;
      }
      this.pickedObject = intersectedObjects[0].object;
      this.pickedObject.material.opacity = 1;
      this.renderScene();

      // Get crystal partitions
      let crystalID = this.pickedObject.name;
      this.props.evalShapeoddsModelForCrystal(datasetId, decompositionCategory, decompositionField, persistenceLevel,
        crystalID, 50 /* numZ*/, showOrig);
      this.client.fetchCrystalPartition(datasetId, persistenceLevel, crystalID).then((result) => {
        this.props.onCrystalSelection(result.crystalSamples);
      });
    }
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
      let opacity = 0.75;
      if (this.pickedObject && parseInt(this.pickedObject.name) === index) {
        opacity = 1.00;
      }
      let curveMaterial = new THREE.MeshLambertMaterial({
        color: 0xffffff,
        flatShading: true,
        vertexColors: THREE.VertexColors,
        transparent: true,
        opacity: opacity,
      });
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

    this.updateCamera(this.refs.msCanvas.width, this.refs.msCanvas.height, true /*resetPos*/);
  }

  /**
   * createCamerasAndControls
   * There are two types of cameras, perspective and orthographic, each with associated controls.
   * NOTE: need to have essentially "the camera for this scene" before creating controls so that controls.reset() works as expected.
   * TODO: z is normalized by Controller to [0,1], but x/y are not normalized, so adjust target when loading new data.
   */
  createCamerasAndControls() {
    let width = this.refs.msCanvas.clientWidth;
    let height = this.refs.msCanvas.clientHeight;

    // orthographic
    this.orthoCamera = new THREE.OrthographicCamera();
    this.orthoCamera.zoom = 2.5;
    this.orthoCamera.position.set(0, -1, 0.5);
    this.orthoCamera.up.set(0, 0, 1);
    this.updateOrthoCamera(width, height);

    this.orthoControls = new OrbitControls(this.orthoCamera, this.renderer.domElement);
    this.orthoControls.enable = false;
    this.orthoControls.screenSpacePanning = true;
    this.orthoControls.minDistance = this.orthoCamera.near;
    this.orthoControls.maxDistance = this.orthoCamera.far;
    this.orthoControls.target0.set(0, 0, 0.5);  // setting target0 since controls.reset() uses this... todo: try controls.saveState 
    this.orthoControls.addEventListener( 'change', this.renderScene );

    // perspective
    const fov = 25;  // narrow field of view so objects don't shrink so much in the distance
    const aspect = width / height;
    const near = 0.001;
    const far = 100;
    this.perspCamera = new THREE.PerspectiveCamera(fov, aspect, near, far);
    this.perspCamera.position.set(0, -6, 0.5);
    this.perspCamera.up.set(0, 0, 1);
		this.perspCamera.add(new THREE.PointLight(0xffffff, 1));
    this.updatePerspCamera(width, height);

    this.perspControls = new OrbitControls(this.perspCamera, this.renderer.domElement);
    this.perspControls.enable = false;
    this.perspControls.screenSpacePanning = true;
    this.perspControls.minDistance = this.perspCamera.near;
    this.perspControls.maxDistance = this.perspCamera.far;
    this.perspControls.target0.set(0, 0, 0.5);  // setting target0 since controls.reset() uses this... todo: try controls.saveState 
    this.perspControls.addEventListener( 'change', this.renderScene );

    // set default to perspective
    this.camera = this.perspCamera;
    this.controls = this.perspControls;
    this.controls.enable = true;
  }

  /**
   * toggleCamera
   * Toggles between orthographic and perspective cameras.
   */
  toggleCamera() {
    this.controls.enable = false;
    if (this.camera === this.orthoCamera) {
      this.camera = this.perspCamera;
      this.controls = this.perspControls;
    }
    else {
      this.camera = this.orthoCamera;
      this.controls = this.orthoControls;
    }
    this.controls.enable = true;
    this.controls.reset();
  }
  
  /**
   * updateOrthoCamera
   */
  updateOrthoCamera(width, height) {
    let sx = 1;
    let sy = 1;
    if (width > height) {
      sx = width/height;
    } else {
      sy = height/width;
    }
    this.orthoCamera.left   = -4*sx;
    this.orthoCamera.right  = 4*sx;
    this.orthoCamera.top    = 4*sy;
    this.orthoCamera.bottom = -4*sy;
    this.orthoCamera.near   = -16;
    this.orthoCamera.far    = 16;
  }
  
  /**
   * updatePerspCamera
   */
  updatePerspCamera(width, height) {
    this.perspCamera.aspect = width / height;
  }
  
  /**
   * updateCamera
   */
  updateCamera(width, height, resetPos = false) {
    this.updateOrthoCamera(width, height);
    this.updatePerspCamera(width, height);

    if (resetPos) {
      this.controls.reset();   // resets camera to original position (also calls updateProjectionMatrix)
    }
    else {
      this.camera.updateProjectionMatrix();
      this.controls.update();  // it's necessary to call this when the camera is manually changed
    }
  }

  /**
   * Renders Morse-Smale Decomposition
   * @return {JSX} Morse-Smale JSX component
   */
  render() {
    let style = {
      width: '100%',
      height: '100%',
    };

    return (
      <ReactResizeDetector handleWidth handleHeight onResize={() => this.resizeCanvas(null)}>
        <canvas ref='msCanvas' style={style} />
      </ReactResizeDetector>);
  }
}

export default withDSXContext(MorseSmaleWindow);
