#~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~#
## Minimum CMake Required to Build and the policy
#~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~#
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)


###############################################################
## Test independent files and libraries
###############################################################
PROJECT(GLE)

#- - - - - - - - - - - - OpenGL - - - - - - - - - - - - - - - #
FIND_PACKAGE(OpenGL)


###############################################################
## Include directories
###############################################################
INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR}  ${GLUT_INCLUDE_DIR})
			
###############################################################
## Definitions
###############################################################
ADD_DEFINITIONS( ${GL_DEFS})

###############################################################
## Source files
###############################################################
SET(GLE_INCLUDE_FILES copy.h      
                      extrude.h   
		      gle.h       
		      intersect.h 
		      port.h      
		      rot.h       
		      segment.h   
		      tube_gc.h   
		      vvector.h
)

SET(GLE_SOURCE_FILES ex_alpha.c     
                     ex_cut_round.c 
		     extrude.c      
		     qmesh.c        
		     rotate.c       
		     segment.c     
		     urotate.c
		     ex_angle.c     
		     ex_raw.c      
		     intersect.c   
		     rot_prince.c   
		     round_cap.c    
		     texgen.c       
		     view.c
)

###############################################################
## Libraries
###############################################################
LINK_DIRECTORIES( ${OPENGL_LINK_DIRS} )

ADD_LIBRARY( "gle"  ${GLE_INCLUDE_FILES} ${GLE_SOURCE_FILES})

