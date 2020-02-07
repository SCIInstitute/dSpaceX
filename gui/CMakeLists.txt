cmake_minimum_required(VERSION 2.6)

PROJECT(HDViz)

FIND_PACKAGE(GLUT REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(FTGL REQUIRED)
FIND_PACKAGE(Threads)
#FIND_PACKAGE(GLE)
FIND_PACKAGE(PNG REQUIRED)


SET(CMAKE_BUILD_TYPE "Debug")

# option(BUILD_HDVIZ_IMAGE "Build hdviz for image data" OFF)
# option(BUILD_HDVIZPORCESSING "Build HDViz preprocessing tool" OFF)
 
# Use itk.  This sets up include/link directories for us.
if(BUILD_HDVIZ_IMAGE)
# Find common/itk
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})
INCLUDE_DIRECTORIES(${ITK_INCLUDE_DIR})
endif(BUILD_HDVIZ_IMAGE)

LINK_DIRECTORIES( ${GLE_LINK_DIR} ${ANN_LINK_DIR} )

INCLUDE_DIRECTORIES(${FTGL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIR_ft2build}  ${FREETYPE_INCLUDE_DIR_freetype2} ${GLUT_INCLUDE_DIR} ${GLE_INCLUDE_FILES} )
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${HDPROCESS_INCLUDE_DIR})

ADD_EXECUTABLE(HDViz HDViz.cpp DisplayTubes.cpp)
TARGET_LINK_LIBRARIES (HDViz 
    gle 
    hdprocess
    imageutils
    ${FTGL_LIBRARIES}  
    ${FREETYPE_LIBRARY}  
    ${OPENGL_LIBRARIES}  
    ${GLUT_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT} 
)
SET_TARGET_PROPERTIES(HDViz PROPERTIES COMPILE_FLAGS -DFONTNAME=${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf) 

#why doesn't simply adding imageutils to the target_link_libraries work???
SET_TARGET_PROPERTIES(HDViz PROPERTIES
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../lib/imageutils"
)


ADD_EXECUTABLE(HDViz2 HDViz2.cpp DisplayTubes.cpp DisplayGraph.cpp)
TARGET_LINK_LIBRARIES (HDViz2 
    gle 
    hdprocess
    blas
    lapack
    imageutils
    export
    ${FTGL_LIBRARIES}  
    ${FREETYPE_LIBRARY}  
    ${OPENGL_LIBRARIES}  
    ${GLUT_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT} 
    ${PNG_LIBRARIES}
)
SET_TARGET_PROPERTIES(HDViz2 PROPERTIES COMPILE_FLAGS -DFONTNAME=${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf) 

#why doesn't simply adding imageutils to the target_link_libraries work???
SET_TARGET_PROPERTIES(HDViz2 PROPERTIES
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../lib/imageutils"
)

if(BUILD_HDVIZ_IMAGE)

ADD_EXECUTABLE(HDViz_3D HDViz.cpp)
TARGET_LINK_LIBRARIES (HDViz_3D 
    gle 
    gfortran
    blas
    lapack   
    imageutils
    hdprocess
    ${FTGL_LIBRARIES}  
    ${FREETYPE_LIBRARY}  
    ${OPENGL_LIBRARIES}  
    ${GLUT_LIBRARIES}    
    ${ITK_LIBRARIES}
)
#why doesn't simply adding imageutils to the target_link_libraries work???
SET_TARGET_PROPERTIES(HDViz_3D PROPERTIES
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../lib/imageutils"
)

ADD_EXECUTABLE(HDViz_2D HDViz.cpp)
TARGET_LINK_LIBRARIES (HDViz_2D 
    gle 
    gfortran
    blas
    lapack
    imageutils
    hdprocess
    ${FTGL_LIBRARIES}  
    ${FREETYPE_LIBRARY}  
    ${OPENGL_LIBRARIES}  
    ${GLUT_LIBRARIES}
    ${ITK_LIBRARIES}
)
#why doesn't simply adding imageutils to the target_link_libraries work???
SET_TARGET_PROPERTIES(HDViz_2D PROPERTIES
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../lib/imageutils"
)

SET_TARGET_PROPERTIES( HDViz_2D PROPERTIES COMPILE_DEFINITIONS DIMENSION=2 COMPILE_FLAGS -DFONTNAME=${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf)
SET_TARGET_PROPERTIES( HDViz_3D PROPERTIES COMPILE_DEFINITIONS DIMENSION=3 COMPILE_FLAGS -DFONTNAME=${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf)
endif(BUILD_HDVIZ_IMAGE)
