/*
 *      wv: The Web Viewer
 *
 *              call-back Manager functions
 *
 *      Copyright 2011-2017, Massachusetts Institute of Technology
 *      Licensed under The GNU Lesser General Public License, version 2.1
 *      See http://www.opensource.org/licenses/lgpl-2.1.php
 *
 */


//
// Default functions:
//
// Initialize the ui variables
//

wv["InitUI"] = function()
{
  // set up extra storage for matrix-matrix multiplies
  wv.uiMatrix = new J3DIMatrix4();
}


//
// allows for other drawings into the WebGL canvas
//

wv["UpdateCanvas"] = function(gl)
{
}


//
// gets invoked when time to update the ui state from changes due to events
//

wv["UpdateUI"] = function()
{
}


//
// allows for the view matrix update after delta ui manipulations
//

wv["UpdateView"] = function()
{
    if (wv.uiMatrix !== undefined) wv.mvMatrix.multiply(wv.uiMatrix);
}


//
// handles server messages
//

wv["ServerMessage"] = function(text)
{
    wv.logger(" Server Message: " + text);
}


//
// allows for initializing a WebGL 2D canvas
//

wv["InitCanvas2D"] = function(gl)
{
}


//
// allows for drawing a WebGL 2D canvas
//

wv["DrawPicture2D"] = function(gl)
{
}


//
// needed when the canvas size changes or relocates
//

wv["Reshape"] = function(gl)
{

    var canvas = document.getElementById(wv.canvasID);
    if (wv.offTop != canvas.offsetTop || wv.offLeft != canvas.offsetLeft) {
        wv.offTop  = canvas.offsetTop;
        wv.offLeft = canvas.offsetLeft;
    }

    if (wv.width == canvas.width && wv.height == canvas.height) return;

    wv.width  = canvas.width;
    wv.height = canvas.height;

    // Set the viewport and projection matrix for the scene
    gl.viewport(0, 0, wv.width, wv.height);
    wv.perspectiveMatrix = new J3DIMatrix4();
    wv.sceneUpd = 1;

    wv.InitDraw();
}


//
// overrides the default callback code (any of the above)
//

wv["setCallback"] = function(cbtype, fn)
{
  if (wv.toType(fn) != "Function") {
    wv.logger(" Error: setCallback called with Obj Type = " + wv.toType(fn));
    return;
  }

  if (cbtype == "InitUI") {
    wv.InitUI = fn;
  } else if (cbtype == "UpdateCanvas") {
    wv.UpdateCanvas = fn;
  } else if (cbtype == "UpdateUI") {
    wv.UpdateUI = fn;
  } else if (cbtype == "UpdateView") {
    wv.UpdateView = fn;
  } else if (cbtype == "ServerMessage") {
    wv.ServerMessage = fn;
  } else if (cbtype == "Reshape") {
    wv.Reshape = fn;
  } else if (cbtype == "InitCanvas2D") {
    wv.InitCanvas2D = fn;
  } else if (cbtype == "DrawPicture2D") {
    wv.DrawPicture2D = fn;
  } else {
    wv.logger(" Error: setCallback called with type = " + cbtype);
  }
}
