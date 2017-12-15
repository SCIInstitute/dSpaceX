// dSpaceX.js implements UI functions


function postMessage(mesg)
{
  var botm = document.getElementById("botmframe");
  
  var text = document.createTextNode(mesg);
  botm.insertBefore(text, botm.lastChild);
  
  var br = document.createElement("br");
  botm.insertBefore(br, botm.lastChild);
  
  br.scrollIntoView();
}


function resizeFrames()
{
  // get the size of the client
  var body = document.getElementById("mainBody");
  var bodyWidth  = body.clientWidth  - 20;
  var bodyHeight = body.clientHeight - 20;
  
  // get the elements associated with the frames and the canvas
  var left = document.getElementById("leftframe");
  var rite = document.getElementById("riteframe");
  var midl = document.getElementById("midlframe");
  var botm = document.getElementById("botmframe");
  var keyf = document.getElementById("keyframe");
  var canv = document.getElementById(wv.canvasID);
  var can2 = document.getElementById(wv.canvas2D);
  if (wv.canvasKY != undefined)
    var canf = document.getElementById(wv.canvasKY);
  
  // compute and set the widths of the frames
  //    (do not make leftframe larger than 250px)
  var leftWidth = Math.round(0.25 * bodyWidth);
  if (leftWidth > 250)   leftWidth = 250;
  var riteWidth = (bodyWidth - leftWidth)/2;
  var midlWidth = riteWidth;
  var canvWidth = riteWidth - 20;
  var can2Width = midlWidth - 20;
  var keyfWidth = leftWidth - 20;
  
  left.style.width = leftWidth             + "px";
  rite.style.width = riteWidth             + "px";
  midl.style.width = midlWidth             + "px";
  botm.style.width = riteWidth + midlWidth + "px";
  keyf.style.width = leftWidth             + "px";
  canv.style.width = canvWidth             + "px";
  canv.width       = canvWidth;
  can2.style.width = can2Width             + "px";
  can2.width       = can2Width;
  if (wv.canvasKY != undefined) {
    canf.style.width = keyfWidth           + "px";
    canf.width       = keyfWidth;
  }
  
  // compute and set the heights of the frames
  //    (do not make botm frame larger than 200px)
  var botmHeight = Math.round(0.20 * bodyHeight);
  if (botmHeight > 200)   botmHeight = 200;
  var  topHeight = bodyHeight - botmHeight;
  var canvHeight =  topHeight - 25;
  var keyfHeight = botmHeight - 25;
  
  left.style.height =  topHeight    + "px";
  rite.style.height =  topHeight    + "px";
  midl.style.height =  topHeight    + "px";
  botm.style.height = botmHeight    + "px";
  keyf.style.height = botmHeight    + "px";
  canv.style.height = canvHeight-25 + "px";
  canv.height       = canvHeight-25;
  can2.style.height = canvHeight    + "px";
  can2.height       = canvHeight;
  if (wv.canvasKY != undefined) {
    canf.style.height = keyfHeight  + "px";
    canf.height       = keyfHeight;
//  force a key redraw
    wv.drawKey = 1;
  }
}

//
// Event Handlers 3D
//

function getCursorXY(e)
{
  if (!e) var e = event;
  
  wv.cursorX  = e.clientX;
  wv.cursorY  = e.clientY;
  wv.cursorX -= wv.offLeft + 1;
  wv.cursorY  = wv.height - wv.cursorY + wv.offTop + 1;
  
  wv.modifier = 0;
  if (e.shiftKey) wv.modifier |= 1;
  if (e.altKey  ) wv.modifier |= 2;
  if (e.ctrlKey ) wv.modifier |= 4;
}


function getMouseDown(e)
{
  if (!e) var e = event;
  
  wv.startX   = e.clientX;
  wv.startY   = e.clientY;
  wv.startX  -= wv.offLeft + 1;
  wv.startY   = wv.height - wv.startY + wv.offTop + 1;
  
  wv.dragging = true;
  wv.button   = e.button;
  
  wv.modifier = 0;
  if (e.shiftKey) wv.modifier |= 1;
  if (e.altKey  ) wv.modifier |= 2;
  if (e.ctrlKey ) wv.modifier |= 4;
}


