# High dimensional function visualization

![hdviz.png](https://bitbucket.org/repo/R6ok4n/images/4162517882-hdviz.png)

Software for the paper.

Samuel Gerber, Peer-Timo Bremer, Valerio Pascucci, Ross Whitaker
"Visual Exploration of High Dimensional Scalar Functions"
IEEE Transactions on Visualization and Computer Graphics to appear, Proceedings of VIS 2010

An R-package that is capable of a similar visualizations is available the MorseSmaleComplex project.

The software consist of two binaries

1. HDVizProcessing
2. HDViz

The first is for processing a given data set for visualization and the seconf
for the actual visualization.


## Usage of HDVizProcessing:


HDVizProcessing -h will type a help message

The input to HDVizProcessing is a matrix X which represents the samples of the
domain and a corresponding vector which represnets the function values f(X) of
each sample. The other parameters are requireed for Morse-Smale and regression
curve computations.

HDVizProcessing will write a number of files that encode the visualization for
HDViz. 



## Usage of HDViz

HDViz is for the actual visualization. It needs to be started in the same folder
as HDVizPorcessing was run and it will read the files in these folders for
visualization.

HDViz tries to find a defualt font (DejaVuSans.ttf) if it can't find it you can
supply your own font with -f <path-to-font> (for best results use
    DejaVuSans.ttf).

HDViz tries to read a file names.txt. with the names of the variables in X, one
per line. If not available it will use x_1, ..., x_n as labels.


## An exmaple
The directopry examples contains a number of examples.

* cd examples/gaussian2d
* HDVizProcessing -x Geom.data.hdr -f Function.data.hdr -k 25 -s 0.25 -p 10 -n 50
      * This generates the fiels for the gaussain2d for 10 persistence level with 50 samples per curve in each crystal and uses 25 nearest-neighbors for the MS-computation.
* HDViz -c
      * Run HDViz and display curves for each variable



## File format

See FLinalg project.