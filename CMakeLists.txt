##---------------------------------------------------------------------------------------------------------------------
##  DVSAL
##---------------------------------------------------------------------------------------------------------------------
##  Copyright 2020 - Marco Montes Grova (a.k.a. mgrova) marrcogrova@gmail.com 
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

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
PROJECT(dvsal VERSION 0.1)

option(BUILD_EXAMPLES		"Compile examples"	ON)

##################################################
######    Library.                        ########
##################################################
file(GLOB_RECURSE DVSAL_HEADERS "./include/*.h" "./include/*.inl")
file(GLOB_RECURSE DVSAL_SOURCES "./src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${DVSAL_HEADERS} ${DVSAL_SOURCES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC pthread)

##################################################
######    Loading 3rd party libraries.    ########
##################################################

find_package(Qt5 COMPONENTS
             Core
             Widgets
             Gui
             OpenGL
             PrintSupport)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  	Qt5::Core 
							Qt5::Widgets 
							Qt5::Gui
							Qt5::OpenGL 
							Qt5::Printsupport)

find_package(fmt REQUIRED)

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

find_package(Boost REQUIRED) 
if (${Boost_FOUND})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Boost::boost) 
  target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_BOOST)
endif(${Boost_FOUND})

find_package(Eigen3 REQUIRED)
if(${Eigen3_FOUND})
  target_include_directories(${PROJECT_NAME} PUBLIC "/usr/include/eigen3")
endif()

find_package(PCL QUIET REQUIRED)
if(${PCL_FOUND})
  target_include_directories(${PROJECT_NAME} PUBLIC ${PCL_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} LINK_PRIVATE ${PCL_LIBRARIES})
  target_compile_definitions(${PROJECT_NAME} PUBLIC "ENABLE_PCL" "DISABLE_PCAP" "DISABLE_PNG" "DISABLE_LIBUSB_1_0")
endif()

# Compression support
find_package(PkgConfig REQUIRED)
pkg_check_modules(LZ4 REQUIRED liblz4)
if(${LZ4_FOUND})
  target_include_directories(${PROJECT_NAME} PUBLIC ${LZ4_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${LZ4_LIBRARIES})
  message(STATUS "liblz4 Found and Enabled")
endif()

pkg_check_modules(zstd REQUIRED libzstd)
if(${zstd_FOUND})
  target_include_directories(${PROJECT_NAME} PUBLIC ${zstd_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${zstd_LIBRARIES})
  message(STATUS "libzstd Found and Enabled")
endif()

##################################################
######    Examples.                       ########
##################################################
if(${BUILD_EXAMPLES})
  add_subdirectory(examples)
endif(${BUILD_EXAMPLES})


##################################################
######    Installation.                   ########
##################################################
include(CMakePackageConfigHelpers)

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif() 

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
	COMPONENT ${PROJECT_NAME}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
	INCLUDES DESTINATION include
)

install(EXPORT ${PROJECT_NAME}-targets
	FILE "${PROJECT_NAME}-targets.cmake"
	NAMESPACE ${PROJECT_NAME}::
	DESTINATION lib/cmake/${PROJECT_NAME}
	COMPONENT ${PROJECT_NAME}
)

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/templates/${PROJECT_NAME}-config.cmake.in"
	"${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
	@ONLY
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION ${version}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION lib/cmake/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)

# DESTINATION will be automatically prefixed by ${CMAKE_INSTALL_PREFIX}
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
    COMPONENT ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# uninstall target 
if(NOT TARGET uninstall) 
    configure_file( 
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/cmake_uninstall.cmake.in" 
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" 
        @ONLY) 
 
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake) 
endif() 
