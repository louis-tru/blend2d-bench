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
	return 0;
}

static uint32_t toPlutovgOperator(BLCompOp compOp) {
	switch (compOp) {
		case BL_COMP_OP_SRC_COPY: return plutovg_operator_src;
		case BL_COMP_OP_SRC_OVER: return plutovg_operator_src_over;
		case BL_COMP_OP_DST_IN    : return plutovg_operator_dst_in;
		case BL_COMP_OP_DST_OUT    : return plutovg_operator_dst_out;
		default:
			return 0xFFFFFFFFu;
	}
}

static void roundRect(plutovg_t* ctx, const BLRect& rect, double radius) {
	double rw2 = rect.w * 0.5;
	double rh2 = rect.h * 0.5;

	double rx = std::min(blAbs(radius), rw2);
	double ry = std::min(blAbs(radius), rh2);

	double kappaInv = 1 - 0.551915024494;
	double kx = rx * kappaInv;
	double ky = ry * kappaInv;

	double x0 = rect.x;
	double y0 = rect.y;
	double x1 = rect.x + rect.w;
	double y1 = rect.y + rect.h;

	plutovg_move_to(ctx, x0 + rx, y0);
	plutovg_line_to(ctx, x1 - rx, y0);
	plutovg_cubic_to(ctx, x1 - kx, y0, x1, y0 + ky, x1, y0 + ry);

	plutovg_line_to(ctx, x1, y1 - ry);
	plutovg_cubic_to(ctx, x1, y1 - ky, x1 - kx, y1, x1 - rx, y1);

	plutovg_line_to(ctx, x0 + rx, y1);
	plutovg_cubic_to(ctx, x0 + kx, y1, x0, y1 - ky, x0, y1 - ry);

	plutovg_line_to(ctx, x0, y0 + ry);
	plutovg_cubic_to(ctx, x0, y0 + ky, x0 + kx, y0, x0 + rx, y0);

	plutovg_close_path(ctx);
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
	switch (style) {
		case kBenchStyleSolid: {
			BLRgba32 c(_rndColor.nextRgba32());
			plutovg_set_source_rgba(_PlutovgContext, u8ToUnit(c.r()), u8ToUnit(c.g()), u8ToUnit(c.b()), u8ToUnit(c.a()));
			return;
		}

		case kBenchStyleLinearPad:
		case kBenchStyleLinearRepeat:
		case kBenchStyleLinearReflect:
		case kBenchStyleRadialPad:
		case kBenchStyleRadialRepeat:
		case kBenchStyleRadialReflect: {
			double x = rect.x;
			double y = rect.y;

			BLRgba32 c0(_rndColor.nextRgba32());
			BLRgba32 c1(_rndColor.nextRgba32());
			BLRgba32 c2(_rndColor.nextRgba32());

			plutovg_gradient_t* gradient = NULL;
			if (style < kBenchStyleRadialPad) {
				// Linear gradient.
				double x0 = rect.x + rect.w * 0.2;
				double y0 = rect.y + rect.h * 0.2;
				double x1 = rect.x + rect.w * 0.8;
				double y1 = rect.y + rect.h * 0.8;
				gradient = plutovg_gradient_create_linear(x0, y0, x1, y1);

				plutovg_gradient_add_stop_rgba(gradient, 0.0, u8ToUnit(c0.r()), u8ToUnit(c0.g()), u8ToUnit(c0.b()), u8ToUnit(c0.a()));
				plutovg_gradient_add_stop_rgba(gradient, 0.5, u8ToUnit(c1.r()), u8ToUnit(c1.g()), u8ToUnit(c1.b()), u8ToUnit(c1.a()));
				plutovg_gradient_add_stop_rgba(gradient, 1.0, u8ToUnit(c2.r()), u8ToUnit(c2.g()), u8ToUnit(c2.b()), u8ToUnit(c2.a()));
			}
			else {
				// Radial gradient.
				x += double(rect.w) / 2.0;
				y += double(rect.h) / 2.0;

				double r = double(rect.w + rect.h) / 4.0;
				gradient = plutovg_gradient_create_radial(x, y, r, x - r / 2, y - r / 2, 0.0);

				// Color stops in Plutovg's radial gradient are reverse to Blend/Qt.
				plutovg_gradient_add_stop_rgba(gradient, 0.0, u8ToUnit(c2.r()), u8ToUnit(c2.g()), u8ToUnit(c2.b()), u8ToUnit(c2.a()));
				plutovg_gradient_add_stop_rgba(gradient, 0.5, u8ToUnit(c1.r()), u8ToUnit(c1.g()), u8ToUnit(c1.b()), u8ToUnit(c1.a()));
				plutovg_gradient_add_stop_rgba(gradient, 1.0, u8ToUnit(c0.r()), u8ToUnit(c0.g()), u8ToUnit(c0.b()), u8ToUnit(c0.a()));
			}

			switch(style) {
				case kBenchStyleLinearPad:
				case kBenchStyleRadialPad:
					plutovg_gradient_set_spread(gradient, plutovg_spread_method_pad);
					break;
				case kBenchStyleLinearRepeat:
				case kBenchStyleRadialRepeat:
					plutovg_gradient_set_spread(gradient, plutovg_spread_method_repeat);
					break;
				case kBenchStyleLinearReflect:
				case kBenchStyleRadialReflect:
					plutovg_gradient_set_spread(gradient, plutovg_spread_method_reflect);
					break;
			}

			plutovg_set_source_gradient(_PlutovgContext, gradient);
			plutovg_gradient_destroy(gradient);
			return;
		}

		case kBenchStylePatternNN: {
		// case kBenchStylePatternBI: {
			// Matrix associated with Plutovg_pattern_t is inverse to Blend/Qt.
			plutovg_matrix_t matrix;
			plutovg_matrix_init_translate(&matrix, -rect.x, -rect.y);

			plutovg_texture_t* texture = plutovg_texture_create(_PlutovgSprites[nextSpriteId()]);
			plutovg_texture_set_matrix(texture, &matrix);
			// Plutovg_pattern_set_extend(pattern, Plutovg_extend_t(_patternExtend));
			// Plutovg_pattern_set_filter(pattern, Plutovg_filter_t(_patternFilter));
			plutovg_set_source_texture(_PlutovgContext, texture);
			plutovg_texture_destroy(texture);
			return;
		}
	}
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
		// int format = toPlutovgFormat(spriteData.format);
		unsigned char* pixels = static_cast<unsigned char*>(spriteData.pixelData);

		plutovg_surface_t* PlutovgSprite = plutovg_surface_create_for_data(
			pixels, /*Plutovg_format_t(format),*/ spriteData.size.w, spriteData.size.h, stride);

		_PlutovgSprites[i] = PlutovgSprite;
	}

	// Initialize the surface and the context.
	{
		BLImageData surfaceData;
		_surface.create(w, h, _params.format);
		_surface.makeMutable(&surfaceData);

		int stride = int(surfaceData.stride);
		// int format = toPlutovgFormat(surfaceData.format);
		unsigned char* pixels = (unsigned char*)surfaceData.pixelData;

		_PlutovgSurface = plutovg_surface_create_for_data(
			pixels, /*Plutovg_format_t(format),*/ w, h, stride);

		if (_PlutovgSurface == NULL)
			return;

		_PlutovgContext = plutovg_create(_PlutovgSurface);
		if (_PlutovgContext == NULL)
			return;
	}

	// Setup the context.
	plutovg_set_operator(_PlutovgContext, plutovg_operator_src);
	plutovg_set_source_rgb(_PlutovgContext, 0, 0, 0);
	plutovg_rect(_PlutovgContext, 0, 0, w, h);
	plutovg_fill(_PlutovgContext);

	plutovg_set_operator(_PlutovgContext, plutovg_operator_t(toPlutovgOperator(_params.compOp)));
	plutovg_set_line_width(_PlutovgContext, _params.strokeWidth);
}

