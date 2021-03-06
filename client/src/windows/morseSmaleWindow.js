import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { SpriteMaterial } from 'three/src/materials/SpriteMaterial.js';
import { Box3 } from 'three/src/math/Box3.js';
import { Vector3 } from 'three/src/math/Vector3.js';
import { Vector2 } from 'three/src/math/Vector2.js';
import { OutlineEffect } from 'three/examples/jsm/effects/OutlineEffect.js';
import React from 'react';
import { withStyles } from '@material-ui/core/styles';
import { withDSXContext } from '../dsxContext';
import _ from 'lodash';

const styles = (theme) => ({
  ms_canvas: {
    width: '100%',
    height: '100%',
  },
  container: {
    position: 'relative',
  },
  overlay: {
    position: 'absolute',
    left: '10px',
    top: '10px',
  },
});

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
    this.numInterpolants = 10; // how many samples to generate using current model [to fill drawer]

    this.state = {
      drawerAdded: false,               // when parent component adds a drawer, resize isn't called, so force it
      boundingBox: undefined,           // set when curves added to scene, used to set up camera
      evalModel: { inProgress: false,   // dragging along a crystal
                   next: undefined,     // { crystalID, percent }
                   sprites: undefined,  // { nearestLesser, interpolated, nearestGreater }
                   p0: new Vector2(),   // projection of currently selected curve to normalized screen coordinates
                   p1: new Vector3(),
                   percent: 0.5,
                 },
      validate: false,                  // when crystal selected, use model's z_coords to reconstruct original shapes
      diff_validate: false,             // return diff of these with original images

      extremaRad: 0.05,
      crystalRad: 0.02,
    };

    this.client = this.props.dsxContext.client;


    // <ctc> get rid of any of these I can
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
    this.onMouseWheel = this.onMouseWheel.bind(this);

    this.pick = this.pick.bind(this);
    this.getPickPosition = this.getPickPosition.bind(this);
    this.getCanvasPosition = this.getCanvasPosition.bind(this);

    this.addRegressionCurvesToScene = this.addRegressionCurvesToScene.bind(this);
    this.addExtremaToScene = this.addExtremaToScene.bind(this);

    this.renderScene = this.renderScene.bind(this);
  }

  /**
   * Called by react when this component mounts.
   * Initializes Three.js for drawing and adds event listeners.
   */
  componentDidMount() {
    this.init();
    window.addEventListener('resize', _.debounce(this.resizeCanvas, 200));
    window.addEventListener('keydown', this.handleKeyDownEvent);
    this.refs.msCanvas.addEventListener('wheel', _.debounce(this.onMouseWheel, 50));
    this.refs.msCanvas.addEventListener('mousedown', this.onMouseDown);
  }

  /**
   * Test for any decomposition settings changed that require new crystals to be shown.
   *
   * @param {object} prevDecomposition - the previous decomposition
   * @param {object} currentDecomposition - the current decomposition
   * @return {boolean} true if any of the decomposition settings have changed.
   * 
   * NOTE: Similar to function in EmbeddingWindow, but items checked aren't the same. 
   */
  isNewDecomposition(prevDecomposition, currentDecomposition) {
    return (prevDecomposition.datasetId !== currentDecomposition.datasetId
            || prevDecomposition.category !== currentDecomposition.category
            || prevDecomposition.fieldname !== currentDecomposition.fieldname
/*          || prevDecomposition.modelname !== currentDecomposition.modelname  // don't redraw when model [de]selected */
            || prevDecomposition.decompositionMode !== currentDecomposition.decompositionMode
            || prevDecomposition.k !== currentDecomposition.k
            || prevDecomposition.persistenceLevel !== currentDecomposition.persistenceLevel
            || prevDecomposition.ms !== currentDecomposition.ms);
  }

  hasModel() {
    return this.props.decomposition !== null && this.props.decomposition.modelname !== "None";
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

    if (this.state !== prevState) {
      console.log("MS: state changed");
    }

    if (prevProps.decomposition === null
        || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)
        //|| prevProps.distanceMetric !== this.props.distanceMetric
       ) {
      this.resetScene();
      // object unpacking (a javascript thing, props is inherited from the React component)
      const { fieldname, category, datasetId, persistenceLevel } = this.props.decomposition;
      const metric = this.props.distanceMetric;
      const layout = this.props.decomposition.ms.layout;
      Promise.all([
        this.client.fetchMorseSmaleRegression(datasetId, category, fieldname, metric, layout, persistenceLevel),
        this.client.fetchMorseSmaleExtrema(datasetId, category, fieldname, metric, layout, persistenceLevel),
      ]).then((response) => {
        const [regressionResponse, extremaResponse] = response;
        if (!regressionResponse.error && !extremaResponse.error) {
          this.addRegressionCurvesToScene(regressionResponse);
          this.addExtremaToScene(extremaResponse.extrema);
          this.pickCrystal(this.regressionCurves[0]); // automatically pick first crystal
          this.updateDynamicModelEval(0.5); // select middle of crystal
          this.renderScene();
        }
        else {
          console.log('morseSmaleWindow.componentDidUpdate error:\n\t regressionResponse: '
                      +regressionResponse.error_msg+'\n\t extremaResponse: '+extremaResponse.error_msg);
        }
      });
    }
    // if model changed and crystal selected, evaluate new model
    else if (prevProps.decomposition &&
        prevProps.decomposition.modelname !== this.props.decomposition.modelname &&
        this.pickedCrystal !== undefined) {
      let crystalID = this.pickedCrystal.name;
      this.props.evalModelForCrystal(crystalID, this.numInterpolants, false, this.state.validate, this.state.diff_validate);
      this.updateDynamicModelEval(this.state.evalModel.percent); 
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

    // left click to select and slide
    if (event.button === 0 && this.pick(this.getPickPosition(event))) {
      let hasModel = this.props.decomposition.modelname !== "None";
      this.continuousInterpolation = hasModel;
      this.controls.enabled = false;
    }
  }

  /**
   * mousemove ends a potential crystal so rotations don't accidentally select a crystal
   */
  onMouseMove(event) {
    if (this.continuousInterpolation) {
      this.evalModel(this.getPercent(this.getPickPosition(event)));
    }
    else {
      // can release mousemove handler since orbit controls handles camera
      this.refs.msCanvas.removeEventListener('mousemove', this.onMouseMove);
    }
  }

  /**
   * mouseup potentially selects a crystal
   */
  onMouseUp(event) {
    this.refs.msCanvas.removeEventListener('mousemove', this.onMouseMove);
    this.refs.msCanvas.removeEventListener('mouseup', this.onMouseUp);

    // reset camera controls and continuous interpolation state
    this.controls.enabled = true;
    this.continuousInterpolation = false;

    // update projected extrema
    if (this.pickedCrystal !== undefined) {
      this.projectExtrema(this.pickedCrystal);
    }
  }

  onMouseWheel(event) {
    // update projected extrema
    if (this.pickedCrystal !== undefined) {
      this.projectExtrema(this.pickedCrystal);
    }
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

/*
    // fieldval overlay used by dynamic model interpolation (todo: the div is causing the crystals view to turn tiny)
    this.fieldvalNode = this.refs.fieldvalue;
    this.fieldvalNode.textContent = "transyvania six five thousand";
*/

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

    // update sprite's aspect ratio
    const old_aspect = this.perspCamera.aspect;
    const new_aspect = width / height;
    this.imageSprite.scale.y *= new_aspect / old_aspect;

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
      this.toggleCamera();     // calls controls.reset, so if one works, this will too
      break;
    case 'r': // reset view
      this.controls.reset();   // resets camera to original position
      break;
    case ']': //increase numInterpolants
      this.numInterpolants++;
      console.log("numInterpolants increased to " + this.numInterpolants)
      break;
    case '[': //decrease numInterpolants
      this.numInterpolants = Math.max(1, this.numInterpolants - 1);
      console.log("numInterpolants decreased to " + this.numInterpolants)
      break;
    case 'q': // validate model by interpolating original z-coords to fill drawer when crystal selected
      this.state.validate = !this.state.validate;
      break;
    case 'd': // return diff when validating
      this.state.diff_validate = !this.state.diff_validate;
      break;
/*
    // scaling extrema, updated using setstate
    // todo: let componentDidUpdate call render
    //       use same keys to grow/shrink crystal tubes
    //       conflict with embeddingWindow on keys, so currently disabled
    case '=': // Increase thumbnail size
    case '+':
      this.setState({ extremaRad: this.state.extremaRad * 1.25 });
      break;
    case '-': // decrease thumbnail size
      this.setState({ extremaRad: this.state.extremaRad * 0.75 });
      break;
*/
    }
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
    let pick = [ (pos.x / canvas.clientWidth) * 2 - 1, (pos.y / canvas.clientHeight) * -2 + 1 ];
    //console.log("pick position: "+ pick[0] + ", " + pick[1]);

    return {
      x: pick[0],
      y: pick[1],
    };
  }

  /**
    * Return position at given percent along currently selected crystal.
    * @param {float} percent along crystal
    * @return {THREE.Vector3}
    */
  getPosAlongSelectedCrystal(percent) {
    if (this.pickedCrystal !== null) {
      let curve = this.pickedCrystal.geometry.parameters.path;
      return curve.getPoint(percent);
    }
    return new THREE.Vector3(0,0,0);
  }

  /*
   * set the material for the current image, hiding image if no data
   * @param {data} raw base64 [png] image data
   */
  setImage(data) {
    if (data !== undefined && this.hasModel()) {

      let width = this.refs.msCanvas.clientWidth, height = this.refs.msCanvas.clientHeight;
      const aspect = width / height;

      // load the new image sent from the server
      this.textureLoader.load(
        'data:image/png;base64,' + data.img.rawData, function(texture) {
          this.imageSprite.material.map = texture;
          this.imageSprite.visible = true;
          this.imageSprite.material.needsUpdate = true;
          this.imageSprite.scale.x = this.spriteScale;
          this.imageSprite.scale.y = texture.image.height / texture.image.width * this.spriteScale * aspect;

          // update sprite
          //this.renderSprites(); // TODO: renderSprites still clears canvas (React? THREE.js?) so render whole scene
          //NOT EVEN THIS WORKS! it's because orbitcontrols has taken over updates (prolly why it always clears too)
          // Right now you MUST move the mouse to update (FIXME)
          this.renderScene();

        }.bind(this));
    }
    else {
      this.imageSprite.visible = false;
    }
  }

  /*
   * Get a single sample at the given percent along the selected crystal
   * @param {float} percent along current crystal to evaluate (in range [0,1])
   */
  async evalModel(percent) {
    let crystalID = this.pickedCrystal.name;

    // evalModel in progress, so set/replace next to be requested
    if (this.state.evalModel.inProgress) {
      this.state.evalModel.next = { crystalID, percent };
    }
    else {
      this.state.evalModel.inProgress = true;
      this.state.evalModel.percent = percent;
      this.renderScene(); // need to do this right away to keep position object in place <ctc> make sure nothing else does it
      this.props.evalModelForCrystal(crystalID, 1 /*numSamples*/, false /*showOrig*/,
                                     false /*validate*/, false /* diff_validate */,
                                     percent).then((result) => {
        this.state.evalModel.inProgress = false;

        this.setImage(result);
        console.log("dynamic evalModel("+percent+"): fieldval: "+result.val);

        // TODO: display fieldval in it's container: this.fieldvalNode.textContent = result.val.toExponential(4);
        // - https://webglfundamentals.org/webgl/lessons/webgl-text-html.html

        // render the next if any
        if (this.state.evalModel.next !== undefined) {
          this.evalModel(this.state.evalModel.next.percent);
          this.state.evalModel.next = undefined;
        }
      });
    }
  }

  /**
   * Compute how far point is along a curve
   * @param {object} normalizedPosition pick position in normalized screen space <[-1,1], [-1,1]>
   * @return percent of mouse position along currently selected crystal
   */
  getPercent(normalizedPosition) {
    // project vector (pt - p0) to (p1 - p0) to get percent
    let curveVec = new Vector2().subVectors(this.state.evalModel.p1, this.state.evalModel.p0);
    let pickVec = new Vector2().subVectors(normalizedPosition, this.state.evalModel.p0);

    // compute percent of projected pickVec along curveVec
    let crvlen = curveVec.length();
    curveVec.normalize();
    let percent = pickVec.dot(curveVec); // a dot b = |a| * |b| * cos(theta)
    percent = percent / crvlen;

    // clamp percent and handle div by 0
    if (percent === NaN) percent = 0.0; // div by 0 if p0 == p1 (an unlikely but possible projection)
    percent = Math.max(0.0, Math.min(1.0, percent));

    return percent;
  }

  /**
   * Projects the crystal's endpoints to normalized screen space.
   * @param {object} crystal object (a mesh)
   * @param {point} normalized screen space position ([-1,1], [-1,1])
   */
  projectExtrema(crystal) {
    // get original endpoints and apply current geometry's transformation, then
    // use camera's transformation to bring them into normalized screen space
    let matrix = crystal.matrix
    let curve = crystal.geometry.parameters.path;
    let p0 = curve.getPoint(0.0).applyMatrix4(matrix).project(this.camera);
    let p1 = curve.getPoint(1.0).applyMatrix4(matrix).project(this.camera);

    this.state.evalModel.p0.set(p0.x, p0.y);
    this.state.evalModel.p1.set(p1.x, p1.y);
  }

  /**
   * Change picked crystal
   */
  pickCrystal(crystal) {
    if (!crystal) {
      console.log("error: tried to pick null crystal!");  // not expected to ever be called with null
      return;
    }

    let crystalID = crystal.name;
    console.log('New crystal selected (' + crystalID + ')');

    const { datasetId, persistenceLevel } = this.props.decomposition;

    // ensure previously selected object is back to unselected opacity
    if (this.pickedCrystal) {
      this.pickedCrystal.material.opacity = this.unselectedOpacity;
    }

    // set newly-selected crystal as picked
    this.pickedCrystal = crystal;
    this.pickedCrystal.material.opacity = this.selectedOpacity;

    // project extrema of selected crystal to screen space to compute distance along curve
    this.projectExtrema(this.pickedCrystal);

    // highlight this crystal's samples in the other views (e.g., embedding window)
    this.client.fetchCrystal(datasetId, persistenceLevel, crystalID).then((result) => {
      this.props.onCrystalSelection(result.crystalSamples);
    });

    // evaluate the current model for this crystal to fill the drawer (parent component updates drawer)
    this.props.evalModelForCrystal(crystalID, this.numInterpolants, false, this.state.validate, this.state.diff_validate);
  }

  /**
   * If a model is selected, update the dynamic interpolation slider position and sprite.
   * @param {object} percent Distance along crystal at which to evaluate.
   */
  updateDynamicModelEval(percent) {
    if (this.hasModel()) {
      this.evalModel(percent);
    }
    else {
      this.state.evalModel.percent = 0.5;
      // <ctc> if there was never a model in the first place, this is a wasted render
      this.renderScene(); // be sure scene still gets re-rendered so new crystal is highlighted <ctc> still always necessary?
    }
  }

  /**
   * Pick level set of decomposition
   * @param {object} normalizedPosition pick position in normalized screen space <[-1,1], [-1,1]>
   * @return returns false if nothing picked and scene rotation can continue
   */
  pick(normalizedPosition) {
    if (this.props.decomposition === null) {
      return false;
    }

    // Get intersected object
    this.raycaster.setFromCamera(normalizedPosition, this.camera);
    let intersectedObjects = this.raycaster.intersectObjects(this.scene.children);
    intersectedObjects = intersectedObjects.filter((io) => io.object.name !== '');

    if (intersectedObjects.length) {
      let crystal = intersectedObjects[0].object;

      // If this is a new crystal, pick it.
      // otherwise, picked active pointer along curve, or already selected curve, so do nothing
      if (crystal !== this.crystalPosObject && crystal !== this.pickedCrystal) {
        this.pickCrystal(crystal);
      }

      // interactive model evaluation
      this.updateDynamicModelEval(this.state.evalModel.percent);
    
      return true; // curve picked and now we can be sliding along it to dynamically interpolate model
    }

    return false; // nothing picked so go ahead and rotate
  }

  /**
   * Adds a line to the scene
   * @param {object} scene add the new object to this scene
   * @param {vector3} p0 
   * @param {vector3} p1
   * @param {vector3} color THREE.Color object
   * returns object added to scene
   */
  addLine(scene, p0, p1, color = new THREE.Color(0xddccbb)) {
    let vertices = p0.concat(p1);
    var	material = new THREE.LineBasicMaterial( { color: color,  vertexColors: false } );
    var geometry = new THREE.BufferGeometry();
    geometry.setAttribute( 'position', new THREE.Float32BufferAttribute( vertices, 3 ) );
    let line = new THREE.Line(geometry, material);
    scene.add(line);
    return line;
  }

  /**
   * Adds a sphere to the scene
   * @param {vector3} pos position
   * @param {vector3} color THREE.Color object
   * @param {float} radius
   * returns object added to scene
   */
  addSphere(scene, pos, color, radius = 0.05) {
    let geometry = new THREE.SphereBufferGeometry(radius, 32, 32);
    let material = new THREE.MeshStandardMaterial({ color:color });
    let mesh = new THREE.Mesh(geometry, material);
    let dist = pos.length();
    let dir = pos.clone().normalize();
    mesh.translateOnAxis(dir, dist);
    scene.add(mesh);
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
  addPlane({scene, position = [0,0,0], normal = [0,0,1], size = [1,1], color = null} ) {
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
    scene.add(mesh);
    return mesh;
  }

  /**
   * Adds the regression curves to the scene
   * @param {object} regressionData
   */
  addRegressionCurvesToScene(regressionData) {
    this.state.boundingBox = new Box3();

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
      let radius = this.state.crystalRad;
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
      this.regressionCurves.push(curveMesh);  // save these in order to automatically select a crystal when ms updated

      // add to scene bounding box
      curveGeometry.computeBoundingBox();
      this.state.boundingBox.union(curveGeometry.boundingBox);
    });
  }

  /**
   * Adds the extrema to the scene.
   * @param {object} extrema
   */
  addExtremaToScene(extrema) {
    extrema.forEach((extreme) => {
      let position = new THREE.Vector3().fromArray(extreme.position);
      this.addSphere(this.scene, position, new THREE.Color(extreme.color[0], extreme.color[1], extreme.color[2]),
                     this.state.extremaRad);
    });
  }

  createUI() {
    // uiScene
    this.uiScene = new THREE.Scene();

    // uiCamera
    this.uiCamera = new THREE.OrthographicCamera( - 1, 1, 1, - 1, 0, 2 );
    this.uiCamera.position.set( 0, 0, 1 );
    this.uiCamera.name = "uiCamera";

    // instantiate a loader and sprites
    this.textureLoader = new THREE.TextureLoader();

    // imageSprite
    this.spriteScale = 0.4;
    this.imageSprite = new THREE.Sprite();
    this.imageSprite.position.set(0.7,-0.7,0.0); // position of center of sprite
    this.uiScene.add(this.imageSprite);

    // for continuous interpolation of a selected crystal
    this.continuousInterpolation = false;
  }

  /**
   * createCamerasAndControls
   * There are two types of cameras, perspective and orthographic, each with associated controls.
   * NOTE: need to have essentially "the camera for this scene" before creating controls so that controls.reset() works as expected.
   * TODO: z is normalized by Controller to [0,1], but x/y are not normalized, so adjust target when loading new data.
   *       use this.state.boundingBox computed in addRegressionCurvesToScene
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
    let perspPosition = new THREE.Vector3(4, -6, 1);
    this.perspCamera = new THREE.PerspectiveCamera(fov, aspect, near, far);
    this.perspCamera.position.copy(perspPosition);
    this.perspCamera.up.set(0, 0, 1);
    this.perspCamera.add(new THREE.PointLight(0xffffff));
    this.perspCamera.name = "camera";
    this.updatePerspCamera(width, height, fov, this.state.boundingBox);  // <ctc> this huh??

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

    this.regressionCurves = [];
    this.pickedCrystal = undefined;
    this.crystalPosObject = this.addSphere(this.scene, new THREE.Vector3(), new THREE.Color('darkorange'),
                                           this.state.extremaRad * 0.6);
    this.crystalPosObject.visible = false;
    this.imageSprite.visible = false;

    this.state.evalModel.inProgress = false;
    this.state.evalModel.next = undefined;
    this.state.evalModel.percent = 0.5;

    this.updateCamera(this.refs.msCanvas.width, this.refs.msCanvas.height, true /*resetPos*/);
  }

  /**
   * Draws the 2d portion of the scene.
   */
  renderSprites() {
    // FIXME: This shouldn't clear canvas but currently it does.
    //        It means we always renderScene even when only this sprite needs updating.
    this.renderer.render(this.uiScene, this.uiCamera);
  }

  /**
   * Draws the scene to the canvas.
   */
  renderScene() {
    // let start = Date.now();  // time the render (it's fast, just commiting this to remember how :)

    var scene = Object.values(this.scene.children);
    //this.renderer.clear();  // FIXME: canvas gets cleared regardless (why??); renderSprite issue. We *want* this here.

    // update the crystalPosObject
    this.updateCrystalPosObject();

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
    // let end = Date.now();
    // console.log("Rendered M-S Crystals in: " + (end - start).toString() + " ms");

    // ...and finally, redraw the sprite
    this.renderSprites();
  }

  updateCrystalPosObject() {
    this.crystalPosObject.visible = this.hasModel() && this.pickedCrystal;

    // set crystalPosObject position
    if (this.pickedCrystal) {
      let pos = this.getPosAlongSelectedCrystal(this.state.evalModel.percent);
      this.crystalPosObject.position.copy(pos.applyMatrix4(this.pickedCrystal.matrix));
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
        <canvas ref='msCanvas' style={style} />
    );

    /*
    const { classes } = this.props;  // come from styles dict at the top using withStyles (below)
    return (
      <div className='container'>
        <canvas ref='msCanvas' className={classes.ms_style} />
        <div ref='fieldvalue' className={classes.overlay} />
      </div>
    );
    */
  }
}

export default withDSXContext(withStyles(styles)(MorseSmaleWindow));
