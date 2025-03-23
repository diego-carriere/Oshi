cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME Oshi)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")

# prevent installing to system directories. 
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Ajouter les fichiers sources
file(GLOB SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})



# Trouver et lier OpenGL (et glew)
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# pour que CMake trouve les fichiers .h et ajoute les librairies nécessaires
include_directories(${PROJECT_SOURCE_DIR}/include)

#Trouver et lier glew
find_library(GLEW name glew32s PATHS ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLEW})
add_definitions(-DGLEW_STATIC)  # Ou -DGLEW_DYNAMQUE selon le type utilisé

#Trouver et lier SDL
add_subdirectory(lib/SDL EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