void PlutovgModule::onAfterRun() {
	// Free the surface & the context.
	plutovg_destroy(_PlutovgContext);
	plutovg_surface_destroy(_PlutovgSurface);

	_PlutovgContext = NULL;
	_PlutovgSurface = NULL;

	// Free the sprites.
	for (uint32_t i = 0; i < kBenchNumSprites; i++) {
		plutovg_surface_destroy(_PlutovgSprites[i]);
		_PlutovgSprites[i] = NULL;
	}
}

void PlutovgModule::onDoRectAligned(bool stroke) {
	BLSizeI bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	int wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRectI rect(_rndCoord.nextRectI(bounds, wh, wh));
		setupStyle<BLRectI>(style, rect);

		if (stroke) {
			plutovg_rect(_PlutovgContext, rect.x + 0.5, rect.y + 0.5, rect.w, rect.h);
			plutovg_stroke(_PlutovgContext);
		}
		else {
			plutovg_rect(_PlutovgContext, rect.x, rect.y, rect.w, rect.h);
			plutovg_fill(_PlutovgContext);
		}
	}
}

void PlutovgModule::onDoRectSmooth(bool stroke) {
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));

		setupStyle<BLRect>(style, rect);
		plutovg_rect(_PlutovgContext, rect.x, rect.y, rect.w, rect.h);

		if (stroke)
			plutovg_stroke(_PlutovgContext);
		else
			plutovg_fill(_PlutovgContext);
	}
}

