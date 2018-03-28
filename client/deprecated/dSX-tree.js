//
// dSX-tree -- API for left hand frame control
//
"use strict";


//
// callback to toggle Viz property
//
function toggleViz(e) {
    // alert("in toggleViz(e="+e+")");

    // get the Tree Node
    var inode = e["target"].id.substring(4);
    inode     = inode.substring(0,inode.length-4);
    inode     = Number(inode);

    // toggle the Viz property
    if (myTree.gprim[inode] != "") {
        if ((wv.sceneGraph[myTree.gprim[inode]].attrs & wv.plotAttrs.ON) == 0) {
            myTree.prop(inode, 1, "on");
        } else {
            myTree.prop(inode, 1, "off");
        }

    //  toggle the Viz property (on all Core/Tube in this Crystal)
    } else {
        var myElem = myTree.document.getElementById("node"+inode+"col3");
        if (myElem.getAttribute("class") == "fakelinkoff") {
            myTree.prop(inode, 1, "on");
        } else {
            myTree.prop(inode, 1, "off");
        }
    }

    myTree.update();
}


//
// callback to toggle Grd property
//
function toggleGrd(e) {
    // alert("in toggleGrd(e="+e+")");

    // get the Tree Node
    var inode = e["target"].id.substring(4);
    inode     = inode.substring(0,inode.length-4);
    inode     = Number(inode);

    // toggle the Grd property
    if (myTree.gprim[inode] != "") {
        if ((wv.sceneGraph[myTree.gprim[inode]].attrs & wv.plotAttrs.LINES) == 0) {
            myTree.prop(inode, 2, "on");
        } else {
            myTree.prop(inode, 2, "off");
        }

    // toggle the Grd property (on all Core/Tube in this Crystal)
    } else {
        var myElem = myTree.document.getElementById("node"+inode+"col4");
        if (myElem.getAttribute("class") == "fakelinkoff") {
            myTree.prop(inode, 2, "on");
        } else {
            myTree.prop(inode, 2, "off");
        }
    }

    myTree.update();
}


//
// callback to toggle Trn property
//
function toggleTrn(e) {
    // alert("in toggleTrn(e="+e+")");

    // get the Tree Node
    var inode = e["target"].id.substring(4);
    inode     = inode.substring(0,inode.length-4);
    inode     = Number(inode);

    // toggle the Trn property
    if (myTree.gprim[inode] != "") {
        if ((wv.sceneGraph[myTree.gprim[inode]].attrs & wv.plotAttrs.TRANSPARENT) == 0) {
            myTree.prop(inode, 3, "on");
        } else {
            myTree.prop(inode, 3, "off");
        }

    // toggle the Trn property (on all)
    } else {
        var myElem = myTree.document.getElementById("node"+inode+"col5");
        if (myElem.getAttribute("class") == "fakelinkoff") {
            myTree.prop(inode, 3, "on");
            myElem.setAttribute("class", "fakelinkon");
            myElem.title = "Toggle Trn off";
        } else {
            myTree.prop(inode, 3, "off");
            myElem.setAttribute("class", "fakelinkoff");
            myElem.title = "Toggle Trn on";
        }
    }

    myTree.update();
}


//
// callback to toggle Ori property
//
function toggleOri(e) {
    // alert("in toggleOri(e="+e+")");

    // get the Tree Node
    var inode = e["target"].id.substring(4);
    inode     = inode.substring(0,inode.length-4);
    inode     = Number(inode);

    // toggle the Ori property
    if (myTree.gprim[inode] != "") {
        if ((wv.sceneGraph[myTree.gprim[inode]].attrs & wv.plotAttrs.ORIENTATION) == 0) {
            myTree.prop(inode, 3, "on");
        } else {
            myTree.prop(inode, 3, "off");
        }

    // toggle the Ori property (on all)
    } else {
        var myElem = myTree.document.getElementById("node"+inode+"col5");
        if (myElem.getAttribute("class") == "fakelinkoff") {
            myTree.prop(inode, 3, "on");
            myElem.setAttribute("class", "fakelinkon");
            myElem.title = "Toggle Ori off";
        } else {
            myTree.prop(inode, 3, "off");
            myElem.setAttribute("class", "fakelinkoff");
            myElem.title = "Toggle Ori on";
        }
    }

    myTree.update();
}


