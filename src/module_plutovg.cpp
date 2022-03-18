// Blend2D - 2D Vector Graphics Powered by a JIT Compiler
//
//  * Official Blend2D Home Page: https://blend2d.com
//  * Official Github Repository: https://github.com/blend2d/blend2d
//
// Copyright (c) 2017-2020 The Blend2D Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

// #ifdef BLBENCH_ENABLE_PLUTOVG

#include "./app.h"
#include "./module_plutovg.h"

#include <algorithm>

namespace blbench {

static inline double u8ToUnit(int x) {
	static const double kDiv255 = 1.0 / 255.0;
	return double(x) * kDiv255;
}

static uint32_t toPlutovgFormat(uint32_t format) {
	switch (format) {
		case BL_FORMAT_PRGB32: return CAIRO_FORMAT_ARGB32;
		case BL_FORMAT_XRGB32: return CAIRO_FORMAT_RGB24;
		case BL_FORMAT_A8    : return CAIRO_FORMAT_A8;
		default:
			return 0xFFFFFFFFu;
	}

  // BL_FORMAT_NONE = 0,
  // //! 32-bit premultiplied ARGB pixel format (8-bit components).
  // BL_FORMAT_PRGB32 = 1,
  // //! 32-bit (X)RGB pixel format (8-bit components, alpha ignored).
  // BL_FORMAT_XRGB32 = 2,
  // //! 8-bit alpha-only pixel format.
  // BL_FORMAT_A8 = 3,

  // // Maximum value of `BLFormat`.
  // BL_FORMAT_MAX_VALUE = 3,
  // //! Count of pixel formats (reserved for future use).
  // BL_FORMAT_RESERVED_COUNT = 16

}

// ============================================================================
// [bench::PlutovgModule - Construction / Destruction]
// ============================================================================

PlutovgModule::PlutovgModule() {
	strcpy(_name, "Plutovg");
	_PlutovgSurface = NULL;
	_PlutovgContext = NULL;
	memset(_PlutovgSprites, 0, sizeof(_PlutovgSprites));
}

PlutovgModule::~PlutovgModule() {}

// ============================================================================
// [bench::PlutovgModule - Helpers]
// ============================================================================

template<typename RectT>
void PlutovgModule::setupStyle(uint32_t style, const RectT& rect) {
	// switch (style) {
	// 	case kBenchStyleSolid: {
	// 		BLRgba32 c(_rndColor.nextRgba32());
	// 		Plutovg_set_source_rgba(_PlutovgContext, u8ToUnit(c.r()), u8ToUnit(c.g()), u8ToUnit(c.b()), u8ToUnit(c.a()));
	// 		return;
	// 	}

	// 	case kBenchStyleLinearPad:
	// 	case kBenchStyleLinearRepeat:
	// 	case kBenchStyleLinearReflect:
	// 	case kBenchStyleRadialPad:
	// 	case kBenchStyleRadialRepeat:
	// 	case kBenchStyleRadialReflect: {
	// 		double x = rect.x;
	// 		double y = rect.y;

	// 		BLRgba32 c0(_rndColor.nextRgba32());
	// 		BLRgba32 c1(_rndColor.nextRgba32());
	// 		BLRgba32 c2(_rndColor.nextRgba32());

	// 		Plutovg_pattern_t* pattern = NULL;
	// 		if (style < kBenchStyleRadialPad) {
	// 			// Linear gradient.
	// 			double x0 = rect.x + rect.w * 0.2;
	// 			double y0 = rect.y + rect.h * 0.2;
	// 			double x1 = rect.x + rect.w * 0.8;
	// 			double y1 = rect.y + rect.h * 0.8;
	// 			pattern = Plutovg_pattern_create_linear(x0, y0, x1, y1);

	// 			Plutovg_pattern_add_color_stop_rgba(pattern, 0.0, u8ToUnit(c0.r()), u8ToUnit(c0.g()), u8ToUnit(c0.b()), u8ToUnit(c0.a()));
	// 			Plutovg_pattern_add_color_stop_rgba(pattern, 0.5, u8ToUnit(c1.r()), u8ToUnit(c1.g()), u8ToUnit(c1.b()), u8ToUnit(c1.a()));
	// 			Plutovg_pattern_add_color_stop_rgba(pattern, 1.0, u8ToUnit(c2.r()), u8ToUnit(c2.g()), u8ToUnit(c2.b()), u8ToUnit(c2.a()));
	// 		}
	// 		else {
	// 			// Radial gradient.
	// 			x += double(rect.w) / 2.0;
	// 			y += double(rect.h) / 2.0;

	// 			double r = double(rect.w + rect.h) / 4.0;
	// 			pattern = Plutovg_pattern_create_radial(x, y, r, x - r / 2, y - r / 2, 0.0);

	// 			// Color stops in Plutovg's radial gradient are reverse to Blend/Qt.
	// 			Plutovg_pattern_add_color_stop_rgba(pattern, 0.0, u8ToUnit(c2.r()), u8ToUnit(c2.g()), u8ToUnit(c2.b()), u8ToUnit(c2.a()));
	// 			Plutovg_pattern_add_color_stop_rgba(pattern, 0.5, u8ToUnit(c1.r()), u8ToUnit(c1.g()), u8ToUnit(c1.b()), u8ToUnit(c1.a()));
	// 			Plutovg_pattern_add_color_stop_rgba(pattern, 1.0, u8ToUnit(c0.r()), u8ToUnit(c0.g()), u8ToUnit(c0.b()), u8ToUnit(c0.a()));
	// 		}

	// 		Plutovg_pattern_set_extend(pattern, Plutovg_extend_t(_patternExtend));
	// 		Plutovg_set_source(_PlutovgContext, pattern);
	// 		Plutovg_pattern_destroy(pattern);
	// 		return;
	// 	}

	// 	case kBenchStylePatternNN:
	// 	case kBenchStylePatternBI: {
	// 		// Matrix associated with Plutovg_pattern_t is inverse to Blend/Qt.
	// 		Plutovg_matrix_t matrix;
	// 		Plutovg_matrix_init_translate(&matrix, -rect.x, -rect.y);

	// 		Plutovg_pattern_t* pattern = Plutovg_pattern_create_for_surface(_PlutovgSprites[nextSpriteId()]);
	// 		Plutovg_pattern_set_matrix(pattern, &matrix);
	// 		Plutovg_pattern_set_extend(pattern, Plutovg_extend_t(_patternExtend));
	// 		Plutovg_pattern_set_filter(pattern, Plutovg_filter_t(_patternFilter));

	// 		Plutovg_set_source(_PlutovgContext, pattern);
	// 		Plutovg_pattern_destroy(pattern);
	// 		return;
	// 	}
	// }
}

// ============================================================================
// [bench::PlutovgModule - Interface]
// ============================================================================

bool PlutovgModule::supportsCompOp(uint32_t compOp) const {
	return compOp == BL_COMP_OP_SRC_COPY || // plutovg_operator_src
				compOp == BL_COMP_OP_SRC_OVER || // plutovg_operator_src
				compOp == BL_COMP_OP_DST_IN || // plutovg_operator_dst_in
				compOp == BL_COMP_OP_DST_OUT;  // plutovg_operator_dst_out

	return false;
}

bool PlutovgModule::supportsStyle(uint32_t style) const {
	return style == kBenchStyleSolid         ||
				 style == kBenchStyleLinearPad     ||
				 style == kBenchStyleLinearRepeat  ||
				 style == kBenchStyleLinearReflect ||
				 style == kBenchStyleRadialPad     ||
				 style == kBenchStyleRadialRepeat  ||
				 style == kBenchStyleRadialReflect ||
				 style == kBenchStylePatternNN;   // kNearest
				//  style == kBenchStylePatternBI     ; // kLinear

	return false;
}

void PlutovgModule::onBeforeRun() {
	int w = int(_params.screenW);
	int h = int(_params.screenH);
	uint32_t style = _params.style;

	// Initialize the sprites.
	for (uint32_t i = 0; i < kBenchNumSprites; i++) {
		const BLImage& sprite = _sprites[i];

		BLImageData spriteData;
		sprite.getData(&spriteData);

		int stride = int(spriteData.stride);
		int format = toPlutovgFormat(spriteData.format);
		unsigned char* pixels = static_cast<unsigned char*>(spriteData.pixelData);

		Plutovg_surface_t* PlutovgSprite = Plutovg_image_surface_create_for_data(
			pixels, Plutovg_format_t(format), spriteData.size.w, spriteData.size.h, stride);

		_PlutovgSprites[i] = PlutovgSprite;
	}

	// // Initialize the surface and the context.
	// {
	// 	BLImageData surfaceData;
	// 	_surface.create(w, h, _params.format);
	// 	_surface.makeMutable(&surfaceData);

	// 	int stride = int(surfaceData.stride);
	// 	int format = toPlutovgFormat(surfaceData.format);
	// 	unsigned char* pixels = (unsigned char*)surfaceData.pixelData;

	// 	_PlutovgSurface = Plutovg_image_surface_create_for_data(
	// 		pixels, Plutovg_format_t(format), w, h, stride);

	// 	if (_PlutovgSurface == NULL)
	// 		return;

	// 	_PlutovgContext = Plutovg_create(_PlutovgSurface);
	// 	if (_PlutovgContext == NULL)
	// 		return;
	// }

	// // Setup the context.
	// Plutovg_set_operator(_PlutovgContext, Plutovg_OPERATOR_CLEAR);
	// Plutovg_rectangle(_PlutovgContext, 0, 0, w, h);
	// Plutovg_fill(_PlutovgContext);

	// Plutovg_set_operator(_PlutovgContext, Plutovg_operator_t(PlutovgUtils::toPlutovgOperator(_params.compOp)));
	// Plutovg_set_line_width(_PlutovgContext, _params.strokeWidth);

	// // Setup globals.
	// _patternExtend = Plutovg_EXTEND_REPEAT;
	// _patternFilter = Plutovg_FILTER_NEAREST;

	// switch (style) {
	// 	case kBenchStyleLinearPad      : _patternExtend = Plutovg_EXTEND_PAD     ; break;
	// 	case kBenchStyleLinearRepeat   : _patternExtend = Plutovg_EXTEND_REPEAT  ; break;
	// 	case kBenchStyleLinearReflect  : _patternExtend = Plutovg_EXTEND_REFLECT ; break;
	// 	case kBenchStyleRadialPad      : _patternExtend = Plutovg_EXTEND_PAD     ; break;
	// 	case kBenchStyleRadialRepeat   : _patternExtend = Plutovg_EXTEND_REPEAT  ; break;
	// 	case kBenchStyleRadialReflect  : _patternExtend = Plutovg_EXTEND_REFLECT ; break;
	// 	case kBenchStylePatternNN      : _patternFilter = Plutovg_FILTER_NEAREST ; break;
	// 	case kBenchStylePatternBI      : _patternFilter = Plutovg_FILTER_BILINEAR; break;
	// }
}

void PlutovgModule::onAfterRun() {
	// // Free the surface & the context.
	// Plutovg_destroy(_PlutovgContext);
	// Plutovg_surface_destroy(_PlutovgSurface);

	// _PlutovgContext = NULL;
	// _PlutovgSurface = NULL;

	// // Free the sprites.
	// for (uint32_t i = 0; i < kBenchNumSprites; i++) {
	// 	Plutovg_surface_destroy(_PlutovgSprites[i]);
	// 	_PlutovgSprites[i] = NULL;
	// }
}

void PlutovgModule::onDoRectAligned(bool stroke) {
	BLSizeI bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	int wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRectI rect(_rndCoord.nextRectI(bounds, wh, wh));
		setupStyle<BLRectI>(style, rect);

		if (stroke) {
			Plutovg_rectangle(_PlutovgContext, rect.x + 0.5, rect.y + 0.5, rect.w, rect.h);
			Plutovg_stroke(_PlutovgContext);
		}
		else {
			Plutovg_rectangle(_PlutovgContext, rect.x, rect.y, rect.w, rect.h);
			Plutovg_fill(_PlutovgContext);
		}
	}
}

