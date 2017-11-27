This project uses the CMake build system.

# Steps for build.
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