//
// constructor for a Tree
//
function Tree(doc, treeId) {
    // alert("in Tree(doc="+doc+", treeId="+treeId+")");

    // remember the document
    this.document = doc;
    this.treeId   = treeId;

    // arrays to hold the Nodes
    this.name    = new Array();
    this.tooltip = new Array();
    this.gprim   = new Array();
    this.click   = new Array();
    this.parent  = new Array();
    this.child   = new Array();
    this.next    = new Array();
    this.nprop   = new Array();
    this.opened  = new Array();

    this.prop1  = new Array();
    this.cbck1  = new Array();
    this.prop2  = new Array();
    this.cbck2  = new Array();
    this.prop3  = new Array();
    this.cbck3  = new Array();

    // initialize Node=0 (the root)
    this.name[  0]  = "**root**";
    this.tooltip[0] = "";
    this.gprim[ 0]  = "";
    this.click[ 0]  = null;
    this.parent[0]  = -1;
    this.child[ 0]  = -1;
    this.next[  0]  = -1;
    this.nprop[ 0]  =  0;
    this.prop1[ 0]  = "";
    this.cbck1[ 0]  = null;
    this.prop2[ 0]  = "";
    this.cbck2[ 0]  = null;
    this.prop3[ 0]  = "";
    this.cbck3[ 0]  = null;
    this.opened[0]  = +1;

    // add methods
    this.addNode  = TreeAddNode;
    this.expand   = TreeExpand;
    this.contract = TreeContract;
    this.prop     = TreeProp;
    this.clear    = TreeClear;
    this.build    = TreeBuild;
    this.update   = TreeUpdate;
}


//
// add a Node to the Tree
//
function TreeAddNode(iparent, name, tooltip, gprim, click,
                     prop1, cbck1,
                     prop2, cbck2,
                     prop3, cbck3)
{
    // alert("in TreeAddNode(iparent="+iparent+", name="+name+", tooltip="+tooltip+", gprim="+gprim+
    //                       ", click="+click+
    //                       ", prop1="+prop1+", cbck1="+cbck1+
    //                       ", prop2="+prop2+", cbck2="+cbck2+
    //                       ", prop3="+prop3+", cbck3="+cbck3+")");

    // validate the input
    if (iparent < 0 || iparent >= this.name.length) {
        alert("iparent="+iparent+" is out of range");
        return;
    }

    // find the next Node index
    var inode = this.name.length;

    // store this Node's values
    this.name[   inode] = name;
    this.tooltip[inode] = tooltip;
    this.gprim[  inode] = gprim;
    this.click[  inode] = click;
    this.parent[ inode] = iparent;
    this.child[  inode] = -1;
    this.next[   inode] = -1;
    this.nprop[  inode] =  0;
    this.opened[ inode] =  0;

    // store the properties
    if (prop1 !== undefined) {
        this.nprop[inode] = 1;
        this.prop1[inode] = prop1;
        this.cbck1[inode] = cbck1;
    }

    if (prop2 !== undefined) {
        this.nprop[inode] = 2;
        this.prop2[inode] = prop2;
        this.cbck2[inode] = cbck2;
    }

    if (prop3 !== undefined) {
        this.nprop[inode] = 3;
        this.prop3[inode] = prop3;
        this.cbck3[inode] = cbck3;
    }

    // if the parent does not have a child, link this
    //    new Node to the parent
    if (this.child[iparent] < 0) {
        this.child[iparent] = inode;

    // otherwise link this Node to the last parent's child
    } else {
        var jnode = this.child[iparent];
        while (this.next[jnode] >= 0) {
            jnode = this.next[jnode];
        }

        this.next[jnode] = inode;
    }
}


