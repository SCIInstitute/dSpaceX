import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';

import { OutlineEffect } from 'three/examples/jsm/effects/OutlineEffect.js';
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

    this.selectedOpacity = 1.0;
    this.unselectedOpacity = 0.80;
    this.numInterpolants = 51; // how many samples to generate using current model [to fill drawer]

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
    this.resetBounds = this.resetBounds.bind(this);
    this.resizeCanvas = this.resizeCanvas.bind(this);
    this.renderScene = this.renderScene.bind(this);

    this.addSphere = this.addSphere.bind(this);
  }
  
  /**
   * Called by react when this component mounts.
   * Initializes Three.js for drawing and adds event listeners.
   */
  componentDidMount() {
    this.init();
    window.addEventListener('resize', this.resizeCanvas);
    window.addEventListener('keydown', this.handleKeyDownEvent);
    this.refs.msCanvas.addEventListener('mousedown', this.handleMouseRelease, { passive:true }); // todo: selection/rotation conflict
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
   * Scene bounds, usually [0,1] in z and whatever the crystals are in x and y.
   */
  resetBounds() {
    this.bounds = {
      minX: -1,
      maxX: 1,
      minY: -1,
      maxY: 1,
      minZ: -0.5,
      maxZ: 1.5
    };
  }

  /**
   * Initializes the renderer, camera, and scene for Three.js.
   */
  init() {
    // canvas
    let canvas = this.refs.msCanvas;
    let gl = canvas.getContext('webgl');
    let width = canvas.clientWidth, height = canvas.clientHeight;

    // scene
    this.scene = new THREE.Scene();
    //this.scene.background = new THREE.Color('whatever'); // do NOT set scene background or outline selection won't work

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl, antialias:true });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight, false);
    this.renderer.autoClear = false; // clear scene manually in renderScene

    const outline = new OutlineEffect(this.renderer, {
      defaultThickness: 0.007,
      defaultColor: [0, 1, 1],
      defaultAlpha: 1.0,
      defaultKeepAlive: false // keeps outline material in cache even if material is removed from scene (no need for us)
    });
    this.outline = outline;

    // camera and controls
    this.createCamerasAndControls();

    // Bounds
    this.resetBounds();  //TODO need to set bounds

    // lights
    this.ambientLight = new THREE.AmbientLight(0xcccccc, 0.5); // soft white light

    // picking
    this.raycaster = new THREE.Raycaster();

    // reset and render
    this.resetScene();
  }

  /**
   * Called when the canvas is resized.
   * This can happen on a window resize or when another window is added to dSpaceX.
   * @param {boolean} newWindowAdded
   */
  resizeCanvas(event) {
    let width = this.refs.msCanvas.clientWidth;
    let height = this.refs.msCanvas.clientHeight;

    // update camera
    this.updateCamera(width, height);

    // resize renderer
    this.renderer.setSize(width, height, false);

    // Redraw scene with updates
    this.renderScene();
  }

  /**
   * Event handling for mouse click
   * @param {Event} event
   *
   * // todo: selection/rotation conflict -> should listen for mouseup, but currently tied to mousedown.
   */
  handleMouseRelease(event) {
    // Handle left click release
    if (event.button === 0) {
      const position = this.getPickPosition(event);
      if (this.pick(position, event.ctrlKey)) // click w/ ctrl held down to produce model's original samples
        event.stopPropagation(); // release the event if this picks something // todo: doesn't seem to work as event still goes to controller
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
    case '+': //increase numInterpolants
    case '=': //increase numInterpolants
      this.numInterpolants++;
      console.log("numInterpolants increased to " + this.numInterpolants)
      break;
    case '-': //decrease numInterpolants
      this.numInterpolants = Math.max(1, this.numInterpolants - 1);
      console.log("numInterpolants decreased to " + this.numInterpolants)
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
    if (this.props.decomposition === null) {
      return;
    }

    // Get intersected object
    const { datasetId, decompositionCategory, decompositionField, persistenceLevel } = this.props.decomposition;
    this.raycaster.setFromCamera(normalizedPosition, this.camera);
    let intersectedObjects = this.raycaster.intersectObjects(this.scene.children);
    intersectedObjects = intersectedObjects.filter((io) => io.object.name !== '');
    if (intersectedObjects.length) {
      if (intersectedObjects[0].object === this.crystalPosObject) {
        // we picked the active pointer along a curve, so turn on dragging of the pointer
        //this.draggingCrystalPos = true;  // TODO
        console.log('You clicked the crystalPosObject!');

      }
      else if (intersectedObjects[0].object === this.pickedObject) {
        console.log('You clicked the already selected object');

        // TODO: (maybe) move crystal pos object to selected position along this crystal
      }
      else {
        console.log('New crystal selected');

        // Ensure previously selected object is back to unselected opacity
        if (this.pickedObject) {
          this.pickedObject.material.opacity = this.unselectedOpacity;
        }

        this.pickedObject = intersectedObjects[0].object;
        this.pickedObject.material.opacity = this.selectedOpacity;

        // add a clickable plane perpendicular to the curve (using curve.getTangent(u))... or just another sphere for now
        // Hmm... maybe create one of these for each crystal? Then they can remember their positions per crystal.
        //        may need to save the catmull rom curves for each crystal in order to move these along the curve.
        //this.crystalPosObject = this.addSphere(curve.getPoint(0.5), new THREE.Color('darkorange'));

        //...or start with a simple sphere
        //   // add a clickable sphere along the curve
        //   this.addSphere(curve.getPoint(0.5), THREE.Color('darkorange'));
      }

      this.renderScene();

      // Get crystal partitions
      let crystalID = this.pickedObject.name;
      this.props.evalShapeoddsModelForCrystal(datasetId, decompositionCategory, decompositionField, persistenceLevel,
        crystalID, this.numInterpolants, showOrig);
      this.client.fetchCrystalPartition(datasetId, persistenceLevel, crystalID).then((result) => {
        this.props.onCrystalSelection(result.crystalSamples);
      });

      return true; // tell caller something was picked so event propagation can be stopped (avoiding undesired rotation)
    }

    return false;
  }

  /**
   * Adds a sphere to the scene
   * @param {vector3} pos position
   * @param {vector3} color THREE.Color object
   * @param {float} radius
   * returns object added to scene
   */
  addSphere(pos, color, radius = 0.05) {
    let geometry = new THREE.SphereBufferGeometry(radius, 32, 32);
    let material = new THREE.MeshStandardMaterial({ color:color });
    let mesh = new THREE.Mesh(geometry, material);
    let dist = pos.length();
    let dir = pos.clone().normalize();
    mesh.translateOnAxis(dir, dist);
    this.scene.add(mesh);
    return mesh;
  }

  /**
   * Adds a plane to the scene
   * @param {vector3} pos position
   * @param {vector3} normal position
   * @param {vector3} color rgb color
   * @param {float} size xy dims
   * returns object added to scene
   */
  addPlane( {position = [0,0,0], normal = [0,0,1], size = [1,1], color = null} ) {
    if (!color)
      color = new THREE.Color('brown');
    let geometry = new THREE.PlaneBufferGeometry(size[0], size[1]);
    let material = new THREE.MeshStandardMaterial({ color:color });
    let mesh = new THREE.Mesh(geometry, material);
    let translation = position.length();
    let translation_dir = position.clone().normalize();
    let up_vec = new THREE.Vector3(0,0,1);
    let perp_vec = normal.clone().cross(up_vec);
    let eps = 0.001;
    if (perp_vec.length() > eps) {
      mesh.rotate(perp_vec, up_vec.angleTo(normal));
    }
    mesh.translateOnAxis(translation_dir, translation);
    this.scene.add(mesh);
    return mesh;
  }

  /**
   * Adds a box to the scene
   * @param {vector3} pos position
   * @param {vector3} normal position
   * @param {vector3} color rgb color
   * @param {float} size xy dims
   * returns object added to scene
   */
/* TODO for crystal slider
  addBox(pos = [0,0,0], normal = [0,0,1], size = [1,1], color = null) {
    if (!color)
      color = new THREE.Color('brown');
    let geometry = new THREE.BoxBufferGeometry(size[0], size[1]);
    let material = new THREE.MeshStandardMaterial({ color:color });
    let mesh = new THREE.Mesh(geometry, material);
    let dist = pos.length();
    let dir = pos.clone().normalize();
    let up = THREE.Vector3(0,1,0);
    let perp = dir.clone().cross(up);
    let eps = 0.001;
    if (perp.length() > eps) {
      mesh.rotate(perp, up.angleTo(dir));
    }
    mesh.translate(dir, dist);
    this.scene.add(mesh);
    return mesh;
  }
*/
  
  /**
   * Adds the regression curves to the scene
   * @param {object} regressionData
   */
  addRegressionCurvesToScene(regressionData) {
    regressionData.curves.forEach((rCurve, index) => {
      let numPts = rCurve.points.length;

      // Use this as curve position to ensure transparency sorting has better odds of working.
      // NOTE: conflicts arise when using first or last point, bounding box, the most extreme point, etc, so midpoint is the compromise.
      let midPoint = new THREE.Vector3(rCurve.points[numPts/2][0], rCurve.points[numPts/2][1], rCurve.points[numPts/2][2]);

      let curvePoints = [];
      rCurve.points.forEach((regressionPoint) => {
        let P = new THREE.Vector3(regressionPoint[0], regressionPoint[1], regressionPoint[2]);
        curvePoints.push(P.sub(midPoint));
      });

      // Create curve
      let curve = new THREE.CatmullRomCurve3(curvePoints);
      let segmentsPerPoint = 6;
      let radialSegments = 10;
      let curveGeometry = new THREE.TubeBufferGeometry(curve, (curvePoints.length-1) * (segmentsPerPoint-1) /*tubularSegments*/,
                                                       .02 /*radius*/, radialSegments-1, false /*closed curve*/);
      let count = curveGeometry.attributes.position.count;
      curveGeometry.setAttribute('color', new THREE.BufferAttribute(new Float32Array(count * 3), 3));
      let colors = rCurve.colors, colorAttribute = curveGeometry.attributes.color;
      
      // set colors for first point
      let c1 = new THREE.Color(colors[0][0], colors[0][1], colors[0][2]);
      for (let r = 0; r < radialSegments; r++) {
        colorAttribute.setXYZ(r, c1.r, c1.g, c1.b);
      }
      // set colors for the rest
      let c2 = new THREE.Color;
      let color = new THREE.Color;
      for (let i = 0; i < curvePoints.length-1; ++i) {
        c2.setRGB(colors[i+1][0], colors[i+1][1], colors[i+1][2]);
        for (let j = 1; j < segmentsPerPoint+1; ++j) {
          color = c1.lerp(c2, 1.0/segmentsPerPoint * j);
          for (let r = 0; r < radialSegments; r++)
            colorAttribute.setXYZ(i*segmentsPerPoint*radialSegments + j*radialSegments + r, color.r, color.g, color.b);
        }
        c1.copy(c2);
      }

      let curveMaterial = new THREE.MeshStandardMaterial({
        vertexColors: true,
        transparent: true,
        opacity: this.unselectedOpacity,
      });
      let curveMesh = new THREE.Mesh(curveGeometry, curveMaterial);
      curveMesh.name = index;

      // translate to location of middle point of curve, ensuring all curves are [more likely to be] different distances from camera,
      // since default sort only uses their position, not any of their points, their bounding box, etc
      let dist = midPoint.length();
      let dir = midPoint.clone().normalize();
      curveMesh.translateOnAxis(dir, dist);

      this.scene.add(curveMesh);
    });
  }

  /**
   * Adds the extrema to the scene.
   * @param {object} extrema
   */
  addExtremaToScene(extrema) {
    extrema.forEach((extreme) => {
      let position = new THREE.Vector3().fromArray(extreme.position);
      //todo: scale radius relative to this.bounds; ~0.05 (default) is good for bounds of size 1, 1, 1.
      this.addSphere(position, new THREE.Color(extreme.color[0], extreme.color[1], extreme.color[2]));
    });
  }

  /**
   * createCamerasAndControls
   * There are two types of cameras, perspective and orthographic, each with associated controls.
   * NOTE: need to have essentially "the camera for this scene" before creating controls so that controls.reset() works as expected.
   * TODO: z is normalized by Controller to [0,1], but x/y are not normalized, so adjust target when loading new data.
   */
  createCamerasAndControls() {
    let width = this.refs.msCanvas.clientWidth, height = this.refs.msCanvas.clientHeight;

    // orthographic
    this.orthoCamera = new THREE.OrthographicCamera();
    this.orthoCamera.zoom = 2.5;
    this.orthoCamera.position.set(0, -1, 0.5);
    this.orthoCamera.up.set(0, 0, 1);
    this.orthoCamera.add(new THREE.PointLight(0xffffff, 1));
    this.orthoCamera.name = "camera";
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
    this.perspCamera.name = "camera";
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
    let sx = 1, sy = 1;
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
      this.controls.reset(); // resets camera to original position (also calls updateProjectionMatrix)
    } else {
      this.camera.updateProjectionMatrix();
      this.controls.update(); // it's necessary to call this when the camera is manually changed
    }
  }

  /**
   * Resets the scene when there is new data by removing
   * the old scene children and adding back the lights.
   */
  resetScene() {
    while (this.scene.children.length > 0) {
      this.scene.remove(this.scene.children[0]);
    }
    this.scene.add(this.camera);        // camera MUST be added to scene for it's light to work
    this.scene.add(this.ambientLight);

    this.pickedObject = undefined;
    this.crystalPosObject = undefined;

    this.updateCamera(this.refs.msCanvas.width, this.refs.msCanvas.height, true /*resetPos*/);
  }

  /**
   * Draws the scene to the canvas.
   */
  renderScene() {
    var scene = Object.values(this.scene.children);
    this.renderer.clear();

    // render first... 
    this.renderer.render(this.scene, this.camera);

    // ...then outline
    if (this.pickedObject !== undefined) {
      // hide everything but selected object for outline
      for (var item of scene) {
        item.visible = (item === this.pickedObject);
      }
      this.outline.renderOutline(this.scene, this.camera);

      // now show everything
      for (var item of scene) {
        item.visible = true;
      }
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
