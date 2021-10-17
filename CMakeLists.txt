cmake_minimum_required(VERSION 3.15)
project(
  aur_rebuild
  VERSION 0.1.0
  DESCRIPTION ""
  HOMEPAGE_URL ""
  LANGUAGES CXX
)

add_subdirectory(alpmpp)

add_executable(rebuildaur)
target_sources(rebuildaur PRIVATE rebuildaur.cpp)
target_compile_features(rebuildaur PRIVATE cxx_std_20)
target_link_libraries(rebuildaur PRIVATE alpmpp)
