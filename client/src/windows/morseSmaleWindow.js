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
    this.addExtremaToScene = this.addExtremaToScene.bind(this);
    this.renderScene = this.renderScene.bind(this);
    this.animate = this.animate.bind(this);
    this.resetScene = this.resetScene.bind(this);
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
    this.ambientLight = new THREE.AmbientLight( 0x404040 ); // soft white light
    this.directionalLight = new THREE.DirectionalLight(0xffffff);
    this.directionalLight.position.set(0, 5, 5);

    // world
    this.scene = new THREE.Scene();
    this.scene.add(this.ambientLight);
    this.scene.add(this.directionalLight);

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
      let curveMaterial = new THREE.MeshLambertMaterial({ color:0xffffff, flatShading:true, vertexColors:THREE.VertexColors });
      let curveMesh = new THREE.Mesh(curveGeometry, curveMaterial);
      curveMesh.rotateX(-90);
      this.scene.add(curveMesh);
    });
  }

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

  renderScene() {
    this.renderer.render(this.scene, this.camera);
  }

  animate() {
    requestAnimationFrame(this.animate);
    this.controls.update();
  }

  resetScene() {
    while (this.scene.children.length > 0) {
      this.scene.remove(this.scene.children[0]);
    }
    this.scene.add(this.ambientLight);
    this.scene.add(this.directionalLight);
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