function getMouseUp(e)
{
  wv.dragging = false;
}


function mouseLeftCanvas(e)
{
  if (wv.dragging) wv.dragging = false;
}


function getKeyPress(e)
{
  if (!e) var e = event;
  
  wv.keyPress = e.charCode;
}

//
// Event Handlers 2D
//

function getCursorXY2D(e)
{
  if (!e) var e = event;

//  postMessage("2D mousemove = [" + e.clientX + "," + e.clientY + "]");
  wv.cursrX2D  = e.clientX;
  wv.cursrY2D  = e.clientY;
  wv.cursrX2D -= wv.offLft2D;
  wv.cursrY2D  = wv.height2D - wv.cursrY2D + wv.offTop2D - 1;
  
  wv.modifier = 0;
  if (e.shiftKey) wv.modifier |= 1;
  if (e.altKey  ) wv.modifier |= 2;
  if (e.ctrlKey ) wv.modifier |= 4;
}


function getMouseDown2D(e)
{
  if (!e) var e = event;
  
//  postMessage("2D mousedown = [" + e.clientX + "," + e.clientY +
//              "]   button = " + e.button);
//  postMessage("        " + wv.height2D + "  " + wv.offTop2D);
  wv.startX2D  = e.clientX;
  wv.startY2D  = e.clientY;
  wv.startX2D -= wv.offLft2D;
  wv.startY2D  = wv.height2D - wv.startY2D + wv.offTop2D - 1;
  
  wv.drag2D    = true;
  wv.button    = e.button;
  
  wv.modifier  = 0;
  if (e.shiftKey) wv.modifier |= 1;
  if (e.altKey  ) wv.modifier |= 2;
  if (e.ctrlKey ) wv.modifier |= 4;
}


function getMouseUp2D(e)
{
  wv.drag2D = false;
}


function mouseLeftCanv2D(e)
{
  if (wv.drag2D) wv.drag2D = false;
}


//
// slider handler
//

function updateSlider(slideAmount)
{
  postMessage("slider value = " + slideAmount);
  wv.socketUt.send("slide " + slideAmount);
}


//
// call-back functions
//

function wvInitUI()
{

    // set up extra storage for matrix-matrix multiplies
    wv.uiMatrix   = new J3DIMatrix4();
    wv.uiMatrix2D = new J3DIMatrix4();

                                  // ui cursor variables
    wv.cursorX  = -1;              // current cursor position
    wv.cursorY  = -1;
    wv.keyPress = -1;              // last key pressed
    wv.startX   = -1;              // start of dragging position
    wv.startY   = -1;
    wv.button   = -1;              // button pressed
    wv.modifier =  0;              // modifier (shift,alt,cntl) bitflag
    wv.offTop   =  0;              // offset to upper-left corner of the canvas
    wv.offLeft  =  0;
    wv.dragging = false;           // true during drag operation
    wv.picking  =  0;              // keycode of command that turns picking on
    wv.txtInit  = "initialize";    // message sent at text protocol initialize
    wv.sgUpdate =  1;              // sceneGraph has been updated
//  wv.pick                        // set to 1 to turn picking on
//  wv.picked                      // sceneGraph object that was picked
//  wv.locate                      // set to 1 to turn on locating
//  wv.located                     // coordinates (local) that were pointed at
//  wv.centerV                     // set to 1 to put wv into view centering mode
//  wv.sceneUpd                    // set to 1 when scene needs rendering
    wv.cursrX2D = -1;              // current cursor position
    wv.cursrY2D = -1;
    wv.startX2D = -1;              // start of dragging position
    wv.startY2D = -1;
    wv.off2DTop =  0;              // offset to upper-left corner of 2D canvas
    wv.off2DLft =  0;
    wv.width2D  = -1;              // 2D "canvas" size
    wv.height2D = -1;
    wv.drag2D   = false;           // true during drag operation
    wv.sceneU2D =  1;              // 2D scene Update flag

    var canvas = document.getElementById(wv.canvasID);
      canvas.addEventListener('mousemove',  getCursorXY,     false);
      canvas.addEventListener('mousedown',  getMouseDown,    false);
      canvas.addEventListener('mouseup',    getMouseUp,      false);
      canvas.addEventListener('mouseout',   mouseLeftCanvas, false);
    var canv2D = document.getElementById(wv.canvas2D);
      canv2D.addEventListener('mousemove',  getCursorXY2D,   false);
      canv2D.addEventListener('mousedown',  getMouseDown2D,  false);
      canv2D.addEventListener('mouseup',    getMouseUp2D,    false);
      canv2D.addEventListener('mouseout',   mouseLeftCanv2D, false);
    document.addEventListener('keypress',   getKeyPress,     false);
}


