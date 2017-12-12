			dSpaceX Prototype User Interface


	Contained within is prototype code used to layout the browser-based
user interface envisioned for dSpaceX. It is NOT fully functional in any
operation, but there are methods to show what some functionality may appear
like. In particular, there is currently no rendering in the "scatter plot"
frame, and the left-hand frame currently displays the "active case" in a
poor format (it will possibly also provide control of the rendering state
in the future).


Building:

	It can build under Windows, but what is provided will only work for
LINUX and MACs (64 bit). There are very few dependencies: (1) libJpeg dev
and (2) libPng development. On the MAC these have been provided (and tested)
using Home Brew.

	A single Environment Variable is required to be set "DSX_ARCH",
which is the build architecture and can be either: LINUX64 or DARWIN64.

	Once both the dependencies and the environment variable is set,
simply type: "make" at this level.


Execution:

	There are 2 components of this UI. A "server", which is built from
the compiled code (made above), and the "client", which runs in a web browser.
Note that any Web Browser that fully supports HTML5 may be used (this excludes
the Microsoft's IE and Edge).
 
	The server requires that a dynamically loaded back-end be available that
deals with the specifics of the design suite data. There is one that gets built
with the server (the source is in "server/sampleBE.c"). This is specifically 
written for the BEES dataset found in "data/BEES" where you will also find
links to the server and the back-end. To execute the server "cd" into the BEES
directory and type "dSpaceX".

	The client can be started (after the server) by loading the file
"client/dSpaceX.html" into the browser URL bar. You will be asked for a hostname
and port -- just click OK. You should see stuff and be up and running.

	There is really not much you can do, but you can:
1) Move the persistence slider (and see 1 to 4 leaves in the 3D frame)
2) Move around in 3D
    - mouse down         - translate
    - mouse down/control - x/y rotate
    - mouse down/alt     - z rotate
3) Hit T to get a thumbnail
4) Hit N to change the QoI
5) Hit C to change the case seen in left-hand frame