void PlutovgModule::onDoRectSmooth(bool stroke) {
	// BLSize bounds(_params.screenW, _params.screenH);
	// uint32_t style = _params.style;

	// double wh = _params.shapeSize;

	// for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
	// 	BLRect rect(_rndCoord.nextRect(bounds, wh, wh));

	// 	setupStyle<BLRect>(style, rect);
	// 	Plutovg_rectangle(_PlutovgContext, rect.x, rect.y, rect.w, rect.h);

	// 	if (stroke)
	// 		Plutovg_stroke(_PlutovgContext);
	// 	else
	// 		Plutovg_fill(_PlutovgContext);
	// }
}

void PlutovgModule::onDoRectRotated(bool stroke) {
	// BLSize bounds(_params.screenW, _params.screenH);
	// uint32_t style = _params.style;

	// double cx = double(_params.screenW) * 0.5;
	// double cy = double(_params.screenH) * 0.5;
	// double wh = _params.shapeSize;
	// double angle = 0.0;

	// for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
	// 	BLRect rect(_rndCoord.nextRect(bounds, wh, wh));

	// 	Plutovg_translate(_PlutovgContext, cx, cy);
	// 	Plutovg_rotate(_PlutovgContext, angle);
	// 	Plutovg_translate(_PlutovgContext, -cx, -cy);

	// 	setupStyle<BLRect>(style, rect);
	// 	Plutovg_rectangle(_PlutovgContext, rect.x, rect.y, rect.w, rect.h);

	// 	if (stroke)
	// 		Plutovg_stroke(_PlutovgContext);
	// 	else
	// 		Plutovg_fill(_PlutovgContext);

	// 	Plutovg_identity_matrix(_PlutovgContext);
	// }
}

