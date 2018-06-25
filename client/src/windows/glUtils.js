/**
 * Compile vertex and fragment shader into a shader program.
 * @param {object} gl The OpenGL context.
 * @param {string} vertexShaderSource
 * @param {string} fragmentShaderSource
 * @return {reference}
 */
export function createShaderProgram(
  gl, vertexShaderSource, fragmentShaderSource) {
  let vertexShader = gl.createShader(gl.VERTEX_SHADER);
  gl.shaderSource(vertexShader, vertexShaderSource);
  gl.compileShader(vertexShader);
  let compiled = gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS);
  if (compiled) {
    console.log('Vertex shader compiled successfully');
  } else {
    console.log('Vertex shader failed to compile.');
    let compilationLog = gl.getShaderInfoLog(vertexShader);
    console.log('Shader compiler log: ' + compilationLog);
  }

  let fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
  gl.shaderSource(fragmentShader, fragmentShaderSource);
  gl.compileShader(fragmentShader);

  compiled = gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS);
  if (compiled) {
    console.log('Fragment shader compiled successfully');
  } else {
    console.log('Fragment shader failed to compile.');
    let compilationLog = gl.getShaderInfoLog(fragmentShader);
    console.log('Shader compiler log: ' + compilationLog);
  }

  let shaderProgram = gl.createProgram();
  gl.attachShader(shaderProgram, vertexShader);
  gl.attachShader(shaderProgram, fragmentShader);
  gl.linkProgram(shaderProgram);
  gl.useProgram(shaderProgram);

  return shaderProgram;
}


/**
 * WebGL error check wrapper - logs to console
 * @param {object} gl
 */
export function webGLErrorCheck(gl) {
  let error = gl.getError();
  let str = '';
  switch (error) {
  case gl.NO_ERROR:
    break;
  case gl.INVALID_ENUM:
    str = 'GL ERROR: gl.INVALID_ENUM: ' +
          'An unacceptable value has been ' +
          'specified for an enumerated argument. ' +
          'The command is ignored and the error flag is set.';
    break;
  case gl.INVALID_VALUE:
    str = 'GL ERROR: gl.INVALID_VALUE: ' +
          'A numeric argument is out of range. ' +
          'The command is ignored and the error flag is set.';
    break;
  case gl.INVALID_OPERATION:
    str = 'GL ERROR: gl.INVALID_OPERATION: ' +
          'The specified command is not allowed for the current ' +
          'state. The command is ignored and the error flag is set.';
    break;
  case gl.INVALID_FRAMEBUFFER_OPERATION:
    str = 'GL ERROR: gl.INVALID_FRAMEBUFFER_OPERATION: ' +
          'The currently bound framebuffer is not framebuffer ' +
          'complete when trying to render to or to read from it.';
    break;
  case gl.OUT_OF_MEMORY:
    str = 'GL ERROR: gl.OUT_OF_MEMORY: ' +
          'Not enough memory is left to execute the command.';
    break;
  case gl.CONTEXT_LOST_WEBGL:
    str = 'GL ERROR: gl.CONTEXT_LOST_WEBGL: ' +
          'If the WebGL context is lost, this error is returned ' +
          'on the first call to getError. Afterwards and until ' +
          'the context has been restored, it returns gl.NO_ERROR.';
    break;
  default:
    str = 'GL ERROR: UNRECOGNIZED ERROR TYPE: ' + error;
    break;
  }

  if (str.length > 1) {
    console.log(str);
    throw new Error(str);
  }
};
