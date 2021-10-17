cmake_minimum_required(VERSION 3.15)
project(
  alpmpp
  VERSION 0.1.0
  DESCRIPTION ""
  HOMEPAGE_URL ""
  LANGUAGES CXX
)

add_library(alpmpp)
target_sources(
  alpmpp
  PRIVATE src/Handle.cpp
          src/Package.cpp
          src/alpmpp_utils.hpp
  PUBLIC include/alpmpp/Handle.hpp
         include/alpmpp/Package.hpp
         include/alpmpp/Alpm_range.hpp
)
target_include_directories(alpmpp PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_compile_features(alpmpp PRIVATE cxx_std_20)
target_link_libraries(alpmpp PRIVATE alpm)
