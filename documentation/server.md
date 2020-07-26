# The dSpaceX Server

- [Building](#building-the-server)  
- [Running](#running-the-server)  


## Building the server
0. Activate the dspacex conda environment:
``` bash
<.../dSpaceX>$ conda activate dspacex
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

## Running the server
``` bash
<../dSpaceX/build>$ conda activate dspacex
<../dSpaceX/build>$ ./bin/dSpaceX
```
Options include `--port` and `--datapath` to specify the port on which to listen for client connections and the path to available datasets.
Use `--help` to list all options.  
See [Configuration](configuration.md) for more about preparing datasets to be hosted by the dSpaceX server.

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
