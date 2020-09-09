cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0063 NEW) # Honor visibility properties.

# Bench Project
# -------------

project(bl_bench CXX)
include(FindPkgConfig)

set(CMAKE_CXX_STANDARD 11)

set(BLBENCH_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "BlBench directory")
set(BLEND2D_DIR "${BLBENCH_DIR}/../blend2d" CACHE PATH "Location of 'blend2d'")

set(BLBENCH_INCLUDE_DIRS "")
set(BLBENCH_DEFINITIONS "")
set(BLBENCH_LIBRARIES "")

set(BLBENCH_SRC
  src/app.cpp
  src/app.h
  src/images_data.h
  src/module.cpp
  src/module.h
  src/module_agg.cpp
  src/module_agg.h
  src/module_blend2d.cpp
  src/module_blend2d.h
  src/module_cairo.cpp
  src/module_cairo.h
  src/module_qt.cpp
  src/module_qt.h
  src/shapes_data.cpp
  src/shapes_data.h
)

# Bench Module - Blend2D
# ----------------------

set(BLEND2D_STATIC TRUE)
include("${BLEND2D_DIR}/CMakeLists.txt")
list(APPEND BLBENCH_LIBRARIES blend2d::blend2d)

# Bench Module - AGG
# -------------------

set(AGG_SRC
  3rdparty/agg/include/agg_alpha_mask_u8.h
  3rdparty/agg/include/agg_arc.h
  3rdparty/agg/include/agg_array.h
  3rdparty/agg/include/agg_arrowhead.h
  3rdparty/agg/include/agg_basics.h
  3rdparty/agg/include/agg_bezier_arc.h
  3rdparty/agg/include/agg_bitset_iterator.h
  3rdparty/agg/include/agg_blur.h
  3rdparty/agg/include/agg_bounding_rect.h
  3rdparty/agg/include/agg_bspline.h
  3rdparty/agg/include/agg_clip_liang_barsky.h
  3rdparty/agg/include/agg_color_gray.h
  3rdparty/agg/include/agg_color_rgba.h
  3rdparty/agg/include/agg_config.h
  3rdparty/agg/include/agg_conv_adaptor_vcgen.h
  3rdparty/agg/include/agg_conv_adaptor_vpgen.h
  3rdparty/agg/include/agg_conv_bspline.h
  3rdparty/agg/include/agg_conv_clip_polygon.h
  3rdparty/agg/include/agg_conv_clip_polyline.h
  3rdparty/agg/include/agg_conv_close_polygon.h
  3rdparty/agg/include/agg_conv_concat.h
  3rdparty/agg/include/agg_conv_contour.h
  3rdparty/agg/include/agg_conv_curve.h
  3rdparty/agg/include/agg_conv_dash.h
  3rdparty/agg/include/agg_conv_gpc.h
  3rdparty/agg/include/agg_conv_marker_adaptor.h
  3rdparty/agg/include/agg_conv_marker.h
  3rdparty/agg/include/agg_conv_segmentator.h
  3rdparty/agg/include/agg_conv_shorten_path.h
  3rdparty/agg/include/agg_conv_smooth_poly1.h
  3rdparty/agg/include/agg_conv_stroke.h
  3rdparty/agg/include/agg_conv_transform.h
  3rdparty/agg/include/agg_conv_unclose_polygon.h
  3rdparty/agg/include/agg_curves.h
  3rdparty/agg/include/agg_dda_line.h
  3rdparty/agg/include/agg_ellipse_bresenham.h
  3rdparty/agg/include/agg_ellipse.h
  3rdparty/agg/include/agg_embedded_raster_fonts.h
  3rdparty/agg/include/agg_font_cache_manager2.h
  3rdparty/agg/include/agg_font_cache_manager.h
  3rdparty/agg/include/agg_gamma_functions.h
  3rdparty/agg/include/agg_gamma_lut.h
  3rdparty/agg/include/agg_glyph_raster_bin.h
  3rdparty/agg/include/agg_gradient_lut.h
  3rdparty/agg/include/agg_gsv_text.h
  3rdparty/agg/include/agg_image_accessors.h
  3rdparty/agg/include/agg_image_filters.h
  3rdparty/agg/include/agg_line_aa_basics.h
  3rdparty/agg/include/agg_math.h
  3rdparty/agg/include/agg_math_stroke.h
  3rdparty/agg/include/agg_path_length.h
  3rdparty/agg/include/agg_path_storage.h
  3rdparty/agg/include/agg_path_storage_integer.h
  3rdparty/agg/include/agg_pattern_filters_rgba.h
  3rdparty/agg/include/agg_pixfmt_amask_adaptor.h
  3rdparty/agg/include/agg_pixfmt_base.h
  3rdparty/agg/include/agg_pixfmt_gray.h
  3rdparty/agg/include/agg_pixfmt_rgba.h
  3rdparty/agg/include/agg_pixfmt_rgb.h
  3rdparty/agg/include/agg_pixfmt_rgb_packed.h
  3rdparty/agg/include/agg_pixfmt_transposer.h
  3rdparty/agg/include/agg_rasterizer_cells_aa.h
  3rdparty/agg/include/agg_rasterizer_compound_aa.h
  3rdparty/agg/include/agg_rasterizer_outline_aa.h
  3rdparty/agg/include/agg_rasterizer_outline.h
  3rdparty/agg/include/agg_rasterizer_scanline_aa.h
  3rdparty/agg/include/agg_rasterizer_scanline_aa_nogamma.h
  3rdparty/agg/include/agg_rasterizer_sl_clip.h
  3rdparty/agg/include/agg_renderer_base.h
  3rdparty/agg/include/agg_renderer_markers.h
  3rdparty/agg/include/agg_renderer_mclip.h
  3rdparty/agg/include/agg_renderer_outline_aa.h
  3rdparty/agg/include/agg_renderer_outline_image.h
  3rdparty/agg/include/agg_renderer_primitives.h
  3rdparty/agg/include/agg_renderer_raster_text.h
  3rdparty/agg/include/agg_renderer_scanline.h
  3rdparty/agg/include/agg_rendering_buffer_dynarow.h
  3rdparty/agg/include/agg_rendering_buffer.h
  3rdparty/agg/include/agg_rounded_rect.h
  3rdparty/agg/include/agg_scanline_bin.h
  3rdparty/agg/include/agg_scanline_boolean_algebra.h
  3rdparty/agg/include/agg_scanline_p.h
  3rdparty/agg/include/agg_scanline_storage_aa.h
  3rdparty/agg/include/agg_scanline_storage_bin.h
  3rdparty/agg/include/agg_scanline_u.h
  3rdparty/agg/include/agg_shorten_path.h
  3rdparty/agg/include/agg_simul_eq.h
  3rdparty/agg/include/agg_span_allocator.h
  3rdparty/agg/include/agg_span_converter.h
  3rdparty/agg/include/agg_span_gouraud_gray.h
  3rdparty/agg/include/agg_span_gouraud.h
  3rdparty/agg/include/agg_span_gouraud_rgba.h
  3rdparty/agg/include/agg_span_gradient_alpha.h
  3rdparty/agg/include/agg_span_gradient_contour.h
  3rdparty/agg/include/agg_span_gradient.h
  3rdparty/agg/include/agg_span_gradient_image.h
  3rdparty/agg/include/agg_span_image_filter_gray.h
  3rdparty/agg/include/agg_span_image_filter.h
  3rdparty/agg/include/agg_span_image_filter_rgba.h
  3rdparty/agg/include/agg_span_image_filter_rgb.h
  3rdparty/agg/include/agg_span_interpolator_adaptor.h
  3rdparty/agg/include/agg_span_interpolator_linear.h
  3rdparty/agg/include/agg_span_interpolator_persp.h
  3rdparty/agg/include/agg_span_interpolator_trans.h
  3rdparty/agg/include/agg_span_pattern_gray.h
  3rdparty/agg/include/agg_span_pattern_rgba.h
  3rdparty/agg/include/agg_span_pattern_rgb.h
  3rdparty/agg/include/agg_span_solid.h
  3rdparty/agg/include/agg_span_subdiv_adaptor.h
  3rdparty/agg/include/agg_trans_affine.h
  3rdparty/agg/include/agg_trans_bilinear.h
  3rdparty/agg/include/agg_trans_double_path.h
  3rdparty/agg/include/agg_trans_perspective.h
  3rdparty/agg/include/agg_trans_single_path.h
  3rdparty/agg/include/agg_trans_viewport.h
  3rdparty/agg/include/agg_trans_warp_magnifier.h
  3rdparty/agg/include/agg_vcgen_bspline.h
  3rdparty/agg/include/agg_vcgen_contour.h
  3rdparty/agg/include/agg_vcgen_dash.h
  3rdparty/agg/include/agg_vcgen_markers_term.h
  3rdparty/agg/include/agg_vcgen_smooth_poly1.h
  3rdparty/agg/include/agg_vcgen_stroke.h
  3rdparty/agg/include/agg_vcgen_vertex_sequence.h
  3rdparty/agg/include/agg_vertex_sequence.h
  3rdparty/agg/include/agg_vpgen_clip_polygon.h
  3rdparty/agg/include/agg_vpgen_clip_polyline.h
  3rdparty/agg/include/agg_vpgen_segmentator.h
  3rdparty/agg/include/util/agg_color_conv.h
  3rdparty/agg/include/util/agg_color_conv_rgb16.h
  3rdparty/agg/include/util/agg_color_conv_rgb8.h
  3rdparty/agg/src/agg_arc.cpp
  3rdparty/agg/src/agg_arrowhead.cpp
  3rdparty/agg/src/agg_bezier_arc.cpp
  3rdparty/agg/src/agg_bspline.cpp
  3rdparty/agg/src/agg_color_rgba.cpp
  3rdparty/agg/src/agg_curves.cpp
  3rdparty/agg/src/agg_embedded_raster_fonts.cpp
  3rdparty/agg/src/agg_gsv_text.cpp
  3rdparty/agg/src/agg_image_filters.cpp
  3rdparty/agg/src/agg_line_aa_basics.cpp
  3rdparty/agg/src/agg_line_profile_aa.cpp
  3rdparty/agg/src/agg_rounded_rect.cpp
  3rdparty/agg/src/agg_sqrt_tables.cpp
  3rdparty/agg/src/agg_trans_affine.cpp
  3rdparty/agg/src/agg_trans_double_path.cpp
  3rdparty/agg/src/agg_trans_single_path.cpp
  3rdparty/agg/src/agg_trans_warp_magnifier.cpp
  3rdparty/agg/src/agg_vcgen_bspline.cpp
  3rdparty/agg/src/agg_vcgen_contour.cpp
  3rdparty/agg/src/agg_vcgen_dash.cpp
  3rdparty/agg/src/agg_vcgen_markers_term.cpp
  3rdparty/agg/src/agg_vcgen_smooth_poly1.cpp
  3rdparty/agg/src/agg_vcgen_stroke.cpp
  3rdparty/agg/src/agg_vpgen_clip_polygon.cpp
  3rdparty/agg/src/agg_vpgen_clip_polyline.cpp
  3rdparty/agg/src/agg_vpgen_segmentator.cpp
)

