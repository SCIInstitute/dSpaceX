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
**BUILDING ON OS MOJAVE OR LATER**

Apple made changes that removed the /usr/include file and now some programs won't compile, including dSpaceX. 
The following steps provide instructions for adding the /usr/include file back. This only needs to be done once.
1. Check to see if you have Xcode command line tools installed
    1. Start terminal
    2. run command ``` xcode-select -p ```
2. If you do not have Xcode command line tools installed follow these [directions](https://www.embarcadero.com/starthere/xe5/mobdevsetup/ios/en/installing_the_commandline_tools.html) 
3. Run the following command in the terminal 
``` 
sudo installer -pkg /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg -target /
```

**Additional Notes**
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
