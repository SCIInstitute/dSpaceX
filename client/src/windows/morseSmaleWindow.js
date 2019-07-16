import Paper from '@material-ui/core/Paper';
import React from 'react';
import * as THREE from 'three';
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
  }

  componentDidMount() {
    let canvas = this.refs.msCanvas;
    let gl = canvas.getContext('webgl');

    this.scene = new THREE.Scene();
    this.camera = new THREE.PerspectiveCamera(75, canvas.clientWidth/canvas.clientHeight, 0.1, 1000);
    this.renderer = new THREE.WebGLRenderer({ canvas:canvas, context:gl });
    this.renderer.setSize(canvas.clientWidth, canvas.clientHeight);
  }

  componentDidUpdate(prevProps, prevState, prevContext) {
    if (this.props.decomposition === null) {
      return;
    }

    if (prevProps.decomposition === null
      || this.isNewDecomposition(prevProps.decomposition, this.props.decomposition)) {
      const { datasetId, k, persistenceLevel } = this.props.decomposition;
      this.client.fetchMorseSmaleLayoutForPersistenceLevel(datasetId, k, persistenceLevel).then((r) => {
        r.crystals.forEach((c) => {
          let rPoints = [];
          c.regressionPoints.forEach((rp) => {
            rPoints.push(new THREE.Vector3(rp[0], rp[1], rp[2]));
          });
          console.log(rPoints); // TODO tomorrow draw the curve :)
        });
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
