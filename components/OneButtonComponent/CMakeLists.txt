# OneButton Library
# https://github.com/mathertel/OneButton
# BSD-3-clause License

cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "../OneButton/src/OneButton.cpp" "../OneButton/src/OneButton.h"
                       INCLUDE_DIRS "../OneButton/src"
                       REQUIRES arduino)

project(OneButton)
