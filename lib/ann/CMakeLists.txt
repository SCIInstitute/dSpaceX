CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)


PROJECT(ann)
INCLUDE_DIRECTORIES( . )
SET(ANN_INCLUDE_FILES 
   ANN/ANN.h
   ANN/ANNperf.h
   ANN/ANNx.h
#  bd_tree.h  
#  kd_fix_rad_search.h  
#  kd_pr_search.h  
#  kd_search.h  
#  kd_split.h
#  kd_tree.h  
#  kd_util.h  
#  pr_queue.h  
#  pr_queue_k.h   
)

SET(ANN_SOURCE_FILES 
    ANN.cpp                
    bd_pr_search.cpp  
    bd_tree.cpp  
    #kd_dump.cpp
    kd_pr_search.cpp  
    kd_split.cpp  
    kd_util.cpp
    bd_fix_rad_search.cpp  
    bd_search.cpp     
    brute.cpp    
    kd_fix_rad_search.cpp  
    kd_search.cpp     
    kd_tree.cpp   
    perf.cpp
)

ADD_LIBRARY( "ANN"  ${ANN_INCLUDE_FILES} ${ANN_SOURCE_FILES})