//
// build the Tree (ie, create the html table from the Nodes)
//
function TreeBuild() {
    // alert("in TreeBuild()");

    var doc = this.document;

    // if the table already exists, delete it and all its children (3 levels)
    var thisTable = doc.getElementById(this.treeId);
    if (thisTable) {
        var child1 = thisTable.lastChild;
        while (child1) {
            var child2 = child1.lastChild;
            while (child2) {
                var child3 = child2.lastChild;
                while (child3) {
                    child2.removeChild(child3);
                    child3 = child2.lastChild;
                }
                child1.removeChild(child2);
                child2 = child1.lastChild;
            }
            thisTable.removeChild(child1);
            child1 = thisTable.lastChild;
        }
        thisTable.parentNode.removeChild(thisTable);
    }

    // build the new table
    var newTable = doc.createElement("table");
    newTable.setAttribute("id", this.treeId);
    doc.getElementById("leftframe").appendChild(newTable);

    // traverse the Nodes using depth-first search
    var inode = 1;
    while (inode > 0) {

        // table row "node"+inode
        var newTR = doc.createElement("TR");
        newTR.setAttribute("id", "node"+inode);
        newTable.appendChild(newTR);

        // table data "node"+inode+"col1"
        var newTDcol1 = doc.createElement("TD");
        newTDcol1.setAttribute("id", "node"+inode+"col1");
        newTR.appendChild(newTDcol1);

        var newTexta = doc.createTextNode("");
        newTDcol1.appendChild(newTexta);

        // table data "node"+inode+"col2"
        var newTDcol2 = doc.createElement("TD");
        newTDcol2.setAttribute("id", "node"+inode+"col2");
        if (this.click[inode] != null) {
            newTDcol2.className = "fakelinkcmenu";
            if (this.tooltip[inode].length > 0) {
                newTDcol2.title = this.tooltip[inode];
            }
        }
        newTR.appendChild(newTDcol2);

        var newTextb = doc.createTextNode(this.name[inode]);
        newTDcol2.appendChild(newTextb);

        var name = this.name[inode].replace(/\u00a0/g, "");

        // table data "node"+inode+"col3"
        if (this.nprop[inode] > 0) {
            var newTDcol3 = doc.createElement("TD");
            newTDcol3.setAttribute("id", "node"+inode+"col3");
            if (this.cbck1[inode] != "") {
                newTDcol3.className = "fakelinkon";
            }
            newTR.appendChild(newTDcol3);

            if (this.nprop[inode] == 1) {
                newTDcol3.setAttribute("colspan", "3");
            }

            var newTextc = doc.createTextNode(this.prop1[inode]);
            newTDcol3.appendChild(newTextc);
        }

        // table data "node:+inode+"col4"
        if (this.nprop[inode] > 1) {
            var newTDcol4 = doc.createElement("TD");
            newTDcol4.setAttribute("id", "node"+inode+"col4");
            if (this.cbck2[inode] != "") {
                newTDcol4.className = "fakelinkoff";
            }
            newTR.appendChild(newTDcol4);

            if (this.nprop[inode] == 2) {
                newTDcol4.setAttribute("colspan", "2");
            }

            var newTextd = doc.createTextNode(this.prop2[inode]);
            newTDcol4.appendChild(newTextd);
        }

        // table data "node:+inode+"col5"
        if (this.nprop[inode] > 2) {
            var newTDcol5 = doc.createElement("TD");
            newTDcol5.setAttribute("id", "node"+inode+"col5");
            if (this.cbck3[inode] != "") {
                if (this.name[inode] == "\u00a0\u00a0\u00a0\u00a0Tubes") {
                    newTDcol5.className = "fakelinkon";
                } else {
                    newTDcol5.className = "fakelinkoff";
                }
            }
            newTR.appendChild(newTDcol5);

            var newTextd = doc.createTextNode(this.prop3[inode]);
            newTDcol5.appendChild(newTextd);
        }

        // go to next row
        if        (this.child[inode] >= 0) {
            inode = this.child[inode];
        } else if (this.next[inode] >= 0) {
            inode = this.next[inode];
        } else {
            while (inode > 0) {
                inode = this.parent[inode];
                if (this.parent[inode] == 0) {
                    newTR = doc.createElement("TR");
                    newTR.setAttribute("height", "10px");
                    newTable.appendChild(newTR);
                }
                if (this.next[inode] >= 0) {
                    inode = this.next[inode];
                    break;
                }
            }
        }
    }

    this.update();
}