void PlutovgModule::onDoRectRotated(bool stroke) {
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double cx = double(_params.screenW) * 0.5;
	double cy = double(_params.screenH) * 0.5;
	double wh = _params.shapeSize;
	double angle = 0.0;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));

		plutovg_translate(_PlutovgContext, cx, cy);
		plutovg_rotate(_PlutovgContext, angle);
		plutovg_translate(_PlutovgContext, -cx, -cy);

		setupStyle<BLRect>(style, rect);
		plutovg_rect(_PlutovgContext, rect.x, rect.y, rect.w, rect.h);

		if (stroke)
			plutovg_stroke(_PlutovgContext);
		else
			plutovg_fill(_PlutovgContext);

		plutovg_identity_matrix(_PlutovgContext);
	}
}

void PlutovgModule::onDoRoundSmooth(bool stroke) {
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));
		double radius = _rndExtra.nextDouble(4.0, 40.0);

		setupStyle<BLRect>(style, rect);
		roundRect(_PlutovgContext, rect, radius);

		if (stroke)
			plutovg_stroke(_PlutovgContext);
		else
			plutovg_fill(_PlutovgContext);
	}
}

void PlutovgModule::onDoRoundRotated(bool stroke) {
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double cx = double(_params.screenW) * 0.5;
	double cy = double(_params.screenH) * 0.5;
	double wh = _params.shapeSize;
	double angle = 0.0;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));
		double radius = _rndExtra.nextDouble(4.0, 40.0);

		plutovg_translate(_PlutovgContext, cx, cy);
		plutovg_rotate(_PlutovgContext, angle);
		plutovg_translate(_PlutovgContext, -cx, -cy);

		setupStyle<BLRect>(style, rect);
		roundRect(_PlutovgContext, rect, radius);

		if (stroke)
			plutovg_stroke(_PlutovgContext);
		else
			plutovg_fill(_PlutovgContext);

		plutovg_identity_matrix(_PlutovgContext);
	}
}

void PlutovgModule::onDoPolygon(uint32_t mode, uint32_t complexity) {
	BLSizeI bounds(_params.screenW - _params.shapeSize,
								 _params.screenH - _params.shapeSize);
	uint32_t style = _params.style;
	bool stroke = (mode == 2);

	double wh = double(_params.shapeSize);

	if (mode == 0) plutovg_set_fill_rule(_PlutovgContext, plutovg_fill_rule_non_zero);
	if (mode == 1) plutovg_set_fill_rule(_PlutovgContext, plutovg_fill_rule_even_odd);

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLPoint base(_rndCoord.nextPoint(bounds));

		double x = _rndCoord.nextDouble(base.x, base.x + wh);
		double y = _rndCoord.nextDouble(base.y, base.y + wh);

		plutovg_move_to(_PlutovgContext, x, y);
		for (uint32_t p = 1; p < complexity; p++) {
			x = _rndCoord.nextDouble(base.x, base.x + wh);
			y = _rndCoord.nextDouble(base.y, base.y + wh);
			plutovg_line_to(_PlutovgContext, x, y);
		}
		setupStyle<BLRect>(style, BLRect(base.x, base.y, wh, wh));

		if (stroke)
			plutovg_stroke(_PlutovgContext);
		else
			plutovg_fill(_PlutovgContext);
	}
}

void PlutovgModule::onDoShape(bool stroke, const BLPoint* pts, size_t count) {
	BLSizeI bounds(_params.screenW - _params.shapeSize,
								 _params.screenH - _params.shapeSize);
	uint32_t style = _params.style;

	// No idea who invented this, but you need a `Plutovg_t` to create a `Plutovg_path_t`.
	plutovg_path_t* path = nullptr;

	bool start = true;
	double wh = double(_params.shapeSize);

	for (size_t i = 0; i < count; i++) {
		double x = pts[i].x;
		double y = pts[i].y;

		if (x == -1.0 && y == -1.0) {
			start = true;
			continue;
		}

		if (start) {
			plutovg_move_to(_PlutovgContext, x * wh, y * wh);
			start = false;
		}
		else {
			plutovg_line_to(_PlutovgContext, x * wh, y * wh);
		}
	}

	path = plutovg_path_clone(plutovg_get_path(_PlutovgContext));
	plutovg_new_path(_PlutovgContext);

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		plutovg_save(_PlutovgContext);

		BLPoint base(_rndCoord.nextPoint(bounds));
		setupStyle<BLRect>(style, BLRect(base.x, base.y, wh, wh));

		plutovg_translate(_PlutovgContext, base.x, base.y);
		plutovg_add_path(_PlutovgContext, path);

		if (stroke)
			plutovg_stroke(_PlutovgContext);
		else
			plutovg_fill(_PlutovgContext);

		plutovg_restore(_PlutovgContext);
	}

	plutovg_path_destroy(path);
}

} // {blbench}

// #endif // BLBENCH_ENABLE_Plutovg
