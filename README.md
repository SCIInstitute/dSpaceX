# dSpaceX - Design Space Exploration Tool

This project uses the CMake build system. The main application is a server
and client application. The Server is written in C++ and the client is
a web application written in Javascript. It relies on HTML5 which is 
supported by all modern browsers.

## Install dependencies

We use anaconda to create a sandbox environment, which facilitates multiple applications with different dependencies. It is not a virtual environment and therefore incurs no performance penalty. Install Anaconda and the dSpaceX dependencies using:  
```bash
source ./conda_installs.sh
```
Accept the cryptography license terms and default installation path.  

## Build server
0. Activate the dspacex conda environment:
``` bash
conda activate dspacex
```

1. Create a build directory.
```bash
<.../dSpaceX>$ mkdir build
```

2. Run cmake to configure.
```bash
<.../dSpaceX>$ cd build
<.../dSpaceX/build>$ cmake -G<generator> -DCMAKE_PREFIX_PATH=${CONDA_PREFIX} ../
```
Generator can be omitted for a simple Makefile, or set to one of those [listed on the CMake page](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#command-line-build-tool-generators).

3. If a generator was specified load and build the DSPACEX project file. Otherwise, simply run make to build the targets.
```bash
<../dSpaceX/build>$ make -j8
```

4. Run the server.
``` bash
<../dSpaceX/build>$ ./bin/dSpaceX
```
Options include `--port` and `--datapath` to specify the port on which to listen for client connections and the path to available datasets.
Use `--help` to list all options.


**Additional Notes**

**Building Other Artifacts**
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

**Building in CLion**
If you build the server in the CLion IDE you see the following error
```cmake
CMake Error at /Applications/CLion.app/Contents/bin/cmake/mac/share/cmake-3.16/Modules/FindPackageHandleStandardArgs.cmake:146 (message):
  Could NOT find PNG (missing: PNG_LIBRARY PNG_PNG_INCLUDE_DIR)
```
You will need to add -DCMAKE_PREFIX_PATH=${CONDA_PREFIX} to the CMake options in CLion.
1. Select the settings(gear) icon in the CMake window
2. Fromt the dropdown select CMake Settings
3. In the CMake Options: field paste -DCMAKE_PREFIX_PATH=${CONDA_PREFIX} 

CONDA_PREFIX is the path to the conda environment.
Adding it to the CMake prefix path guarantees CMake can find all packages installed in that environment.


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
  channels: 3                    # num channels in each shape (e.g., 1-greyscale, 3-RGB, 4-RGBA)

distances:
  format: csv
  file: CantileverBeam_distance_matrix.csv
  metric: euclidean

embeddings:
  - name: tsne
    format: csv
    file: CantileverBeam_tsne_layout.csv
  - name: ShapeOdds
    format: csv
    file: shapeodds_global_embedding.csv
  - name: Shared GP
    format: csv
    file: shared_gp_global_embedding.csv

models:
  - fieldname: Max Stress
    type: shapeodds                                            # shapeodds, pca, sharedgp, etc
    root: shapeodds_models_maxStress                           # directory of models for this field
    persistences: persistence-?                                # persistence files
    crystals: crystal-?                                        # in each persistence dir are its crystals
    padZeroes: false                                           # for both persistence and crystal dirs/files
    partitions: CantileverBeam_CrystalPartitions_maxStress.csv # has 20 lines of varying length and 20 persistence levels
    rowmajor: false                                            # the shape produced by this model is a row-major image
    ms:                                                        # Morse-Smale parameters used to compute partitions
      knn: 15                                                  # k-nearest neighbors
      sigma: 0.25                                              # 
      smooth: 15.0                                             # 
      depth: 20                                                # num persistence levels; -1 means compute them all
      noise: true                                              # add mild noise to the field to ensure inequality
      curvepoints: 50                                          # vis only? Not sure if this matters for crystal partitions 
      normalize: false                                         # vis only? Not sure if this matters for crystal partitions
    interpolations:                                            # precomputed interps
       - i1:
         params:                                               # model interpolation parameters used
           sigma: 0.15                                         # Gaussian width
           num_interps: 50                                     # precomputed interps per crystal
       - i2:
         params:
           sigma: 0.01
           num_interps: 500

  - fieldname: Angle
    type: pca
    root: pca_models/pca_model_param_Angle
    persistences: persistence-?
    crystals: crystal-?
    padZeroes: false
    partitions: crystal_partitions/cantilever_crystal_partitions_Angle.csv
    rowmajor: true
    ms:
      knn: 15
      sigma: 0.25
      smooth: 15.0
      depth: -1
      noise: true
      curvepoints: 50
      normalize: false
    interpolations:
       - i1:
         params:
           sigma: 0.15
           num_interps: 50
       - i2:
         params:
           sigma: 0.01
           num_interps: 500

  - fieldname: Angle
    type: custom                                        # a new model type (no dynamic interpolation will be available, so precomputed should be provided)
    root: custom_models/custom_model_param_Angle
    persistences: persistence-?
    crystals: crystal-?
    padZeroes: false
    partitions: crystal_partitions/cantilever_crystal_partitions_Angle.csv
    ms:
      knn: 15
      sigma: 0.25
      smooth: 15.0
      depth: -1
      noise: true
      curvepoints: 50
      normalize: false
    interpolations:
       - i1:
         params:
           sigma: 0.15
           num_interps: 50
       - i2:
         params:
           sigma: 0.01
           num_interps: 500
```
