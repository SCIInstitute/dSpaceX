import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { SpriteMaterial } from 'three/src/materials/SpriteMaterial.js';
import { OutlineEffect } from 'three/examples/jsm/effects/OutlineEffect.js';
import React from 'react';
import { withDSXContext } from '../dsxContext';
import _ from 'lodash';

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
    this.numInterpolants = 50; // how many samples to generate using current model [to fill drawer]

    this.state = {
      drawerAdded: false,               // when parent component adds a drawer, resize isn't called, so force it
      selectingCrystal: false,
      percent: 0.5,
      evalModel: { inProgress: false,   // dragging along a crystal
                   next: undefined,     // { crystalID, percent }
                   sprites: undefined,  // { nearestLesser, interpolated, nearestGreater }
                 },
    };

    this.client = this.props.dsxContext.client;

    this.init = this.init.bind(this);
    this.createCamerasAndControls = this.createCamerasAndControls.bind(this);
    this.updateCamera = this.updateCamera.bind(this);
    this.updatePerspCamera = this.updatePerspCamera.bind(this);
    this.updateOrthoCamera = this.updateOrthoCamera.bind(this);
    this.toggleCamera = this.toggleCamera.bind(this);

    this.handleKeyDownEvent = this.handleKeyDownEvent.bind(this);
    this.onMouseDown = this.onMouseDown.bind(this);
    this.onMouseMove = this.onMouseMove.bind(this);
    this.onMouseUp = this.onMouseUp.bind(this);

    this.pick = this.pick.bind(this);
    this.getPickPosition = this.getPickPosition.bind(this);
    this.getCanvasPosition = this.getCanvasPosition.bind(this);

    this.addRegressionCurvesToScene = this.addRegressionCurvesToScene.bind(this);
    this.addExtremaToScene = this.addExtremaToScene.bind(this);

    this.resetScene = this.resetScene.bind(this);
    this.resetBounds = this.resetBounds.bind(this);
    this.renderScene = this.renderScene.bind(this);

    this.addSphere = this.addSphere.bind(this);
    this.setImage = this.setImage.bind(this);
  }
  
  /**
   * Called by react when this component mounts.
   * Initializes Three.js for drawing and adds event listeners.
   */
  componentDidMount() {
    this.init();
    window.addEventListener('resize', _.debounce(this.resizeCanvas, 200));
    window.addEventListener('keydown', this.handleKeyDownEvent);
    this.refs.msCanvas.addEventListener('mousedown', this.onMouseDown);
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
        if (!regressionResponse.error && !extremaResponse.error) {
          this.regressionCurves = regressionResponse;
          this.addRegressionCurvesToScene(regressionResponse);
          this.addExtremaToScene(extremaResponse.extrema);
          this.renderScene();
        }
        else {
          console.log('morseSmaleWindow.componentDidUpdate error:\n\t regressionResponse: '
                      +regressionResponse.error_msg+'\n\t extremaResponse: '+extremaResponse.error_msg);
        }
      });
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
    // todo: do we need to dispose and remove event listeners for everything (i.e., a destructor)?
    // this.controls.removeEventListener( 'change', this.renderScene );
    // this.controls.dispose();

    window.removeEventListener('resize', this.resizeCanvas);
    window.removeEventListener('keydown', this.handleKeyDownEvent);
    this.refs.msCanvas.removeEventListener('mousedown', this.onMouseDown);
  }

  /**
   * mousedown starts a potential crystal select
   */
  onMouseDown(event) {
    this.refs.msCanvas.addEventListener('mousemove', this.onMouseMove);
    this.refs.msCanvas.addEventListener('mouseup', this.onMouseUp);

    // // Handle left click
    // if (this.state.selectingCrystal && event.button === 0) {
    //   return this.pick(this.getPickPosition(event),
    //             event.ctrlKey && !event.shiftKey /*showOrig*/,
    //             event.ctrlKey && event.shiftKey /*validate*/);
    // }

    // if left button clicked start a potential crystal selection action
    if (event.button === 0)
      this.setState({ selectingCrystal:true });
  }
  
  /**
   * mousemove ends a potential crystal so rotations don't accidentally select a crystal
   */
  onMouseMove(event) {
    this.setState({ selectingCrystal:false });

    if (this.continuousInterpolation) {
      this.handleContinuousInterpolationMouseMove(this.getPickPosition(event));
      return true; // no more percolation of this mouse move event (doesn't work, instead control.disable is probably what we need)
    }
    else {
      // can release these event handlers now since orbit controls handles camera
      this.refs.msCanvas.removeEventListener('mousemove', this.onMouseMove);
      this.refs.msCanvas.removeEventListener('mouseup', this.onMouseUp);
    }
  }

  /**
   * mouseup potentially selects a crystal
   */
  onMouseUp(event) {
    this.refs.msCanvas.removeEventListener('mousemove', this.onMouseMove);
    this.refs.msCanvas.removeEventListener('mouseup', this.onMouseUp);

    // Handle left click release
    if (this.state.selectingCrystal && event.button === 0) {
      this.pick(this.getPickPosition(event),
                event.ctrlKey && !event.shiftKey /*showOrig*/,
                event.ctrlKey && event.shiftKey /*validate*/);
    }
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
    //<ctc> debugging vars
    this.nResizes = 0;

    // canvas
    let canvas = this.refs.msCanvas;
    let gl = canvas.getContext('webgl');
    let width = canvas.clientWidth, height = canvas.clientHeight;

    // scene
    this.scene = new THREE.Scene();
    //this.scene.background = new THREE.Color('whatever'); // do NOT set scene background or outline selection won't work

    // UI
    this.createUI();

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
  resizeCanvas = () => {
    let width = this.refs.msCanvas.clientWidth, height = this.refs.msCanvas.clientHeight;
    //console.log('['+ this.nResizes++ +'] morseSmaleWindow resizing canvas from '+this.refs.msCanvas.width+' x '+this.refs.msCanvas.height+' to '+width+' x '+height);

    // update camera
    this.updateCamera(width, height);

    // resize renderer
    this.renderer.setSize(width, height, false);

    // Redraw scene with updates
    this.renderScene();
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
    case 'x': // continuous interpolation
      this.continuousInterpolation = !this.continuousInterpolation;
      if (this.continuousInterpolation) {
        console.log("continuous interpolation (press 'x' again to disable)");
        this.controls.enabled = false;
      }
      else {
        console.log("continuous interpolation disabled");
        this.controls.enabled = true;
      }
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
  getPickPosition(event) {  //todo: maybe rename to getMousePosition
    const pos = this.getCanvasPosition(event);
    const canvas = this.refs.msCanvas;
    return {
      x: (pos.x / canvas.clientWidth) * 2 - 1,
      y: (pos.y / canvas.clientHeight) * -2 + 1,
    };
  }

  /**
   * Handle mouse move for continuous interpolation
   */
  handleContinuousInterpolationMouseMove(position) {
    //let percent = Math.max(Math.min(position.y - this.startInterpolation, 100.0), 0.0);
    let percent = Math.max(Math.min(position.y/2.0 + 0.5, 1.0), 0.0);  //todo: crazy cheap shot right now
    console.log("percent = " + percent);    
    this.evalModel(percent);
  }

  /*
   * set the material for the current image, hiding image if no data
   * @param {data} raw base64 [png] image data
   */
  setImage(data) {
    if (data !== undefined) {
      
      // load the new image sent from the server
      this.textureLoader.load(
	      'data:image/png;base64,' + data.img.rawData, function(texture) {
          this.imageSprite.material.map = texture;
          this.imageSprite.visible = true;
          this.imageSprite.material.needsUpdate = true;
          this.imageSprite.scale.x = this.spriteScale;
          this.imageSprite.scale.y = texture.image.height / texture.image.width * this.spriteScale;
          this.renderScene();
        }.bind(this));
    }
    else {
      this.imageSprite.visible = false;
    }
  }
  
  // get a single sample at the given percent along the selected crystal
  // todo:
  // - if no model, hide sprites
  // - show adjacent sprites at field's value closest <= and >= to this percent along crystal
  async evalModel(percent) {
    console.log("evalModel("+percent+")");
    let crystalID = this.pickedCrystal.name;
    if (this.state.evalModel.inProgress) {
      this.state.evalModel.next = { crystalID, percent };
      console.log("evalModel in progress, setting next to("+percent+")");
    }
    else {
      this.state.evalModel.inProgress = true;
      this.props.evalModelForCrystal(crystalID, 1 /*numSamples*/, false /*showOrig*/, false /*validate*/, percent).then((image) => {
        console.log("evalModel("+percent+") complete! setting image");
        this.setImage(image);
        this.state.evalModel.inProgress = false;
        if (this.state.evalModel.next !== undefined) {
          console.log("evalModel.next exists, calling that now(id: "+this.state.evalModel.next.crystalID+", pct: "+this.state.evalModel.next.percent+")");
          this.evalModel(this.state.evalModel.next.percent);
          this.state.evalModel.next = undefined;
        }
        this.renderScene();
      });
    }
  }
  
  /**
   * Pick level set of decomposition
   * @param {object} normalizedPosition
   * @param {boolean} showOrig
   * returns true if slider on crystal was selected so the... crap. anyway, gonna think about it.
   */
  pick(normalizedPosition, showOrig, validate) {
    if (this.props.decomposition === null) {
      return;// true; // nothing to pick and no need to propagate this event
    }

    // Get intersected object
    const { datasetId, persistenceLevel } = this.props.decomposition;
    this.raycaster.setFromCamera(normalizedPosition, this.camera);
    let intersectedObjects = this.raycaster.intersectObjects(this.scene.children);
    intersectedObjects = intersectedObjects.filter((io) => io.object.name !== '');

    if (intersectedObjects.length) {
      let crystalID = intersectedObjects[0].object.name;
      let percent = 0.5; // TODO where we are along selected crystal
      if (intersectedObjects[0].object === this.crystalPosObject || intersectedObjects[0].object === this.pickedCrystal) {
        // We picked the active pointer along a curve, or the already selected
        // curve, so update its position along curve. NOTE: this function is
        // called on mouseUp, so interactive interpolation is still disabled.
        console.log('You clicked the crystalPosObject or the already selected crystal');
        crystalID = this.pickedCrystal.name;
        percent = 0.15; // TODO where we are along selected crystal
        // <ctc> look at normalizedPosition to see if it might make sense to use
      }
      else {
        // New crystal selected.

        // ensure previously selected object is back to unselected opacity
        if (this.pickedCrystal) {
          this.pickedCrystal.material.opacity = this.unselectedOpacity;
        }

        this.pickedCrystal = intersectedObjects[0].object;
        this.pickedCrystal.material.opacity = this.selectedOpacity;

        console.log('New crystal selected (' + crystalID + ')');

        // highlight this crystal's samples in the other views (e.g., embedding window)
        this.client.fetchCrystalPartition(datasetId, persistenceLevel, crystalID).then((result) => {
          this.props.onCrystalSelection(result.crystalSamples);
        });

        // evaluate the current model for this crystal to fill the drawer (parent component updates drawer)
        this.props.evalModelForCrystal(crystalID, this.numInterpolants, showOrig, validate, this.state.evalModel.percent);
      }

      // TODO: add a clickable plane (crystal pos object) perpendicular to the
      // curve (using curve.getTangent(u)) at selected position along this
      // crystal (just another sphere for now).

      // NOTE: may need to save the catmull rom curves for each crystal in
      // order to move these along the curve.
      //this.crystalPosObject = this.addSphere(curve.getPoint(0.5), new THREE.Color('darkorange'));
      // <ctc> need a curve. is the selected object that abstract, or just a triangle?

      // add sprite for model evaluation at selected point along crystal
      this.evalModel(percent);

      //this.renderScene();  //<ctc> this works without needing to render here! (but do need to in setImage above)

      //return true; // tell caller something was picked so event propagation can be stopped (avoiding undesired rotation)
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
      // Use midpoint as curve position to ensure transparency sorting has better odds of working,
      // since conflicts arise when using first or last point, bounding box, etc.
      let numPts = rCurve.points.length;
      let midPoint = new THREE.Vector3(rCurve.points[Math.floor(numPts/2)][0],
                                       rCurve.points[Math.floor(numPts/2)][1],
                                       rCurve.points[Math.floor(numPts/2)][2]);

      let curvePoints = [];
      rCurve.points.forEach((regressionPoint) => {
        let P = new THREE.Vector3(regressionPoint[0], regressionPoint[1], regressionPoint[2]);
        curvePoints.push(P.sub(midPoint));
      });
      let cr_curve = new THREE.CatmullRomCurve3(curvePoints);

      let curveColors = [];
      rCurve.colors.forEach((regressionColor) => {
        let C = new THREE.Vector3(regressionColor[0], regressionColor[1], regressionColor[2]);
        curveColors.push(C);
      });
      let cr_color = new THREE.CatmullRomCurve3(curveColors, false, "centripetal", 0.9); // match colors by using high tension

      // Create curve
      let extrusionSegments = 350;
      let colors = cr_color.getPoints(extrusionSegments); // "interpolate" colors
      let radialSegments = 10;
      let radius = 0.02;  // TODO: should be relative to scene size (which needs adjusting as noted in a github issue #158)
      let curveGeometry = new THREE.TubeBufferGeometry(cr_curve, extrusionSegments, radius, radialSegments, false /*closed curve*/);
      let count = curveGeometry.attributes.position.count; // geom size; we need to set a color per point in the geometry
      curveGeometry.setAttribute('color', new THREE.BufferAttribute(new Float32Array(count * 3), 3));
      let colorAttribute = curveGeometry.attributes.color;
      
      // set colors and material
      let color = new THREE.Color;
      for (let i = 0; i <= extrusionSegments; ++i) {
        color.setRGB(colors[i].x, colors[i].y, colors[i].z);
        for (let r = 0; r <= radialSegments; r++)
          colorAttribute.setXYZ(i*(radialSegments+1) + r, color.r, color.g, color.b);
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

  createUI() {
    // uiScene
    this.uiScene = new THREE.Scene();

    // uiCamera
    this.uiCamera = new THREE.OrthographicCamera( - 1, 1, 1, - 1, 1, 2 );
    this.uiCamera.position.set( 0, 0, 1 );
    this.uiCamera.name = "uiCamera";

    // instantiate a loader and sprites
    this.textureLoader = new THREE.TextureLoader();
    
    // imageSprite
    this.spriteScale = 0.25;
    this.imageSprite = new THREE.Sprite();
    this.imageSprite.position.set(0,-0.85,0); // position of center of sprite
    this.imageSprite.visible = false;
    this.uiScene.add(this.imageSprite);

    // for continuous interpolation of a selected crystal
    this.continuousInterpolation = false;
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
    const orthoPosition = new THREE.Vector3(0, -1, 0.5);
    this.orthoCamera = new THREE.OrthographicCamera();
    this.orthoCamera.zoom = 2.5;
    this.orthoCamera.position.copy(orthoPosition);
    this.orthoCamera.up.set(0, 0, 1);
    this.orthoCamera.add(new THREE.PointLight(0xffffff));
    this.orthoCamera.name = "camera";
    this.updateOrthoCamera(width, height);

    this.orthoControls = new OrbitControls(this.orthoCamera, this.renderer.domElement);
    this.orthoControls.enabled = false;
    this.orthoControls.screenSpacePanning = true;
    this.orthoControls.minDistance = this.orthoCamera.near;
    this.orthoControls.maxDistance = this.orthoCamera.far;
    this.orthoControls.target0.set(0, 0, 0.5);  // setting position0 and target0 since controls.reset() uses these
    this.orthoControls.position0.copy(orthoPosition);
    this.orthoControls.addEventListener( 'change', this.renderScene );

    // perspective
    const fov = 25;  // narrow field of view so objects don't shrink so much in the distance
    const aspect = width / height;
    const near = 0.001;
    const far = 100;
    let perspPosition = new THREE.Vector3(0, -6, 0.5);
    this.perspCamera = new THREE.PerspectiveCamera(fov, aspect, near, far);
    this.perspCamera.position.copy(perspPosition);
    this.perspCamera.up.set(0, 0, 1);
    this.perspCamera.add(new THREE.PointLight(0xffffff));
    this.perspCamera.name = "camera";
    this.updatePerspCamera(width, height);
    
    this.perspControls = new OrbitControls(this.perspCamera, this.renderer.domElement);
    this.perspControls.enabled = false;
    this.perspControls.screenSpacePanning = true;
    this.perspControls.minDistance = this.perspCamera.near;
    this.perspControls.maxDistance = this.perspCamera.far;
    this.perspControls.target0.set(0, 0, 0.5);  // setting position0 and target0 since controls.reset() uses these
    this.perspControls.position0.copy(perspPosition);
    this.perspControls.addEventListener( 'change', this.renderScene );

    // set default to perspective
    this.camera = this.perspCamera;
    this.controls = this.perspControls;
    this.controls.enabled = true;
  }

  /**
   * toggleCamera
   * Toggles between orthographic and perspective cameras.
   */
  toggleCamera() {
    this.controls.enabled = false;
    if (this.camera === this.orthoCamera) {
      this.camera = this.perspCamera;
      this.controls = this.perspControls;
    }
    else {
      this.camera = this.orthoCamera;
      this.controls = this.orthoControls;
    }
    this.controls.enabled = true;
    this.controls.reset();
  }
  
  /**
   * updateOrthoCamera
   */
  updateOrthoCamera(width, height) {
    let sx = width / height, sy = 1;
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
   * Resets the scene when there is new data by removing
   * the old scene children and adding back the lights.
   */
  resetScene() {
    while (this.scene.children.length > 0) {
      this.scene.remove(this.scene.children[0]);
    }
    this.scene.add(this.camera);        // camera MUST be added to scene for it's light to work
    this.scene.add(this.ambientLight);

    this.pickedCrystal = undefined;
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
    if (this.pickedCrystal !== undefined) {
      // hide everything but selected object for outline
      for (var item of scene) {
        item.visible = (item === this.pickedCrystal);
      }
      this.outline.renderOutline(this.scene, this.camera);

      // now show everything
      for (var item of scene) {
        item.visible = true;
      }
    }

    // ...and finally, the UI
    this.renderer.render(this.uiScene, this.uiCamera);
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
        <canvas ref='msCanvas' style={style} />);
  }
}

export default withDSXContext(MorseSmaleWindow);