list(APPEND BLBENCH_DEFINITIONS BLBENCH_ENABLE_AGG)
list(APPEND BLBENCH_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/3rdparty/agg/include")

# Bench Module - Cairo
# --------------------

pkg_check_modules(CAIRO cairo)
if(CAIRO_FOUND)
  message("-- [blbench] adding support for Cairo")
  list(APPEND BLBENCH_LIBRARIES ${CAIRO_LIBRARIES})
  list(APPEND BLBENCH_DEFINITIONS BLBENCH_ENABLE_CAIRO)
  list(APPEND BLBENCH_INCLUDE_DIRS ${CAIRO_INCLUDE_DIRS})
endif()

# Bench Module - Qt5
# ------------------

find_package(Qt5Gui)
if(Qt5Gui_FOUND)
  message("-- [blbench] adding support for Qt5")
  list(APPEND BLBENCH_LIBRARIES Qt5::Gui)
  list(APPEND BLBENCH_DEFINITIONS BLBENCH_ENABLE_QT)
endif()

# Bench Executable
# ----------------

add_executable(bl_bench ${BLBENCH_SRC} ${AGG_SRC})
set_property(TARGET bl_bench PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET bl_bench PROPERTY COMPILE_DEFINITIONS "${BLBENCH_DEFINITIONS}")
set_property(TARGET bl_bench PROPERTY INCLUDE_DIRECTORIES "${BLBENCH_INCLUDE_DIRS}")
target_link_libraries(bl_bench ${BLBENCH_LIBRARIES})
