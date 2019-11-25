# dSpaceX - Design Space Exploration Tool

This project uses the CMake build system. The main application is a server
and client application. The Server is written in C++ and the client is
a web application written in Javascript. It relies on HTML5 which is 
supported by all modern browsers.

## Install dependencies (OSX/Linux)

We use anaconda to create a sandbox environment, which facilitates multiple applications with different dependencies. It is not a virtual environment and therefore incurs no performance penalty. Install Anaconda and the dSpaceX dependencies using:  
```bash
source ./conda_installs.sh
```
Accept the cryptography license terms and default installation path.  

## Building Native & Server Code.
1. Create a build directory
```bash
<.../dSpaceX>$ mkdir build
```

2. Run cmake or ccmake to configure.
```bash
<.../dSpaceX>$ cd build
<.../dSpaceX/build>$ ccmake -G<generator> ../
```
Generator can be omitted for a simple Makefile, or set to one of those [listed on the CMake page](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#command-line-build-tool-generators).

3. Run make to build the targets.
```bash
<../dSpaceX/build>$ make -j8
```

**Additional Notes**
The default CMake configuration will only build the HDProcess library.
It can also optionally build the following binaries if desired.
These may require installation of additional dependencies, listed below.
- HDViz           - A GUI for Visualizating Datasets.
- HDVizProcessing - A Commnad Line tool for feeding a dataset through the analysis tool.
- HDVizImage      - A similar tool for working with Image data.

The HDProcess library has the following dependencies:
- BLAS
- LAPACK
- gfortran

The HDViz GUI has the following additional dependencies:
- GLUT
- OpenGL
- FTGL
- Freetype
- Threads

The HDVizImage binary has the following additional dependencies:
- ITK

The beSpace server backend has the following dependencies:
- libJpeg
- libPng

# Building the Client Code
**See [README.md in the client directory](./client/README.md)**



# Configuring Datasets

The *dSpaceX* server reads datasets consisting of _images (samples)_, _design parameters (parameters)_, and _quantities of interest (QoIs)_. These must be organized into a single directory with a `config.yaml` that specifies the name of the dataset, its number of samples, and the names, locations, and formats of its images, parameters, QoIs, distance matrices, embeddings (e.g., a tsne layout), and probabilistic models. The currently supported formats are csv, json, and yaml (comma-separated values, JavaScript object notation, and "YAML ain't markup language"), and png images. Here is an example of a the yaml configuration:

```yaml
name: CantileverBeam

samples:
  count: 1000

parameters:
  format: csv
  file: CantileverBeam_design_parameters.csv

qois:
  format: csv
  file: CantileverBeam_QoIs.csv

thumbnails:
  format: png
  files: images/?.png
  offset: 1                     # base-1 image names (0th name is 1; if offset by 1000, names would start at 1000)
  padZeroes: false              # padded image names (min chars needed must represent offset + num_files)

distances:
  format: csv
  file: CantileverBeam_distance_matrix.csv
  metric: euclidean

embeddings:
  - name: tsne
    format: csv
    file: CantileverBeam_tsne_layout.csv

models:
  - fieldname: maxStress
    type: shapeodds                                            # could be shapeodds or sharedgp
    root: shapeodds_models_maxStress                           # directory of models for this field
    persistences: persistence-?                                # persistence files
    crystals: crystal-?                                        # in each persistence dir are its crystals
    padZeroes: false                                           # for both persistence and crystal dirs/files
    #format: csv   (just use extension of most files to determine format) # lots of csv files in each crystal: Z, crystalIds, W, wo
    partitions: CantileverBeam_CrystalPartitions_maxStress.csv # has 20 lines of varying length and 20 persistence levels
    embeddings: shapeodds_global_embedding.csv                 # a tsne embedding? Global for each p-lvl, and local for each crystal
```
