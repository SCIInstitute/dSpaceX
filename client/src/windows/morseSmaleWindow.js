import * as THREE from 'three';
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
    this.animate = this.animate.bind(this);
  }

  componentDidMount() {
    // Create renderer, camera, and scene
    let canvas = this.refs.msCanvas;
    let gl = canvas.getContext('webgl');
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl });
    console.log('Initial Renderer.');
    console.log(this.renderer);
    this.renderer.setSize( canvas.clientWidth, canvas.clientHeight );

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

    let light = new THREE.AmbientLight( 0x404040 ); // soft white light

    this.scene = new THREE.Scene();
    this.scene.add(this.camera);
    this.scene.add(light);
  }

  componentDidUpdate(prevProps, prevState, prevContext) {
    if (this.props.decomposition === null) {
      return;
    }

    if (prevProps.decomposition === null
      || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)) {
      // Clear scene
      while (this.scene.children.length > 0) {
        this.scene.remove(this.scene.children[0]);
      }
      const { datasetId, k, persistenceLevel } = this.props.decomposition;
      this.client.fetchMorseSmaleLayoutForPersistenceLevel(datasetId, k, persistenceLevel).then((response) => {
        console.log(response);
        response.crystals.forEach((crystal) => {
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
          });
          let curveMesh = new THREE.Mesh(curveGeometry, curveMaterial);
          // curveMesh.scale.set(2, 2, 2);
          curveMesh.translateZ(5);
          curveMesh.rotateX(-90);
          this.scene.add(curveMesh);

          // Render geometry and material to screen
          this.animate();
        });
      });
    }
  }

  animate() {
    requestAnimationFrame(this.animate);
    this.renderer.render(this.scene, this.camera);
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
        <canvas ref='msCanvas' className='msCanvas' style={canvasStyle} />
      </Paper>
    );
  }
}

export default withDSXContext(MorseSmaleWindow);
