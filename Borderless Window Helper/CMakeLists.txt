cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(Borderless-Window-Helper VERSION 2.0.0 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_NAME}-${PROJECT_VERSION}" CACHE PATH "installation directory" FORCE)
endif()

find_program(CCACHE ccache)
if(CCACHE)
  message(STATUS "found ccache: ${CCACHE}")
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

add_executable(${PROJECT_NAME}
  WIN32
  Borderless-Window-Helper.rc
  inifile.cpp
  main.cpp
  util.cpp
)

option(NANA_CMAKE_ENABLE_CONF "enable config.hpp" ON)
# add_subdirectory(nana)

target_compile_definitions(${PROJECT_NAME} PRIVATE -DUNICODE -D_UNICODE)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME nana
  GITHUB_REPOSITORY oltolm/nana
  GIT_TAG fit_content
)

target_link_libraries(${PROJECT_NAME} PRIVATE nana::nana)

target_link_options(${PROJECT_NAME} PRIVATE
  -municode
  -static
  -static-libgcc
  -static-libstdc++
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

include(cmake/cv2pdb.cmake)
set(suffix ".exe")
if(CV2PDB)
  add_cv2pdb_target(pdb ${PROJECT_NAME})
endif()
