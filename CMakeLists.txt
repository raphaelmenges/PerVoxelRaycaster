cmake_minimum_required(VERSION 2.8)

project(PerVoxelRaycaster)
set(APPNAME Project)

# Activate C++11 in GCC and CLANG
if(NOT MSVC)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
endif(NOT MSVC)

# Used to find resources within c++ code
set(RESOURCES_PATH "${CMAKE_SOURCE_DIR}/res" CACHE PATH "Project specific path. Set manually if it was not found.")
add_definitions(-DRESOURCES_PATH="${RESOURCES_PATH}")

# Include base directory
include_directories("${CMAKE_SOURCE_DIR}")

# Include src directory
include_directories("${CMAKE_SOURCE_DIR}/src")

# Include ext directory
include_directories("${CMAKE_SOURCE_DIR}/ext")

# Collect own code
file(GLOB_RECURSE SOURCES
	"${CMAKE_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_SOURCE_DIR}/src/*.c"
)

file(GLOB_RECURSE HEADERS
	"${CMAKE_SOURCE_DIR}/src/*.h"
)

# OpenGLLoader
file(GLOB OGL
	"${CMAKE_SOURCE_DIR}/ext/OGL/*.c")

# Add GLM
file(GLOB_RECURSE GLM
	"${CMAKE_SOURCE_DIR}/ext/GLM/glm/*.hpp")
include_directories("${CMAKE_SOURCE_DIR}/ext/GLM")

# Add picoPNG
file(GLOB_RECURSE PICO_PNG
	"${CMAKE_SOURCE_DIR}/ext/picoPNG/*")

# Collect all code
set(ALL_CODE
	${SOURCES}
	${HEADERS}
	${OGL}
	${GLM}
	${PICO_PNG})

# Filtering for Visual Studio
if(MSVC)

	# http://stackoverflow.com/questions/9701387/cmake-source-group-multiple-files
	foreach(f ${ALL_CODE})
		# Get the path of the file relative to ${CMAKE_CURRENT_SOURCE_DIR},
		# then alter it (not compulsory)
		file(RELATIVE_PATH SRCGR "${CMAKE_CURRENT_SOURCE_DIR}" ${f})
		set(SRCGR "${LIBNAME}/${SRCGR}")

		# Extract the folder, ie remove the filename part
		string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

		# Source_group expects \\ (double antislash), not / (slash)
		string(REPLACE / \\ SRCGR ${SRCGR})
		source_group("${SRCGR}" FILES ${f})
	endforeach()

endif(MSVC)

# Find libraries
if(WIN32) # Windows

	# GLFW 3
	set(GLFW3_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ext/GLFW")
  include_directories("${GLFW3_DIRECTORY}/include")
	
	IF(MSVC10)
		set(GLFW3_LIBRARIES_DIRECTORY "${GLFW3_DIRECTORY}/lib-vc2012")
	ELSEIF(MSVC11)
		set(GLFW3_LIBRARIES_DIRECTORY "${GLFW3_DIRECTORY}/lib-vc2013")
	ELSE()
		set(GLFW3_LIBRARIES_DIRECTORY "${GLFW3_DIRECTORY}/lib-vc2015")
	ENDIF()

	find_library(GLFW3_STATIC_LIBRARIES
		NAMES glfw glfw3
		HINTS "${GLFW3_LIBRARIES_DIRECTORY}"
		NO_DEFAULT_PATH)

else() # Linux, GLFW3 has to be installed via package manager

	# GLFW 3
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLFW3 REQUIRED glfw3)
	include_directories(${GLFW3_INCLUDE_DIR})
	
endif()

# OpenGL (found this way on all systems)
# ${OPENGL_LIBRARIES}
# ${OPENGL_INCLUDE_DIR}
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# Creation of executeable
add_executable(${APPNAME} ${ALL_CODE})

# Linking
target_link_libraries(${APPNAME} ${OPENGL_LIBRARIES} ${GLFW3_STATIC_LIBRARIES})