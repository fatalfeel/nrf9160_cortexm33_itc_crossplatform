# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.overlay")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_include_directories(app PRIVATE
src)

target_sources(app PRIVATE
src/main.cpp
src/memtracer.cpp
src/eventpack.cpp
src/eventmanager.cpp
src/watchdog.cpp
src/heartbeat.cpp
src/test_uart00.cpp
src/test_uart01.cpp
src/test_uart02.cpp)