//
// clear the Tree
//
function TreeClear() {
    // alert("in TreeClear()");

    // remove all but the first Node
    this.name.splice(   1);
    this.tooltip.splice(1);
    this.gprim.splice(  1);
    this.click.splice(  1);
    this.parent.splice( 1);
    this.child.splice(  1);
    this.next.splice(   1);
    this.nprop.splice(  1);
    this.opened.splice( 1);

    this.prop1.splice(1);
    this.cbck1.splice(1);
    this.prop2.splice(1);
    this.cbck2.splice(1);
    this.prop3.splice(1);
    this.cbck3.splice(1);

    // reset the root Node
    this.parent[0] = -1;
    this.child[ 0] = -1;
    this.next[  0] = -1;
}


//
// expand a Node in the Tree
//
function TreeContract(inode) {
    // alert("in TreeContract(inode="+inode+")");

    // validate inputs
    if (inode < 0 || inode >= this.opened.length) {
        alert("inode="+inode+" is out of range");
        return;
    }

    // contract inode
    this.opened[inode] = 0;

    // contract all descendents of inode
    for (var jnode = 1; jnode < this.parent.length; jnode++) {
        var iparent = this.parent[jnode];
        while (iparent > 0) {
            if (iparent == inode) {
                this.opened[jnode] = 0;
                break;
            }

            iparent = this.parent[iparent];
        }
    }

    // update the display
    this.update();
}


//
// expand a Node in the Tree
//
function TreeExpand(inode) {
    // alert("in TreeExpand(inode="+inode+")");

    // validate inputs
    if (inode < 0 || inode >= this.opened.length) {
        alert("inode="+inode+" is out of range");
        return;
    }

    // expand inode
    this.opened[inode] = 1;

    // update the display
    this.update();
}


