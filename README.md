# dSpaceX - Design Space Exploration Tool

This project uses the CMake build system. The main application is a server
and client application. The Server is written in C++ and the client is
a web application written in Javascript. It relies on HTML5 which is 
supported by all modern browsers.

# Building Native & Server Code.
1. Create a build directory
```
    <../dSpaceX>$  mkdir build
```

2. Run cmake or ccmake to configure.
```
    <../dSpaceX>$  cd build
    <../build>$ ccmake ../
```

3. Run make to build the targets.
```
    <../build>$ make -j8
```


The default CMake configuration will only build the HDProcess library.
It can also optionally build the following binaries if desired:
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
See /client/README.md