void PlutovgModule::onDoRoundSmooth(bool stroke) {
	// BLSize bounds(_params.screenW, _params.screenH);
	// uint32_t style = _params.style;

	// double wh = _params.shapeSize;

	// for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
	// 	BLRect rect(_rndCoord.nextRect(bounds, wh, wh));
	// 	double radius = _rndExtra.nextDouble(4.0, 40.0);

	// 	setupStyle<BLRect>(style, rect);
	// 	PlutovgUtils::roundRect(_PlutovgContext, rect, radius);

	// 	if (stroke)
	// 		Plutovg_stroke(_PlutovgContext);
	// 	else
	// 		Plutovg_fill(_PlutovgContext);
	// }
}

void PlutovgModule::onDoRoundRotated(bool stroke) {
	// BLSize bounds(_params.screenW, _params.screenH);
	// uint32_t style = _params.style;

	// double cx = double(_params.screenW) * 0.5;
	// double cy = double(_params.screenH) * 0.5;
	// double wh = _params.shapeSize;
	// double angle = 0.0;

	// for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
	// 	BLRect rect(_rndCoord.nextRect(bounds, wh, wh));
	// 	double radius = _rndExtra.nextDouble(4.0, 40.0);

	// 	Plutovg_translate(_PlutovgContext, cx, cy);
	// 	Plutovg_rotate(_PlutovgContext, angle);
	// 	Plutovg_translate(_PlutovgContext, -cx, -cy);

	// 	setupStyle<BLRect>(style, rect);
	// 	PlutovgUtils::roundRect(_PlutovgContext, rect, radius);

	// 	if (stroke)
	// 		Plutovg_stroke(_PlutovgContext);
	// 	else
	// 		Plutovg_fill(_PlutovgContext);

	// 	Plutovg_identity_matrix(_PlutovgContext);
	// }
}

