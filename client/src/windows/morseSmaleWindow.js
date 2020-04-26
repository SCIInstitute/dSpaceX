import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { EffectComposer } from 'three/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/examples/jsm/postprocessing/RenderPass.js';
import { OutlinePass } from 'three/examples/jsm/postprocessing/OutlinePass.js';
import { ShaderPass } from 'three/examples/jsm/postprocessing/ShaderPass.js';
import { FXAAShader } from 'three/examples/jsm/shaders/FXAAShader.js';
import Paper from '@material-ui/core/Paper';
import React from 'react';
import ReactResizeDetector from 'react-resize-detector';
import { withDSXContext } from '../dsxContext';
import { RectAreaLightUniformsLib } from 'three/examples/jsm/lights/RectAreaLightUniformsLib.js';
import { RectAreaLightHelper } from 'three/examples/jsm/helpers/RectAreaLightHelper.js';

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
    this.createComposer = this.createComposer.bind(this); // <ctc> this gets called even when not bound. Necessary to bind it?
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
    this.addLights = this.addLights.bind(this);
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
          // TODO: <ctc> verify this is ever executed since it's not clear how there could already be a picked curve w/ a new decomposition
          console.log('Huh?? How is there a picked object with this new decomposition?');
          // ...even if there is, it probably should have been set to undefined when the new decomposition was selected.

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
   * create lights
   */
  initLights() {
    RectAreaLightUniformsLib.init();
    this.lights = new Object;

    this.lights.rectLightTop = new THREE.RectAreaLight( 0xffffff, 1, 10, 10 );
    this.lights.rectLightTop.position.set( 0, 0, this.bounds.maxZ ); // max height of crystal models is 1
    this.lights.rectLightTop.lookAt(0, 0, this.bounds.minZ);
    // this.lights.rectLightHelperTop = new RectAreaLightHelper( this.lights.rectLightTop );
    // this.lights.rectLightTop.add( this.lights.rectLightHelperTop );

    this.lights.rectLightBot = new THREE.RectAreaLight( 0xffffff, 0.5, 10, 10 );
    this.lights.rectLightBot.position.set( 0, 0, this.bounds.minZ ); // min height of crystal models is 0
    this.lights.rectLightBot.lookAt(0, 0, this.bounds.maxZ);
    this.lights.rectLightHelperBot = new RectAreaLightHelper( this.lights.rectLightBot );
    //this.lights.rectLightBot.add( this.lights.rectLightHelperBot );

    this.lights.rectLightFront = new THREE.RectAreaLight( 0xffffff, 1, 10, 10 );
    this.lights.rectLightFront.position.set( 0, this.bounds.minY, 0 ); // max width/depth of crystal models varies (TODO: update when crystals are added)
    this.lights.rectLightFront.lookAt(0, this.bounds.maxY, 0);
    // this.lights.rectLightHelperFront = new RectAreaLightHelper( this.lights.rectLightFront );
    // this.lights.rectLightFront.add( this.lights.rectLightHelperFront );

    this.lights.rectLightBack = new THREE.RectAreaLight( 0xffffff, 0.5, 10, 10 );
    this.lights.rectLightBack.position.set( 0, this.bounds.maxY, 0 );
    this.lights.rectLightBack.lookAt(0, this.bounds.minY, 0);
    this.lights.rectLightHelperBack = new RectAreaLightHelper( this.lights.rectLightBack );
    //this.lights.rectLightBack.add( this.lights.rectLightHelperBack );

    this.lights.rectLightL = new THREE.RectAreaLight( 0xffffff, 1, 10, 10 );
    this.lights.rectLightL.position.set( this.bounds.minX, 0, 0 );
    this.lights.rectLightL.lookAt(this.bounds.maxX, 0, 0);
    // this.lights.rectLightHelperL = new RectAreaLightHelper( this.lights.rectLightL );
    // this.lights.rectLightL.add( this.lights.rectLightHelperL );

    this.lights.rectLightR = new THREE.RectAreaLight( 0xffffff, 0.5, 10, 10 );
    this.lights.rectLightR.position.set( this.bounds.maxX, 0, 0 );
    this.lights.rectLightR.lookAt(this.bounds.minX, 0, 0);
    this.lights.rectLightHelperR = new RectAreaLightHelper( this.lights.rectLightR );
    //this.lights.rectLightR.add( this.lights.rectLightHelperR );

    // light
    this.ambientLight = new THREE.AmbientLight( 0x404040 ); // soft white light
    this.frontDirectionalLight = new THREE.DirectionalLight(0xffffff);
    this.frontDirectionalLight.position.set(5, -1, 5);
    this.backDirectionalLight = new THREE.DirectionalLight(0xffffff, 0.5);
    this.backDirectionalLight.position.set(-5, -1, -5);

    // this.lights.ambientLight = new THREE.AmbientLight( { color: 0xf5f5f5, intensity: 0.01 } );
    // this.lights.frontLight = new THREE.DirectionalLight(0xffffff, 1.0);
    // this.lights.frontLight.position.set(-20, -50, 0.5);
    // this.lights.sideLight = new THREE.DirectionalLight(0xffffff, 0.8);
    // this.lights.sideLight.position.set(50, -10, 0.75);
    // this.lights.backLight = new THREE.DirectionalLight(0xffffff, 1.0);
    // this.lights.backLight.position.set(0, 50, 0.5);
    // this.lights.topLight = new THREE.DirectionalLight(0xffffff, 0.75 );
    // this.lights.topLight.position.set(0, 0, 100);
  }

  /**
   * (re-)inserts the lights into the scene
   */
  addLights() {
    //this.scene.add(this.lights.ambientLight);
    // this.scene.add( this.lights.rectLightTop );
    // this.scene.add( this.lights.rectLightBot );
    // this.scene.add( this.lights.rectLightFront );
    // this.scene.add( this.lights.rectLightBack );
    // this.scene.add( this.lights.rectLightL );
    // this.scene.add( this.lights.rectLightR );
    // this.scene.add(this.lights.frontLight);
    // this.scene.add(this.lights.sideLight);
    // this.scene.add(this.lights.backLight);
    // this.scene.add(this.lights.topLight);
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
    //this.scene.background = new THREE.Color('white'); // do NOT set scene background or outline selection won't work

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl, antialias:true });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight, false);
    //this.renderer.autoClear = false; //<ctc> by default true
    //this.renderer.setClearColor( 0x000000, 0 ); //<ctc> very questionably necessary -> delete me when going through <ctc>s (ensure other stuff works first)
    //this.renderer.setPixelRatio( window.devicePixelRatio ); //<ctc> this is done in example, but killed the camera in mine... try again? tried: failed. 

    // camera and controls
    this.createCamerasAndControls();

    // composer, renderpass, outlinepass, fxaa
    this.createComposer();

    // Bounds
    this.resetBounds();  //TODO need to set bounds and update lights when new crystals are added

    // lights
    var pointLight = new THREE.PointLight( 0xffffff, 1 );
    this.perspCamera.add( pointLight );
    this.initLights();

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
    let width = this.refs.msCanvas.clientWidth, height = this.refs.msCanvas.clientHeight;

    // <ctc> huh? dump this or keep it? (try)
    this.refs.msCanvas.width = width;
    this.refs.msCanvas.height = height;

    // update camera
    this.updateCamera(width, height);  //<ctc> I think this should come before resize renderer and composer

    // Resize renderer and composer
    this.renderer.setSize(width, height, false);
    this.composer.setSize(width, height, false);
    this.effectFXAA.uniforms[ 'resolution' ].value.set( 1 / width, 1 / height );
    // <ctc> maybe need to add things here (see commented section of createComposer)

    // Redraw scene with updates
    this.renderScene();
  }

  /**
   * Event handling for mouse click
   * @param {Event} event
   */
  handleMouseRelease(event) {
    // Handle left click release
    if (event.button === 0) {
      const position = this.getPickPosition(event);
      if (this.pick(position, event.ctrlKey)) // click w/ ctrl held down to produce model's original samples
        event.stopPropagation(); // release the event if this picks something
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
    if (this.props.decomposition === null) {
      return;
    }

    // Get intersected object
    const { datasetId, decompositionCategory, decompositionField, persistenceLevel } = this.props.decomposition;
    this.raycaster.setFromCamera(normalizedPosition, this.camera);
    let intersectedObjects = this.raycaster.intersectObjects(this.scene.children);
    intersectedObjects = intersectedObjects.filter((io) => io.object.name !== '');
    if (intersectedObjects.length) {
      // this.outlinePass.selectedObjects = [];
      // this.outlinePass.selectedObjects.push(intersectedObjects[0].object);
      //this.pickedObject = intersectedObjects[0].object;

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

        this.outlinePass.selectedObjects = [];
        this.outlinePass.selectedObjects.push(intersectedObjects[0].object);

        // new crystal selected, so revert currently picked object and highlight new one
        if (this.pickedObject) {
          this.outlinePass.selectedObjects = [this.pickedObject];
          //this.pickedObject.material.opacity = 1.0;
          this.pickedObject.material.emissive = new THREE.Color('black'); // todo: use Material.dispose() (see https://threejs.org/docs/#manual/en/introduction/How-to-dispose-of-objects)
          this.crystalPosObject = undefined;
        }
        this.pickedObject = intersectedObjects[0].object;
        //this.pickedObject.material.opacity = 0.75; //TODO: declare this value somewhere
        //argh! this.pickedObject.material.emissive = new THREE.Color('aqua');

        // add a clickable plane perpendicular to the curve (using curve.getTangent(u))... or just another sphere for now
        // Hmm... maybe create one of these for each crystal? Then they can remember their positions per crystal.
        //        may need to save the catmull rom curves for each crystal in order to move these along the curve.
        //this.crystalPosObject = this.addSphere(curve.getPoint(0.5), new THREE.Color('darkorange'));
      }

      // // Update opacity to signify selected crystal
      // if (this.pickedObject) {
      //   // Make sure have object in current scene
      //   this.pickedObject = this.scene.getObjectByName(this.pickedObject.name);
      //   this.pickedObject.material.opacity = 0.75;
      //   this.pickedObject = undefined;

      //   // add a clickable sphere along the curve
      //   this.addSphere(curve.getPoint(0.5), THREE.Color('darkorange'));
      // }
      // this.pickedObject = intersectedObjects[0].object;
      // this.pickedObject.material.opacity = 1;
      
      this.renderScene();

      // Get crystal partitions
      let crystalID = this.pickedObject.name;
      this.props.evalShapeoddsModelForCrystal(datasetId, decompositionCategory, decompositionField, persistenceLevel,
        crystalID, 51 /* numZ*/, showOrig);  // <ctc> *this* hardcoded 51 might be the reason the final sample is black! damn hardcoding!!
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
    let material = new THREE.MeshBasicMaterial({ color:color });
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
    let material = new THREE.MeshBasicMaterial({ color:color });
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
    regressionData.curves.forEach((regressionCurve, index) => {
      let curvePoints = [];
      regressionCurve.points.forEach((regressionPoint) => {
        curvePoints.push(new THREE.Vector3(regressionPoint[0], regressionPoint[1], regressionPoint[2]));
      });
      // Create curve
      let curve = new THREE.CatmullRomCurve3(curvePoints);
      let tubularSegments = curvePoints.length * 6;
      let curveGeometry = new THREE.TubeBufferGeometry(curve, tubularSegments, .02 /*radius*/, 10 /*radialSegments*/, false /*closed*/);
      let count = curveGeometry.attributes.position.count;
      curveGeometry.setAttribute('color', new THREE.BufferAttribute(new Float32Array(count * 3), 3));
      let colors = regressionCurve.colors;
      let colorAttribute = curveGeometry.attributes.color;
      let color = new THREE.Color();
      for (let i = 0; i < curvePoints.length; ++i) {
        color.setRGB(colors[i][0], colors[i][1], colors[i][2]);
        for (let j = 0; j < tubularSegments; ++j) {
          colorAttribute.setXYZ(i*tubularSegments+j, color.r, color.g, color.b);
        }
      }
      //let opacity = 1.0;
      let curveMaterial = new THREE.MeshLambertMaterial({
        //color: 0xffffff,
        //flatShading: true,
        vertexColors: true,//THREE.VertexColors,
        //transparent: true,
        //opacity: opacity,
      });
      //curveMaterial.emissive = new THREE.Color('black');
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
   * createComposer
   * Creates rendering architecture
   */
  createComposer() {
    let width = this.refs.msCanvas.clientWidth, height = this.refs.msCanvas.clientHeight;

    this.composer = new EffectComposer(this.renderer);
    var renderPass = new RenderPass(this.scene, this.camera);
    //renderPass.clear = false; //<ctc> by default true, just like renderer
    this.composer.addPass(renderPass);

    this.outlinePass = new OutlinePass(new THREE.Vector2(width, height), this.scene, this.camera); // <ctc> need to update these on resize?
    this.outlinePass.visibleEdgeColor.set('cyan');
    this.outlinePass.hiddenEdgeColor.set('#182838');
    this.outlinePass.edgeStrength = 8.0;
    this.outlinePass.edgeThickness = 1.0;
    this.outlinePass.overlayMaterial.blending = THREE.NormalBlending;
    //this.composer.addPass(this.outlinePass);

    this.effectFXAA = new ShaderPass( FXAAShader );
    this.effectFXAA.uniforms[ 'resolution' ].value.set( 1 / width, 1 / height );  // <ctc> probably need to update these on resize
    /* from example <ctc>: try it
    var pixelRatio = renderer.getPixelRatio();
    fxaaPass.material.uniforms[ 'resolution' ].value.x = 1 / ( container.offsetWidth * pixelRatio );
    fxaaPass.material.uniforms[ 'resolution' ].value.y = 1 / ( container.offsetHeight * pixelRatio );
    */
    //this.composer.addPass( this.effectFXAA );
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
      //this.outlinePass.camera = this.perspCamera; //<ctc> can you do this?
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
    this.scene.add(this.ambientLight);
    this.scene.add(this.frontDirectionalLight);
    this.scene.add(this.backDirectionalLight);

    //this.scene.add(this.camera); // kinda looks like new perspective camera needs to be part of scene (like above) -> it does NOT
    //this.addLights();

    this.pickedObject = undefined;
    this.crystalPosObject = undefined;
    this.outlinePass.selectedObjects = [];

/******/
    // floor and other temporary stuff or visual aids
    let origin = new THREE.Vector3(0, 0, 0);
    //let up = new THREE.Vector3(0, 0, 1);
    //this.floorMesh = this.addPlane( {position:origin.clone().set(0,0,-0.05), normal:up, size:[500,500], color:new THREE.Color(0xd0d0d0)} );
    //<ctc> may add outline(s) and/or back/side walls to help give context to the min/max of the crystals
    //<ctc> another thing that might help would be a color bar, similarly indicating the range of values
    //<ctc> test add some spheres to the scene to help w/ lighting (todo: removeme)
    this.crystalPosObject = this.addSphere( origin, new THREE.Color('purple') );
    this.crystalPosObject = this.addSphere( new THREE.Vector3(1, -1, 1), new THREE.Color('darkorange') );
    this.crystalPosObject = this.addSphere( new THREE.Vector3(-1, 1, 1), new THREE.Color('darkorange') );
/******/

    this.updateCamera(this.refs.msCanvas.width, this.refs.msCanvas.height, true /*resetPos*/);
  }

  /**
   * Draws the scene to the canvas.
   */
  renderScene() {
    //this.renderer.render(this.scene, this.camera);
    this.composer.render();
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
