// Utility functions

/**
 * WebGL error check wrapper - logs to console
 * @param {object} gl
 */
export let webGLErrorCheck = function(gl) {
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
