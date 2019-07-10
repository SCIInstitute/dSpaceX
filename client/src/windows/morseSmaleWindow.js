import { createShaderProgram, webGLErrorCheck } from './glUtils';
import LineFragmentShaderSource from '../shaders/line.frag';
import LineVertexShaderSource from '../shaders/line.vert';
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
  }

  componentDidMount() {
    // Create program
    const canvas = this.refs.msCanvas;
    let gl = canvas.getContext('webgl');
    let program = createShaderProgram(gl, LineVertexShaderSource, LineFragmentShaderSource);

    // Provide data to program
    let positionAttributeLocation = gl.getAttribLocation(program, 'a_position');

    let positionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);

    let positions = [0, 0, 0.45, 0, 0.45, 0, 0.45, 0.45, 0.45, 0.45, 0, 0.45];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(positions), gl.STATIC_DRAW);

    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
    gl.clearColor(0, 0, 0, 0);
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.useProgram(program);

    gl.enableVertexAttribArray(positionAttributeLocation);
    let size = 2;
    let type = gl.FLOAT;
    let normalize = false;
    let stride = 0;
    let offset = 0;
    gl.vertexAttribPointer(positionAttributeLocation, size, type, normalize, stride, offset);

    let primitiveType = gl.LINES;
    gl.drawArrays(primitiveType, 0, (positions.length / size));
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