//
// change a property of a Node
//
function TreeProp(inode, iprop, onoff) {
    // alert("in TreeProp(inode="+inode+", iprop="+iprop+", onoff="+onoff+")");

    // validate inputs
    if (inode < 0 || inode >= this.opened.length) {
        alert("inode="+inode+" is out of range");
        return;
    } else if (onoff != "on" && onoff != "off") {
        alert("onoff="+onoff+" is not 'on' or 'off'");
        return;
    }

    if (this != myTree) {
        alert("this="+this+"   myTree="+myTree);
    }

    var thisNode = "";

    // set the property for inode
    if        (iprop == 1 && this.prop1[inode] == "Viz") {
        thisNode = this.document.getElementById("node"+inode+"col3");

        if (this.gprim[inode] != "") {
            if (onoff == "on") {
                wv.sceneGraph[this.gprim[inode]].attrs |=  wv.plotAttrs.ON;
            } else {
                wv.sceneGraph[this.gprim[inode]].attrs &= ~wv.plotAttrs.ON;
            }
        }
    } else if (iprop == 1) {
    } else if (iprop == 2 && this.prop2[inode] == "Grd") {
        thisNode = this.document.getElementById("node"+inode+"col4");

        if (this.gprim[inode] != "") {
            if (onoff == "on") {
                wv.sceneGraph[this.gprim[inode]].attrs |=  wv.plotAttrs.LINES;
                wv.sceneGraph[this.gprim[inode]].attrs |=  wv.plotAttrs.POINTS;
            } else {
                wv.sceneGraph[this.gprim[inode]].attrs &= ~wv.plotAttrs.LINES;
                wv.sceneGraph[this.gprim[inode]].attrs &= ~wv.plotAttrs.POINTS;
            }
        }
    } else if (iprop == 2) {
    } else if (iprop == 3 && this.prop3[inode] == "Trn") {
        thisNode = this.document.getElementById("node"+inode+"col5");

        if (this.gprim[inode] != "") {
            if (onoff == "on") {
                wv.sceneGraph[this.gprim[inode]].attrs |=  wv.plotAttrs.TRANSPARENT;
            } else {
                wv.sceneGraph[this.gprim[inode]].attrs &= ~wv.plotAttrs.TRANSPARENT;
            }
        }
    } else if (iprop == 3 && this.prop3[inode] == "Ori") {
        thisNode = this.document.getElementById("node"+inode+"col5");

        if (this.gprim[inode] != "") {
            if (onoff == "on") {
                wv.sceneGraph[this.gprim[inode]].attrs |=  wv.plotAttrs.ORIENTATION;
            } else {
                wv.sceneGraph[this.gprim[inode]].attrs &= ~wv.plotAttrs.ORIENTATION;
            }
        }
    } else if (iprop ==3) {
    } else {
        alert("iprop="+iprop+" is not 1, 2, or 3");
        return;
    }

    // update fakelinks in TreeWindow (needed when .attrs do not exist)
    if (thisNode != "") {
        if (onoff == "on") {
            thisNode.setAttribute("class", "fakelinkon");
            thisNode.title = "Toggle Ori off";
        } else {
            thisNode.setAttribute("class", "fakelinkoff");
            thisNode.title = "Toggle Ori on";
        }
    }

    // set property for inode's children
    for (var jnode = inode+1; jnode < this.parent.length; jnode++) {
        if (this.parent[jnode] == inode) {
            this.prop(jnode, iprop, onoff);
        }
    }

    wv.sceneUpd = 1;
}


//
// update the Tree (after build/expension/contraction/property-set)
//
function TreeUpdate() {
    // alert("in TreeUpdate()");

    var doc = this.document;

    // traverse the Nodes using depth-first search
    for (var inode = 1; inode < this.opened.length; inode++) {
        var element = doc.getElementById("node"+inode);

        // unhide the row
        element.style.display = "table-row";

        // hide the row if one of its parents has .opened=0
        var jnode = this.parent[inode];
        while (jnode != 0) {
            if (this.opened[jnode] == 0) {
                element.style.display = "none";
                break;
            }

            jnode = this.parent[jnode];
        }

        // if the current Node has children, set up appropriate event handler to expand/collapse
        if (this.child[inode] > 0) {
            if (this.opened[inode] == 0) {
                var myElem = doc.getElementById("node"+inode+"col1");
                var This   = this;

                myElem.className = "fakelinkexpand";
                myElem.firstChild.nodeValue = "+";
                myElem.title   = "Expand";
                myElem.onclick = function () {
                    var thisNode = this.id.substring(4);
                    thisNode     = thisNode.substring(0,thisNode.length-4);
                    This.expand(thisNode);
                };

            } else {
                var myElem = doc.getElementById("node"+inode+"col1");
                var This   = this;

                myElem.className = "fakelinkexpand";
                myElem.firstChild.nodeValue = "-";
                myElem.title   = "Collapse";
                myElem.onclick = function () {
                    var thisNode = this.id.substring(4);
                    thisNode     = thisNode.substring(0,thisNode.length-4);
                    This.contract(thisNode);
                };
            }
        }

        if (this.click[inode] !== null) {
            var myElem = doc.getElementById("node"+inode+"col2");
            myElem.onclick = this.click[inode];
        }

        // set the class of the properties
        if (this.nprop[inode] >= 1) {
            var myElem = doc.getElementById("node"+inode+"col3");
            myElem.onclick = this.cbck1[inode];

            if (this.prop1[inode] == "Viz") {
                if (this.gprim[inode] != "") {
                    if ((wv.sceneGraph[this.gprim[inode]].attrs & wv.plotAttrs.ON) == 0) {
                        myElem.setAttribute("class", "fakelinkoff");
                        myElem.title = "Toggle Viz on";
                    } else {
                        myElem.setAttribute("class", "fakelinkon");
                        myElem.title = "Toggle Viz off";
                    }
                }
            }
        }

        if (this.nprop[inode] >= 2) {
            var myElem = doc.getElementById("node"+inode+"col4");
            myElem.onclick = this.cbck2[inode];

            if (this.prop2[inode] == "Grd") {
                if (this.gprim[inode] != "") {
                    if ((wv.sceneGraph[this.gprim[inode]].attrs & wv.plotAttrs.LINES) == 0) {
                        myElem.setAttribute("class", "fakelinkoff");
                        myElem.title = "Toggle Grd on";
                    } else {
                        myElem.setAttribute("class", "fakelinkon");
                        myElem.title = "Toggle Grd off";
                    }
                }
            }
        }

        if (this.nprop[inode] >= 3) {
            var myElem = doc.getElementById("node"+inode+"col5");
            myElem.onclick = this.cbck3[inode];

            if (this.prop3[inode] == "Trn") {
                if (this.gprim[inode] != "") {
                    if ((wv.sceneGraph[this.gprim[inode]].attrs & wv.plotAttrs.TRANSPARENT) == 0) {
                        myElem.setAttribute("class", "fakelinkoff");
                        myElem.title = "Toggle Trn on";
                    } else {
                        myElem.setAttribute("class", "fakelinkon");
                        myElem.title = "Toggle Trn off";
                    }
                }
            } else if (this.prop3[inode] == "Ori") {
                if (this.gprim[inode] != "") {
                    if ((wv.sceneGraph[this.gprim[inode]].attrs & wv.plotAttrs.ORIENTATION) == 0) {
                        myElem.setAttribute("class", "fakelinkoff");
                        myElem.title = "Toggle Ori on";
                    } else {
                        myElem.setAttribute("class", "fakelinkon");
                        myElem.title = "Toggle Ori off";
                    }
                }
            }
        }
    }
}


