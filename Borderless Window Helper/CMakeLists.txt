cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(Borderless-Window-Helper VERSION 2.2.0 LANGUAGES CXX)

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
  GIT_TAG e93fe8f7a15823bce375a855686a19825ae599fb
)

target_link_libraries(${PROJECT_NAME} PRIVATE nana::nana)

if(NOT MSVC)
  target_link_options(${PROJECT_NAME}
    PRIVATE
    -municode
    -static
    -static-libgcc
    -static-libstdc++
  )
endif()

if(MSVC)
  target_compile_options(${PROJECT_NAME}
    PRIVATE
    /MT # build static exe
  )
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

set(CPACK_GENERATOR ZIP)
include(CPack)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
