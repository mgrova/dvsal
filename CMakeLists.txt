##---------------------------------------------------------------------------------------------------------------------
##  DVSAL
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
PROJECT(dvsal VERSION 0.1)

option(BUILD_EXAMPLES		"Compile examples"	ON)

#########################################
######			General config	   ######
#########################################
add_definitions(-std=c++17 -pthread -lpthread)

#########################################
######			Library	   ######
#########################################
file(GLOB_RECURSE DVSAL_HEADERS "./include/*.h" "./include/*.inl")
file(GLOB_RECURSE DVSAL_SOURCES "./src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${DVSAL_HEADERS} ${DVSAL_SOURCES})
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

find_package(libcaer REQUIRED)
if (${libcaer_FOUND})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC libcaer::caer)
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_LIBCAER)
endif(${libcaer_FOUND})

find_package(dv REQUIRED)
if (${dv_FOUND})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC dv::dvsdk)
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_DV)
endif(${dv_FOUND})

find_package(OpenCV REQUIRED)
if(${OpenCV_FOUND})
  target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${OpenCV_LIBS})
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_OPENCV)
endif()

find_package(Boost REQUIRED COMPONENTS system filesystem)
if (${Boost_FOUND})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Boost::boost Boost::system Boost::filesystem)
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_BOOST)
endif(${Boost_FOUND})

find_package(Eigen3 REQUIRED)
if(${Eigen3_FOUND})
  target_include_directories(${PROJECT_NAME} PUBLIC "/usr/include/eigen3")
endif()

find_package(flow REQUIRED)
if (${flow_FOUND})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC flow::flow)
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_FLOW)
endif(${flow_FOUND})

#########################################
######			EXAMPLES        		   ######
#########################################
if(${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif(${BUILD_EXAMPLES})


#########################################
######			INSTALL 		   ######
#########################################
include(CMakePackageConfigHelpers)


## INSTALLATION C++ Library
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif() 

install(TARGETS dvsal
        EXPORT dvsalTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}
        )

write_basic_package_version_file(
  dvsalConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
    )


# This installs the include folder
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.h")
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.inl")

export(TARGETS dvsal NAMESPACE dvsal:: FILE dvsal.cmake)

# This generates dvsalTargets.cmake
install(EXPORT dvsalTargets
    FILE dvsalTargets.cmake
    NAMESPACE dvsal::
    DESTINATION lib/cmake/dvsal
    )

configure_file(dvsalConfig.cmake.in dvsalConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/dvsalConfig.cmake"
        DESTINATION lib/cmake/dvsal
        )

# uninstall target 
if(NOT TARGET uninstall) 
  configure_file( 
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in" 
      "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" 
      IMMEDIATE @ONLY) 
 
  add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake) 
endif() 