//
// rebuild the Tree Window
//
function rebuildTreeWindow() {
    // alert("in rebuildTreeWindow()");

    // if there was a previous Tree, keep track of whether or not
    //    the Parameters, Branches, and Display was open
    var pmtr1Open = 0;
    var pmtr2Open = 0;

    if (myTree.opened.length > 3) {
        pmtr1Open = myTree.opened[1];
        pmtr2Open = myTree.opened[2];
    }
  
    // clear previous Nodes from the Tree
    myTree.clear();

    // put the group headers into the Tree
    myTree.addNode(0, "Case " + wv.caseNum + " Parms", "",   "", null);
    myTree.addNode(0, "Case " + wv.caseNum + " QoIs",  "",   "", null);
    myTree.addNode(0, "Display",                       "",   "", null,
                   "Viz", toggleViz, "Grd", toggleGrd);

    // put the Design Parameters into the Tree
  
    for (var ipmtr = 0; ipmtr < wv.params.length; ipmtr++) {
        var name  = "\u00a0\u00a0"+wv.params[ipmtr].name;
        var value =                wv.params[ipmtr].value;
        myTree.addNode(1, name, "", "", null, ""+value, "");
    }

    // open the Design Parameters (if they were open before the Tree was rebuilt)
    if (pmtr1Open == 1) {
        myTree.opened[1] = 1;
    }

    // put the Local Variables into the Tree

    for (var iqoi = 0; iqoi < wv.qois.length; iqoi++) {
        var name  = "\u00a0\u00a0"+wv.qois[iqoi].name;
        var value =                wv.qois[iqoi].value;
        myTree.addNode(2, name, "", "", null, ""+value, "");
    }

    // open the Local Variables (if they were open before the Tree was rebuilt)
    if (pmtr2Open == 1) {
        myTree.opened[2] = 1;
    }

    // put the Display attributes into the Tree
    for (var gprim in wv.sceneGraph) {

        // parse the name
        var matches = gprim.split(" ");
        var iface = -1;
        var iedge = -1;
        var ivert = -1;

        // processing when Crystal is explicitly named
        if (matches.length == 1) {
            var bodyName = matches[0];

        // processing when Crystal is not explicitly named: "Crystal m"
        } else if (matches.length == 2 && matches[0] == "Crystal") {
            var bodyName = matches[0] + " " + matches[1];

        // processing when Crystal is explicitly named: "Crystal m Core/Tube"
        } else if (matches.length == 3 && matches[0] == "Crystal") {
            var bodyName = matches[0] + " " + matches[1];
            if        (matches[2] == "Core") {
                iface = 1;
            } else if (matches[2] == "Tube") {
                iedge = 1;
            } else if (matches[2] == "Case") {
                ivert = 1;
            }

        // processing when Crystal is not explicitly named: "Crystal m Core/Tube n"
        } else if (matches.length == 4 && matches[0] == "Crystal") {
            var bodyName = matches[0] + " " + matches[1];
            if        (matches[2] == "Core") {
                iface = matches[3];
            } else if (matches[2] == "Tube") {
                iedge = matches[3];
            } else if (matches[2] == "Case") {
                ivert = matches[3];
            }
        }

        // determine if Crystal does not exists
        var kbody = -1;
        for (var jnode = 1; jnode < myTree.name.length; jnode++) {
            if (myTree.name[jnode] == "\u00a0\u00a0"+bodyName) {
                kbody = jnode;
            }
        }

        // if Crystal does not exist, create it and its Core/Tube lists
        //    subnodes now
        var kface, kedge, kvert;
        if (kbody < 0) {
            myTree.addNode(3, "\u00a0\u00a0"+bodyName, "", "", null,
                           "Viz", toggleViz, "Grd", toggleGrd);
            kbody = myTree.name.length - 1;

            myTree.addNode(kbody, "\u00a0\u00a0\u00a0\u00a0Cores", "", "", null,
                           "Viz", toggleViz,
                           "Grd", toggleGrd,
                           "Trn", toggleTrn);
            kface = myTree.name.length - 1;

            myTree.addNode(kbody, "\u00a0\u00a0\u00a0\u00a0Tubes", "", "", null,
                           "Viz", toggleViz,
                           "Grd", toggleGrd,
                           "Trn", toggleTrn);
            kedge = myTree.name.length - 1;
          
            myTree.addNode(kbody, "\u00a0\u00a0\u00a0\u00a0Cases", "", "", null,
                           "Viz", toggleViz,
                           "Grd", toggleGrd,
                           "Trn", toggleTrn);
            kvert = myTree.name.length - 1;

        // otherwise, get pointers to the core-group and tube-group Nodes
        } else {
            kface = myTree.child[kbody];
            kedge = kface + 1;
            kvert = kedge + 1;
        }

        // make the Tree Node
        if (iface >= 0) {
            myTree.addNode(kface, "\u00a0\u00a0\u00a0\u00a0\u00a0\u00a0Core "+iface, "",
                           gprim, null, "Viz", toggleViz, "Grd", toggleGrd,
                                        "Trn", toggleTrn);
        } else if (iedge >= 0) {
            myTree.addNode(kedge, "\u00a0\u00a0\u00a0\u00a0\u00a0\u00a0Tube "+iedge, "",
                           gprim, null, "Viz", toggleViz, "Grd", toggleGrd,
                                        "Trn", toggleTrn);
        } else if (ivert >= 0) {
            myTree.addNode(kvert, "\u00a0\u00a0\u00a0\u00a0\u00a0\u00a0Case "+ivert, "",
                           gprim, null, "Viz", toggleViz, "Grd", toggleGrd,
                           "Trn", toggleTrn);
        }
    }

    // open the Display (by default)
    myTree.opened[3] = 1;

    // mark that we have (re-)built the Tree
    wv.sgUpdate = 0;

    // convert the abstract Tree Nodes into an HTML table
    myTree.build();
}
