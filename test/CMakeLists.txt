PROJECT(gpx_test)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_CXX_STANDARD 11)

ADD_EXECUTABLE(gpx_test main.cpp ../lib/gpx/src/gpx.cpp ../lib/gpx/src/gpx.h)