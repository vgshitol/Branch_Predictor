cmake_minimum_required(VERSION 3.12)
project(Branch_Predictor)

set(CMAKE_CXX_STANDARD 14)

add_executable(Branch_Predictor sim_bp.cc Branch_Predictor.h)