void PlutovgModule::onDoPolygon(uint32_t mode, uint32_t complexity) {
	// BLSizeI bounds(_params.screenW - _params.shapeSize,
	// 							 _params.screenH - _params.shapeSize);
	// uint32_t style = _params.style;
	// bool stroke = (mode == 2);

	// double wh = double(_params.shapeSize);

	// if (mode == 0) Plutovg_set_fill_rule(_PlutovgContext, Plutovg_FILL_RULE_WINDING);
	// if (mode == 1) Plutovg_set_fill_rule(_PlutovgContext, Plutovg_FILL_RULE_EVEN_ODD);

	// for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
	// 	BLPoint base(_rndCoord.nextPoint(bounds));

	// 	double x = _rndCoord.nextDouble(base.x, base.x + wh);
	// 	double y = _rndCoord.nextDouble(base.y, base.y + wh);

	// 	Plutovg_move_to(_PlutovgContext, x, y);
	// 	for (uint32_t p = 1; p < complexity; p++) {
	// 		x = _rndCoord.nextDouble(base.x, base.x + wh);
	// 		y = _rndCoord.nextDouble(base.y, base.y + wh);
	// 		Plutovg_line_to(_PlutovgContext, x, y);
	// 	}
	// 	setupStyle<BLRect>(style, BLRect(base.x, base.y, wh, wh));

	// 	if (stroke)
	// 		Plutovg_stroke(_PlutovgContext);
	// 	else
	// 		Plutovg_fill(_PlutovgContext);
	// }
}

void PlutovgModule::onDoShape(bool stroke, const BLPoint* pts, size_t count) {
	// BLSizeI bounds(_params.screenW - _params.shapeSize,
	// 							 _params.screenH - _params.shapeSize);
	// uint32_t style = _params.style;

	// // No idea who invented this, but you need a `Plutovg_t` to create a `Plutovg_path_t`.
	// Plutovg_path_t* path = nullptr;

	// bool start = true;
	// double wh = double(_params.shapeSize);

	// for (size_t i = 0; i < count; i++) {
	// 	double x = pts[i].x;
	// 	double y = pts[i].y;

	// 	if (x == -1.0 && y == -1.0) {
	// 		start = true;
	// 		continue;
	// 	}

	// 	if (start) {
	// 		Plutovg_move_to(_PlutovgContext, x * wh, y * wh);
	// 		start = false;
	// 	}
	// 	else {
	// 		Plutovg_line_to(_PlutovgContext, x * wh, y * wh);
	// 	}
	// }

	// path = Plutovg_copy_path(_PlutovgContext);
	// Plutovg_new_path(_PlutovgContext);

	// for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
	// 	Plutovg_save(_PlutovgContext);

	// 	BLPoint base(_rndCoord.nextPoint(bounds));
	// 	setupStyle<BLRect>(style, BLRect(base.x, base.y, wh, wh));

	// 	Plutovg_translate(_PlutovgContext, base.x, base.y);
	// 	Plutovg_append_path(_PlutovgContext, path);

	// 	if (stroke)
	// 		Plutovg_stroke(_PlutovgContext);
	// 	else
	// 		Plutovg_fill(_PlutovgContext);

	// 	Plutovg_restore(_PlutovgContext);
	// }

	// Plutovg_path_destroy(path);
}

} // {blbench}

// #endif // BLBENCH_ENABLE_Plutovg
