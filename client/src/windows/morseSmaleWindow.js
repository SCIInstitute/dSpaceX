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

    this.init = this.init.bind(this);
    this.addRegressionCurvesToScene = this.addRegressionCurvesToScene.bind(this);
    this.renderScene = this.renderScene.bind(this);
    this.animate = this.animate.bind(this);
    this.clearScene = this.clearScene.bind(this);
  }

  componentDidMount() {
    this.init();
    this.animate();
  }

  componentDidUpdate(prevProps, prevState, prevContext) {
    if (this.props.decomposition === null) {
      return;
    }

    if (prevProps.decomposition === null
      || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)) {
      this.clearScene();
      const { datasetId, k, persistenceLevel } = this.props.decomposition;
      Promise.all([
        this.client.fetchMorseSmaleRegression(datasetId, k, persistenceLevel),
        this.client.fetchMorseSmaleExtrema(datasetId, k, persistenceLevel),
      ]).then((response) => {
        const [regression, extrema] = response;
        console.log(extrema);
        this.addRegressionCurvesToScene(regression.crystals);
        this.renderScene();
      });
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

  init() {
    // canvas
    let canvas = this.msCanvas;
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
    let ambientLight = new THREE.AmbientLight( 0x404040 ); // soft white light
    let directionalLight1 = new THREE.DirectionalLight(0xffffff);
    directionalLight1.position.set(1, 1, 1);
    let directionalLight2 = new THREE.DirectionalLight(0x002288)
    directionalLight2.position.set(-1, -1, -1);

    // world
    this.scene = new THREE.Scene();
    this.scene.add(this.camera);
    this.scene.add(ambientLight);
    this.scene.add(directionalLight1);
    this.scene.add(directionalLight2);

    // renderer
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl });
    this.renderer.setSize( canvas.clientWidth, canvas.clientHeight );

    // controls
    this.controls = new OrthographicTrackballControls(this.camera, this.renderer.domElement );
    this.controls.rotateSpeed = 0.5;
    this.controls.zoomSpeed = 0.1;
    this.controls.panSpeed = 0.5;
    this.controls.noZoom = false;
    this.controls.noPan = false;
    this.controls.staticMoving = true;
    this.controls.dynamicDampingFactor = 0.3;
    this.controls.keys = [65, 83, 68];
    this.controls.addEventListener( 'change', this.renderScene );

    this.renderScene();
  }

  addRegressionCurvesToScene(crystals) {
    crystals.forEach((crystal) => {
      let curvePoints = [];
      crystal.regressionPoints.forEach((regressionPoint) => {
        curvePoints.push(new THREE.Vector3(regressionPoint[0], regressionPoint[1], regressionPoint[2]));
      });
      // Create curve
      let curve = new THREE.CatmullRomCurve3(curvePoints);
      let curveGeometry = new THREE.TubeBufferGeometry(curve, 50, .02, 50, false);
      let curveMaterial = new THREE.ShaderMaterial({
        uniforms: {
          color1: {
            value: new THREE.Color('green'),
          },
          color2: {
            value: new THREE.Color('red'),
          },
          bboxMin: {
            value: 1,
          },
          bboxMax: {
            value: 100,
          },
        },
        vertexShader: `
        varying vec2 vUv;

        void main() {
          vUv = uv;
          gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);}`,
        fragmentShader: `
        uniform vec3 color1;
        uniform vec3 color2;

        varying vec2 vUv;

        void main() {
          gl_FragColor = vec4(mix(color1, color2, vUv.x), 1.0);}`,
        wireframe: false,
        depthTest: true,
      });
      let curveMesh = new THREE.Mesh(curveGeometry, curveMaterial);
      curveMesh.rotateX(-90);
      this.scene.add(curveMesh);
    });
  }

  renderScene() {
    this.renderer.render(this.scene, this.camera);
  }

  animate() {
    requestAnimationFrame(this.animate);
    this.controls.update();
  }

  clearScene() {
    while (this.scene.children.length > 0) {
      this.scene.remove(this.scene.children[0]);
    }
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
        <canvas ref={(ref)=> (this.msCanvas = ref)} style={canvasStyle} />
      </Paper>
    );
  }
}

export default withDSXContext(MorseSmaleWindow);
