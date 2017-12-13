/*
 * dSpaceX 2D window handling functions
 */


// do GPtype == 10
function plotPoints2D(gl, graphic)
{

  gl.uniform3f(wv.u_conColorLoc2D,  graphic.pColor[0],
               graphic.pColor[1],   graphic.pColor[2]);
  gl.uniform1f(wv.u_pointSizeLoc2D, graphic.pSize);

  for (var i = 0; i < graphic.nStrip; i++) {
    var vbo = graphic.points[i];

    gl.disableVertexAttribArray(1);
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo.vertex);
    gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(0);
    
    if (((graphic.attrs & wv.plotAttrs.SHADING) != 0) &&
        (vbo.color != undefined)) {
      gl.bindBuffer(gl.ARRAY_BUFFER, vbo.color);
      gl.vertexAttribPointer(1, 3, gl.UNSIGNED_BYTE, false, 0, 0);
      gl.enableVertexAttribArray(1);
    } else {
      gl.uniform1f(wv.u_wColorLoc2D, 0.0);
    }

    if (vbo.index == undefined) {
      gl.drawArrays(gl.POINTS, 0, vbo.nVerts);
    } else {
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vbo.index);
      gl.drawElements(gl.POINTS, vbo.nIndices, gl.UNSIGNED_SHORT, 0);
    }
    wv.checkGLError(gl, "plotPoints - after draw on points");
    gl.uniform1f(wv.u_wColorLoc2D, 1.0);
  }
}


// do GPtype == 11
function plotLines2D(gl, graphic)
{

  //
  // do the lines
  //
  if ((graphic.attrs & wv.plotAttrs.ON) != 0) {
    gl.uniform3f(wv.u_conColorLoc2D, graphic.lColor[0],
                 graphic.lColor[1],  graphic.lColor[2]);
    gl.lineWidth(graphic.lWidth);
    for (var i = 0; i < graphic.nStrip; i++) {
      var vbo = graphic.lines[i];

      gl.disableVertexAttribArray(1);
      gl.bindBuffer(gl.ARRAY_BUFFER, vbo.vertex);
      gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(0);

      if (((graphic.attrs & wv.plotAttrs.SHADING) != 0) &&
          (vbo.color != undefined)) {
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo.color);
        gl.vertexAttribPointer(1, 3, gl.UNSIGNED_BYTE, false, 0, 0);
        gl.enableVertexAttribArray(1);
      } else {
        gl.uniform1f(wv.u_wColorLoc2D, 0.0);
      }

      if (vbo.index == undefined) {
        gl.drawArrays(gl.LINES, 0, vbo.nVerts);
      } else {
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vbo.index);
        gl.drawElements(gl.LINES, vbo.nIndices, gl.UNSIGNED_SHORT, 0);
      }
      wv.checkGLError(gl, "plotLines - after draw on lines");
      gl.uniform1f(wv.u_wColorLoc2D, 1.0);
    }
  }

  //
  // do the points
  //
  if ((graphic.attrs & wv.plotAttrs.POINTS) != 0) {
    gl.uniform1f(wv.u_wColorLoc2D,    0.0);
    gl.uniform3f(wv.u_conColorLoc2D,  graphic.pColor[0],
                 graphic.pColor[1],   graphic.pColor[2]);
    gl.uniform1f(wv.u_pointSizeLoc2D, graphic.pSize);
    gl.disableVertexAttribArray(1);
    for (var i = 0; i < graphic.nStrip; i++) {
      var vbop = graphic.points[i];
      if (vbop == undefined) continue;
      var vbol = graphic.lines[i];
      gl.bindBuffer(gl.ARRAY_BUFFER, vbol.vertex);
      gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(0);
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vbop.index);
      gl.drawElements(gl.POINTS, vbop.nIndices, gl.UNSIGNED_SHORT, 0);
      wv.checkGLError(gl, "plotLines - after draw on points");
    }
  }

}


