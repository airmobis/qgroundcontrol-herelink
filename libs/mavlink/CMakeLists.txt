cmake_minimum_required(VERSION 3.10)

project(Mavlink LANGUAGES C)

add_library(mavlink INTERFACE)
target_include_directories(mavlink INTERFACE include)
