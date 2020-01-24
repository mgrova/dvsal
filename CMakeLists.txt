##---------------------------------------------------------------------------------------------------------------------
##  DV-MICO
##---------------------------------------------------------------------------------------------------------------------
##  Copyright 2019 - Marco Montes Grova (a.k.a. marrcogrova) 
##---------------------------------------------------------------------------------------------------------------------
##  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
##  and associated documentation files (the "Software"), to deal in the Software without restriction, 
##  including without limitation the rights to use, copy, modify, merge, publish, distribute, 
##  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
##  furnished to do so, subject to the following conditions:
##
##  The above copyright notice and this permission notice shall be included in all copies or substantial 
##  portions of the Software.
##
##  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
##  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
##  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES 
##  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
##  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
##---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
PROJECT(dv_mico VERSION 0.1)


#########################################
######			General config	   ######
#########################################
add_definitions(-std=c++17 -pthread -lpthread)

#########################################
######			Library	   ######
#########################################
file(GLOB_RECURSE DV_MICO_HEADERS "./include/*.h" "./include/*.inl")
file(GLOB_RECURSE DV_MICO_SOURCES "./src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${DV_MICO_HEADERS} ${DV_MICO_SOURCES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC pthread)

#########################################
######			Dependencies       ######
#########################################

find_package(dv)
if (${dv_FOUND})
  target_include_directories(${PROJECT_NAME} PUBLIC ${dv_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${dv_LIBRARIES})
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_DV)    
endif(${dv_FOUND})

find_package(OpenCV REQUIRED)
if(${OpenCV_FOUND})
  target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} LINK_PRIVATE ${OpenCV_LIBS})
  target_compile_definitions(${PROJECT_NAME} PUBLIC "HAS_OPENCV")
endif()
