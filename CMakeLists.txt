cmake_minimum_required(VERSION 3.16)
project(web_server C)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})
include_directories(
  ${CMAKE_SOURCE_DIR}/include 
)