function wvUpdateUI()
{

    // special code for delayed-picking mode
    if (wv.picking > 0) {

        // if something is picked, post a message
        if (wv.picked !== undefined) {

            // second part of 'q' operation
            if (wv.picking == 113) {
                postMessage("Picked: " + wv.picked.gprim);
                wv.socketUt.send("Picked: " + wv.picked.gprim);
            }

            wv.picked  = undefined;
            wv.picking = 0;
            wv.pick    = 0;

        // abort picking on mouse motion
        } else if (wv.dragging) {
            postMessage("Picking aborted");

            wv.picking = 0;
            wv.pick    = 0;
        }

        wv.keyPress = -1;
        wv.dragging = false;
    }

    // if the tree has not been created but the scene graph (possibly) exists...
    if (wv.sgUpdate == 1 && (wv.sceneGraph !== undefined)) {
      rebuildTreeWindow();
    }

    // deal with key presses
    if (wv.keyPress != -1) {

        // '?' -- help
        if (wv.keyPress ==  63) {
            postMessage("C - next case");
            postMessage("N - next scalar");
            postMessage("L - change scalar limits");
            postMessage("q - query at cursor");
            postMessage("m - set view matrix");
            postMessage("M - current view matrix");
            postMessage("x - view from -X direction");
            postMessage("X - view from +X direction");
            postMessage("y - view from -Y direction");
            postMessage("Y - view from +Y direction");
            postMessage("z - view from -Z direction");
            postMessage("Z - view from +Z direction");
            postMessage("* - center view at cursor");
            postMessage("? - get help");
        
        // "C" -- case
        } else if (wv.keyPress == 67) {
            wv.socketUt.send("case");
          
        // "l" -- 2D locate
        } else if (wv.keyPress == 108) {
          var fact = wv.width2D/wv.height2D;
          var scrX = -1.0 + 2.0*wv.cursrX2D/(wv.width2D -1);
          var scrY = -1.0 + 2.0*wv.cursrY2D/(wv.height2D-1);
          var matrix2D = new J3DIMatrix4();
          matrix2D.load(wv.orthoMatrix2D);
          matrix2D.multiply(wv.mvMatrix2D);
          matrix2D.invert();
          var vec = new J3DIVector3(scrX, scrY*fact, 0.0);
          vec.multVecMatrix(matrix2D);
          postMessage(" locate2D = " + vec[0] + " " + vec[1]/fact);
          
        // 'T' -- thumbnail
        } else if (wv.keyPress == 84) {
            postMessage("Ask for ThumbNail...");
            wv.socketUt.send("thumbnail");
          
        // 'M' -- dump view matrix
        } else if (wv.keyPress == 77) {
            var mVal = wv.mvMatrix.getAsArray();
            postMessage(" View Matrix (scale = " + wv.scale + "):");
            postMessage("  "+mVal[ 0]+" "+mVal[ 1]+" "+mVal[ 2]+" "+mVal[ 3]);
            postMessage("  "+mVal[ 4]+" "+mVal[ 5]+" "+mVal[ 6]+" "+mVal[ 7]);
            postMessage("  "+mVal[ 8]+" "+mVal[ 9]+" "+mVal[10]+" "+mVal[11]);
            postMessage("  "+mVal[12]+" "+mVal[13]+" "+mVal[14]+" "+mVal[15]);
      
        // 'N' -- next scalar
        } else if (wv.keyPress == 78) {
            wv.socketUt.send("next");
      
        // 'L' -- scalar limits
        } else if (wv.keyPress == 76) {
            wv.socketUt.send("limits");
        
        // 'm' -- set view matrix
        } else if (wv.keyPress == 109) {
            var mVal  = wv.mvMatrix.getAsArray();
            wv.scale  = parseFloat(prompt("Enter scale", wv.scale));
            var strng = prompt("Matrix Pos  0- 3",
                               mVal[ 0]+" "+mVal[ 1]+" "+mVal[ 2]+" "+mVal[ 3]);
            var split = strng.split(' ');
            var len   = split.length;
            if (len > 4) len = 4;
            for (var m =  0; m <    len; m++) mVal[m] = parseFloat(split[m   ]);
            strng = prompt("Matrix Pos  4- 7",
                               mVal[ 4]+" "+mVal[ 5]+" "+mVal[ 6]+" "+mVal[ 7]);
            split = strng.split(' ');
            len   = split.length;
            if (len > 4) len = 4;
            for (var m =  4; m <  4+len; m++) mVal[m] = parseFloat(split[m- 4]);
            strng = prompt("Matrix Pos  8-11",
                               mVal[ 8]+" "+mVal[ 9]+" "+mVal[10]+" "+mVal[11]);
            split = strng.split(' ');
            len   = split.length;
            if (len > 4) len = 4;
            for (var m =  8; m <  8+len; m++) mVal[m] = parseFloat(split[m- 8]);
            strng = prompt("Matrix Pos 12-15",
                               mVal[12]+" "+mVal[13]+" "+mVal[14]+" "+mVal[15]);
            split = strng.split(' ');
            len   = split.length;
            if (len > 4) len = 4;
            for (var m = 12; m < 12+len; m++) mVal[m] = parseFloat(split[m-12]);
            wv.mvMatrix.load(mVal);
            wv.sceneUpd = 1;

        // 'q' -- query at cursor
        } else if (wv.keyPress == 113) {
            wv.picking  = 113;
            wv.pick     = 1;
            wv.sceneUpd = 1;

        // 'x' -- view from -X direction
        } else if (wv.keyPress == 120) {
            wv.mvMatrix.makeIdentity();
            wv.mvMatrix.rotate(+90, 0,1,0);
            wv.sceneUpd = 1;

        // 'X' -- view from +X direction
        } else if (wv.keyPress ==  88) {
            wv.mvMatrix.makeIdentity();
            wv.mvMatrix.rotate(-90, 0,1,0);
            wv.sceneUpd = 1;

        // 'y' -- view from +Y direction
        } else if (wv.keyPress == 121) {
            wv.mvMatrix.makeIdentity();
            wv.mvMatrix.rotate(-90, 1,0,0);
            wv.sceneUpd = 1;

        // 'Y' -- view from +Y direction
        } else if (wv.keyPress ==  89) {
            wv.mvMatrix.makeIdentity();
            wv.mvMatrix.rotate(+90, 1,0,0);
            wv.sceneUpd = 1;

        // 'z' -- view from +Z direction
        } else if (wv.keyPress ==  122) {
            wv.mvMatrix.makeIdentity();
            wv.mvMatrix.rotate(180, 1,0,0);
            wv.sceneUpd = 1;

        // 'Z' -- view from +Z direction
        } else if (wv.keyPress ==  90) {
            wv.mvMatrix.makeIdentity();
            wv.sceneUpd = 1;
          
        // '*' -- center view at cursor
        } else if (wv.keyPress == 42) {
            wv.centerV = 1;

        } else {
            postMessage("'" + String.fromCharCode(wv.keyPress)
                        + "' is not defined.  Use '?' for help.");
        }
    }

    wv.keyPress = -1;

    // UI is in screen coordinates (not object)
    wv.uiMatrix.load(wv.mvMatrix);
    wv.mvMatrix.makeIdentity();
    wv.uiMatrix2D.load(wv.mvMatrix2D);
    wv.mvMatrix2D.makeIdentity();

    // deal with mouse movement
    if (wv.dragging) {

        // cntrl is down (rotate)
        if (wv.modifier == 4) {
            var angleX =  (wv.startY - wv.cursorY) / 4.0;
            var angleY = -(wv.startX - wv.cursorX) / 4.0;
            if ((angleX != 0.0) || (angleY != 0.0)) {
              wv.mvMatrix.rotate(angleX, 1,0,0);
              wv.mvMatrix.rotate(angleY, 0,1,0);
              wv.sceneUpd = 1;
            }

        // alt-shift is down (rotate)
        } else if (wv.modifier == 3) {
            var angleX =  (wv.startY - wv.cursorY) / 4.0;
            var angleY = -(wv.startX - wv.cursorX) / 4.0;
            if ((angleX != 0.0) || (angleY != 0.0)) {
              wv.mvMatrix.rotate(angleX, 1,0,0);
              wv.mvMatrix.rotate(angleY, 0,1,0);
              wv.sceneUpd = 1;
            }

        // alt is down (spin)
        } else if (wv.modifier == 2) {
            var xf = wv.startX - wv.width  / 2;
            var yf = wv.startY - wv.height / 2;

            if ((xf != 0.0) || (yf != 0.0)) {
                var theta1 = Math.atan2(yf, xf);
                xf = wv.cursorX - wv.width  / 2;
                yf = wv.cursorY - wv.height / 2;

                if ((xf != 0.0) || (yf != 0.0)) {
                    var dtheta = Math.atan2(yf, xf)-theta1;
                    if (Math.abs(dtheta) < 1.5708) {
                        var angleZ = 128*(dtheta)/3.1415926;
                        wv.mvMatrix.rotate(angleZ, 0,0,1);
                        wv.sceneUpd = 1;
                    }
                }
            }

        // shift is down (zoom)
        } else if (wv.modifier == 1) {
            if (wv.cursorY != wv.startY) {
              var scale = Math.exp((wv.cursorY - wv.startY) / 512.0);
              wv.mvMatrix.scale(scale, scale, scale);
              wv.scale   *= scale;
              wv.sceneUpd = 1;
            }

        // no modifier (translate)
        } else {
            var transX = (wv.cursorX - wv.startX) / 256.0;
            var transY = (wv.cursorY - wv.startY) / 256.0;
            if ((transX != 0.0) || (transY != 0.0)) {
              wv.mvMatrix.translate(transX, transY, 0.0);
              wv.sceneUpd = 1;
            }
        }

        wv.startX = wv.cursorX;
        wv.startY = wv.cursorY;
    }
  
      // deal with mouse movement -- 2D
    if (wv.drag2D) {

        // shift is down (zoom)
        if (wv.modifier == 1) {
            if (wv.cursrY2D != wv.startY2D) {
              var scale = Math.exp((wv.cursrY2D - wv.startY2D) / 512.0);
              wv.mvMatrix2D.scale(scale, scale, 1.0);
              wv.sceneU2D = 1;
            }

        // no modifier (translate)
        } else if (wv.modifier == 0) {
            var transX = (wv.cursrX2D - wv.startX2D) / wv.width2D;
            var transY = (wv.cursrY2D - wv.startY2D) / wv.height2D;
            if ((transX != 0.0) || (transY != 0.0)) {
              wv.mvMatrix2D.translate(2.0*transX, 2.0*transY, 0.0);
              wv.sceneU2D = 1;
            }
        }

        wv.startX2D = wv.cursrX2D;
        wv.startY2D = wv.cursrY2D;
    }
  
}


function wvServerMessage(text)
{
//  postMessage(" Server Message: " + text);
    var tokens = text.split(':');
    if (tokens[0] == "Error") {
        alert(text);
        return;
    }
  
    if (tokens[0] == "Case") {
        wv.params  = new Array();
        wv.qois    = new Array();
        wv.caseNum = tokens[1];
        var nParam = tokens[2];
        var nQoI   = tokens[3];
        var iTok   = 4;
        for (var i = 0; i < nParam; i++) {
            wv.params[i]       = new Array();
            wv.params[i].name  = tokens[iTok  ];
            wv.params[i].value = tokens[iTok+1];
            iTok += 2;
        }
        for (var i = 0; i < nQoI; i++) {
            wv.qois[i]       = new Array();
            wv.qois[i].name  = tokens[iTok  ];
            wv.qois[i].value = tokens[iTok+1];
            iTok += 2;
        }
        wv.sgUpdate = 1;
        return;
    }
  
}


function wvServerDown()
{
    alert("The server has terminated.  You will need to restart the server"
         +" and this browser in order to continue.");
}
