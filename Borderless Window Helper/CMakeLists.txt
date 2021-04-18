cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(Borderless-Window-Helper VERSION 2.0.0 LANGUAGES CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "${PROJECT_NAME}-${PROJECT_VERSION}"
    CACHE
    PATH
    "installation directory"
    FORCE)
endif()

find_program(CCACHE ccache)
if(CCACHE)
  message(STATUS "found ccache: ${CCACHE}")
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

add_executable(${PROJECT_NAME}
  WIN32
  inifile.cpp
  main.cpp
  util.cpp
)

if(MSVC)
  target_sources(${PROJECT_NAME} PRIVATE "Borderless Window Helper.rc")
else()
  target_sources(${PROJECT_NAME} PRIVATE "Borderless-Window-Helper.rc")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE -DUNICODE -D_UNICODE)

include(cmake/CPM.cmake)

option(NANA_CMAKE_ENABLE_CONF "enable config.hpp" ON)

CPMAddPackage(
  NAME nana
  GITHUB_REPOSITORY cnjinhao/nana
  GIT_TAG 95a609e75d3ad558acfa1f664eb684d963392246
)

target_link_libraries(${PROJECT_NAME} PRIVATE nana::nana)

if(NOT MSVC)
  target_link_options(${PROJECT_NAME} PRIVATE
    -municode
    -static
    -static-libgcc
    -static-libstdc++
  )
endif()

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    /MT # build static exe
    /MP # build in parallel
  )
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(NOT MSVC)
  include(cmake/cv2pdb.cmake)
  if(CV2PDB)
    file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/pdb")
    add_cv2pdb_target(${PROJECT_NAME} ${PROJECT_NAME}.exe ${PROJECT_NAME}.pdb)
    add_custom_target(pdb DEPENDS "${PROJECT_NAME}_pdb")

    install(
      DIRECTORY "${PROJECT_BINARY_DIR}/pdb/"
      DESTINATION .
    )
  endif()
endif()

set(CPACK_GENERATOR ZIP)
include(CPack)
