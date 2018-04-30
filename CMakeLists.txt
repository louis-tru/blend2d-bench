cmake_minimum_required(VERSION 3.1)

# Bench Project
# -------------

project(b2d_bench CXX)
include(FindPkgConfig)

set(CMAKE_CXX_STANDARD 11)

set(B2D_BENCH_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "B2D-Bench directory")
set(B2D_DIR       "${B2D_BENCH_DIR}/../b2d"   CACHE PATH "Location of 'b2d'")

set(B2D_BUILD_STATIC TRUE)
include("${B2D_DIR}/CMakeLists.txt")

set(B2D_BENCH_DEFS "")
set(B2D_BENCH_LIBS ${B2D_LIBS})
set(B2D_BENCH_CFLAGS ${B2D_CFLAGS})
set(B2D_BENCH_INCLUDE_DIRS "")

set(B2D_BENCH_SRC
  src/app.cpp
  src/app.h
  src/images_data.h
  src/module_base.cpp
  src/module_base.h
  src/module_b2d.cpp
  src/module_b2d.h
  src/module_cairo.cpp
  src/module_cairo.h
  src/module_qt.cpp
  src/module_qt.h
  src/shapes_data.cpp
  src/shapes_data.h
)

# Bench Module - Cairo
# --------------------

pkg_check_modules(CAIRO cairo)
if(CAIRO_FOUND)
  message("-- [b2d-bench] adding support for Cairo")
  list(APPEND B2D_BENCH_LIBS ${CAIRO_LIBRARIES})
  list(APPEND B2D_BENCH_DEFS BENCH_ENABLE_CAIRO)
  list(APPEND B2D_BENCH_INCLUDE_DIRS ${CAIRO_INCLUDE_DIRS})
endif()

# Bench Module - Qt5
# ------------------

find_package(Qt5Gui)
if(Qt5Gui_FOUND)
  message("-- [b2d-bench] adding support for Qt5")
  list(APPEND B2D_BENCH_LIBS Qt5::Gui)
  list(APPEND B2D_BENCH_DEFS BENCH_ENABLE_QT)
endif()

# Bench Executable
# ----------------

add_executable(b2d_bench ${B2D_BENCH_SRC})
set_target_properties(b2d_bench PROPERTIES COMPILE_FLAGS "${B2D_BENCH_CFLAGS}")
set_target_properties(b2d_bench PROPERTIES COMPILE_DEFINITIONS "${B2D_BENCH_DEFS}")
set_target_properties(b2d_bench PROPERTIES INCLUDE_DIRECTORIES "${B2D_BENCH_INCLUDE_DIRS}")
target_link_libraries(b2d_bench ${B2D_BENCH_LIBS})
target_include_directories(b2d_bench BEFORE PRIVATE ${B2D_INCLUDE_DIR})