// do GPtype == 12
function plotTriangles2D(gl, graphic)
{

  //
  // do the triangles first
  //
  if ((graphic.attrs & wv.plotAttrs.ON) != 0) {
    gl.uniform3f(wv.u_conColorLoc2D, graphic.fColor[0],
                 graphic.fColor[1],  graphic.fColor[2]);

    for (var i = 0; i < graphic.nStrip; i++) {
      var vbo = graphic.triangles[i];

      gl.disableVertexAttribArray(1);
      gl.bindBuffer(gl.ARRAY_BUFFER, vbo.vertex);
      gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(0);

      if (((graphic.attrs & wv.plotAttrs.SHADING) != 0) &&
          (vbo.color != undefined)) {
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo.color);
        gl.vertexAttribPointer(1, 3, gl.UNSIGNED_BYTE, false, 0, 0);
        gl.enableVertexAttribArray(1);
      } else {
        gl.uniform1f(wv.u_wColorLoc2D, 0.0);
      }

      if (vbo.index == undefined) {
        gl.drawArrays(gl.TRIANGLES, 0, vbo.nVerts);
      } else {
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vbo.index);
        gl.drawElements(gl.TRIANGLES, vbo.nIndices, gl.UNSIGNED_SHORT, 0);
      }
      wv.checkGLError(gl, "plotTriangles - after draw on tris");
      if (vbo.normal == undefined) gl.uniform1f(wv.u_wNormalLoc, 1.0);
    }
  }

  //
  // do the lines
  //
  if ((graphic.attrs & wv.plotAttrs.LINES) != 0) {
    gl.uniform1f(wv.u_wColorLoc2D,   0.0);
    gl.uniform3f(wv.u_conColorLoc2D, graphic.lColor[0],
                 graphic.lColor[1],  graphic.lColor[2]);
    gl.lineWidth(graphic.lWidth);
    gl.disableVertexAttribArray(1);
    for (var i = 0; i < graphic.nStrip; i++) {
      var vbol = graphic.lines[i];
      if (vbol == undefined) continue;
      var vbot = graphic.triangles[i];
      gl.bindBuffer(gl.ARRAY_BUFFER, vbot.vertex);
      gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(0);
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vbol.index);
      gl.drawElements(gl.LINES, vbol.nIndices, gl.UNSIGNED_SHORT, 0);
      wv.checkGLError(gl, "plotTriangles - after draw on lines");
    }
    gl.uniform1f(wv.u_wColorLoc2D, 1.0);
  }

  //
  // do the points
  //
  if ((graphic.attrs & wv.plotAttrs.POINTS) != 0) {
    gl.uniform1f(wv.u_wColorLoc2D,    0.0);
    gl.uniform3f(wv.u_conColorLoc2D,  graphic.pColor[0],
                 graphic.pColor[1],   graphic.pColor[2]);
    gl.uniform1f(wv.u_pointSizeLoc2D, graphic.pSize);
    gl.disableVertexAttribArray(1);
    for (var i = 0; i < graphic.nStrip; i++) {
      var vbop = graphic.points[i];
      if (vbop == undefined) continue;
      var vbot = graphic.triangles[i];
      gl.bindBuffer(gl.ARRAY_BUFFER, vbot.vertex);
      gl.vertexAttribPointer(0, 2, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(0);
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vbop.index);
      gl.drawElements(gl.POINTS, vbop.nIndices, gl.UNSIGNED_SHORT, 0);
      wv.checkGLError(gl, "plotTriangles - after draw on points");
    }
    gl.uniform1f(wv.u_wColorLoc2D, 1.0);
  }

}


