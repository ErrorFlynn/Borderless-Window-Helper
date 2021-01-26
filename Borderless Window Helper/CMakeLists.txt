cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(Borderless-Window-Helper VERSION 2.0.0 LANGUAGES CXX)

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
  GITHUB_REPOSITORY cnjinhao/nana
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