//
// draws the 2D scene
function drawPicture2D(gl)
{
  // Make sure the canvas is sized correctly.
  var canvas = document.getElementById(wv.canvas2D);
  if (wv.off2DTop != canvas.offsetTop || wv.off2DLft != canvas.offsetLeft) {
    wv.offTop2D = canvas.offsetTop;
    wv.offLft2D = canvas.offsetLeft;
  }
  
  if (wv.width2D != canvas.width || wv.height2D != canvas.height) {
    wv.width2D  = canvas.width;
    wv.height2D = canvas.height;
  
    // Set the viewport for the scene
    gl.viewport(0, 0, wv.width2D, wv.height2D);
    wv.mvMatrix2D = new J3DIMatrix4();
    wv.mvMatrix2D.makeIdentity();
    wv.orthoMatrix2D.makeIdentity();
    wv.orthoMatrix2D.ortho(-1, 1, -wv.height2D/wv.width2D,
                                   wv.height2D/wv.width2D, -1, 1);
    wv.sceneU2D = 1;
  }

  if (wv.sceneU2D == 0) return;
  
  // Make a model/view matrix and pass it in
  if (wv.uiMatrix2D !== undefined) wv.mvMatrix2D.multiply(wv.uiMatrix2D);
  var matrix2D = new J3DIMatrix4();
  matrix2D.load(wv.mvMatrix2D);
  matrix2D.multiply(wv.orthoMatrix2D);
  matrix2D.setUniform(gl, wv.u_modelViewMatrixLoc2D, false);

  // Draw the scene
  gl.clear(gl.COLOR_BUFFER_BIT);
  for (var gprim in wv.sceneGraph) {
    var graphic = wv.sceneGraph[gprim];
    switch (graphic.GPtype) {
      case 10:
        if ((graphic.attrs & wv.plotAttrs.ON) == 0) break;
        plotPoints2D(gl, graphic);
        break;
      case 11:
        if (((graphic.attrs & wv.plotAttrs.ON)     == 0) &&
            ((graphic.attrs & wv.plotAttrs.POINTS) == 0)) break;
        plotLines2D(gl, graphic);
        break;
      case 12:
        if (((graphic.attrs & wv.plotAttrs.ON)     == 0) &&
            ((graphic.attrs & wv.plotAttrs.LINES)  == 0) &&
            ((graphic.attrs & wv.plotAttrs.POINTS) == 0)) break;
        plotTriangles2D(gl, graphic);
        break;
      default:
        break;
    }
  }

}


//
// Initilizes the 2D Canvas
function InitCanvas2D(gl)
{
  
  //
  // the shaders
  var vShader2D = [
"    precision mediump float;",
"    precision mediump int;",
"    uniform mat4   u_modelViewMatrix;",
"    uniform vec3   conColor;                    // constant color",
"    uniform float  wColor;                      // Constant color switch",
"    uniform float  pointSize;                   // point size in pixels",
"",
"    attribute vec4 vPosition;",
"    attribute vec3 vColor;",
"",
"    varying vec4   v_Color;",
"",
"    void main()",
"    {",
"        // set the pixel position",
"        gl_Position  = u_modelViewMatrix * vPosition;",
"        gl_PointSize = pointSize; // set the point size",
"        // assumes that colors are coming in as unsigned bytes",
"        vec4 color = vec4(vColor/255.0, 1.0);",
"        v_Color    = color*wColor + vec4(conColor,1.0)*(1.0-wColor);",
"    }"
  ].join("\n");

  var fShader2D = [
"    precision mediump float;",
"    precision mediump int;",
"",
"    varying vec4  v_Color;",
"",
"    void main()",
"    {",
"        gl_FragColor = v_Color;",
"    }"
  ].join("\n");

  //
  // setup the shaders and other stuff for rendering
  wv.program2D = wv.Setup(gl,
                          // The sources of the vertex and fragment shaders.
                          vShader2D, fShader2D,
                          // The vertex attribute names used by the shaders.
                          // The order they appear here corresponds to their indices.
                          [ "vPosition", "vColor" ],
                          // The clear color and depth values.
                          [ 0.0, 0.0, 0.0, 0.0 ], undefined);

  //
  // Set up the uniform variables for the shaders

  wv.u_conColorLoc2D = gl.getUniformLocation(wv.program2D,  "conColor");
  gl.uniform3f(wv.u_conColorLoc2D, 0.0, 1.0, 0.0);
  wv.u_wColorLoc2D = gl.getUniformLocation(wv.program2D,    "wColor");
  gl.uniform1f(wv.u_wColorLoc2D, 1.0);
  wv.u_pointSizeLoc2D = gl.getUniformLocation(wv.program2D, "pointSize");
  gl.uniform1f(wv.u_pointSizeLoc2D, 2.0);

  //
  // Create some matrices to use later and save their locations
  wv.u_modelViewMatrixLoc2D =
                      gl.getUniformLocation(wv.program2D, "u_modelViewMatrix");
  wv.orthoMatrix2D = new J3DIMatrix4();
  wv.mvMatrix2D    = new J3DIMatrix4();
  wv.mvMatrix2D.makeIdentity();

}
