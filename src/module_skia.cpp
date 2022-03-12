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

#ifdef BLBENCH_ENABLE_SKIA

#include "./app.h"
#include "./module_skia.h"

#include <algorithm>

namespace blbench {

// ============================================================================
// [bench::SkiaModule - Construction / Destruction]
// ============================================================================

SkiaModule::SkiaModule() {
	strcpy(_name, "Skia");
	_cairoSurface = NULL;
	_cairoContext = NULL;
	memset(_cairoSprites, 0, sizeof(_cairoSprites));
}
SkiaModule::~SkiaModule() {}

// ============================================================================
// [bench::SkiaModule - Helpers]
// ============================================================================

template<typename RectT>
void SkiaModule::setupStyle(uint32_t style, const RectT& rect) {
	switch (style) {
		case kBenchStyleSolid: {
			BLRgba32 c(_rndColor.nextRgba32());
			cairo_set_source_rgba(_cairoContext, u8ToUnit(c.r()), u8ToUnit(c.g()), u8ToUnit(c.b()), u8ToUnit(c.a()));
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

			cairo_pattern_t* pattern = NULL;
			if (style < kBenchStyleRadialPad) {
				// Linear gradient.
				double x0 = rect.x + rect.w * 0.2;
				double y0 = rect.y + rect.h * 0.2;
				double x1 = rect.x + rect.w * 0.8;
				double y1 = rect.y + rect.h * 0.8;
				pattern = cairo_pattern_create_linear(x0, y0, x1, y1);

				cairo_pattern_add_color_stop_rgba(pattern, 0.0, u8ToUnit(c0.r()), u8ToUnit(c0.g()), u8ToUnit(c0.b()), u8ToUnit(c0.a()));
				cairo_pattern_add_color_stop_rgba(pattern, 0.5, u8ToUnit(c1.r()), u8ToUnit(c1.g()), u8ToUnit(c1.b()), u8ToUnit(c1.a()));
				cairo_pattern_add_color_stop_rgba(pattern, 1.0, u8ToUnit(c2.r()), u8ToUnit(c2.g()), u8ToUnit(c2.b()), u8ToUnit(c2.a()));
			}
			else {
				// Radial gradient.
				x += double(rect.w) / 2.0;
				y += double(rect.h) / 2.0;

				double r = double(rect.w + rect.h) / 4.0;
				pattern = cairo_pattern_create_radial(x, y, r, x - r / 2, y - r / 2, 0.0);

				// Color stops in Cairo's radial gradient are reverse to Blend/Qt.
				cairo_pattern_add_color_stop_rgba(pattern, 0.0, u8ToUnit(c2.r()), u8ToUnit(c2.g()), u8ToUnit(c2.b()), u8ToUnit(c2.a()));
				cairo_pattern_add_color_stop_rgba(pattern, 0.5, u8ToUnit(c1.r()), u8ToUnit(c1.g()), u8ToUnit(c1.b()), u8ToUnit(c1.a()));
				cairo_pattern_add_color_stop_rgba(pattern, 1.0, u8ToUnit(c0.r()), u8ToUnit(c0.g()), u8ToUnit(c0.b()), u8ToUnit(c0.a()));
			}

			cairo_pattern_set_extend(pattern, cairo_extend_t(_patternExtend));
			cairo_set_source(_cairoContext, pattern);
			cairo_pattern_destroy(pattern);
			return;
		}

		case kBenchStylePatternNN:
		case kBenchStylePatternBI: {
			// Matrix associated with cairo_pattern_t is inverse to Blend/Qt.
			cairo_matrix_t matrix;
			cairo_matrix_init_translate(&matrix, -rect.x, -rect.y);

			cairo_pattern_t* pattern = cairo_pattern_create_for_surface(_cairoSprites[nextSpriteId()]);
			cairo_pattern_set_matrix(pattern, &matrix);
			cairo_pattern_set_extend(pattern, cairo_extend_t(_patternExtend));
			cairo_pattern_set_filter(pattern, cairo_filter_t(_patternFilter));

			cairo_set_source(_cairoContext, pattern);
			cairo_pattern_destroy(pattern);
			return;
		}
	}
}

// ============================================================================
// [bench::SkiaModule - Interface]
// ============================================================================

bool SkiaModule::supportsCompOp(uint32_t compOp) const {
	return CairoUtils::toCairoOperator(compOp) != 0xFFFFFFFFu;
}

bool SkiaModule::supportsStyle(uint32_t style) const {
	return style == kBenchStyleSolid         ||
				 style == kBenchStyleLinearPad     ||
				 style == kBenchStyleLinearRepeat  ||
				 style == kBenchStyleLinearReflect ||
				 style == kBenchStyleRadialPad     ||
				 style == kBenchStyleRadialRepeat  ||
				 style == kBenchStyleRadialReflect ||
				 style == kBenchStylePatternNN     ||
				 style == kBenchStylePatternBI     ;
}

void SkiaModule::onBeforeRun() {
	int w = int(_params.screenW);
	int h = int(_params.screenH);
	uint32_t style = _params.style;

	// Initialize the sprites.
	for (uint32_t i = 0; i < kBenchNumSprites; i++) {
		const BLImage& sprite = _sprites[i];

		BLImageData spriteData;
		sprite.getData(&spriteData);

		int stride = int(spriteData.stride);
		int format = CairoUtils::toCairoFormat(spriteData.format);
		unsigned char* pixels = static_cast<unsigned char*>(spriteData.pixelData);

		cairo_surface_t* cairoSprite = cairo_image_surface_create_for_data(
			pixels, cairo_format_t(format), spriteData.size.w, spriteData.size.h, stride);

		_cairoSprites[i] = cairoSprite;
	}

	// Initialize the surface and the context.
	{
		BLImageData surfaceData;
		_surface.create(w, h, _params.format);
		_surface.makeMutable(&surfaceData);

		int stride = int(surfaceData.stride);
		int format = CairoUtils::toCairoFormat(surfaceData.format);
		unsigned char* pixels = (unsigned char*)surfaceData.pixelData;

		_cairoSurface = cairo_image_surface_create_for_data(
			pixels, cairo_format_t(format), w, h, stride);

		if (_cairoSurface == NULL)
			return;

		_cairoContext = cairo_create(_cairoSurface);
		if (_cairoContext == NULL)
			return;
	}

	// Setup the context.
	cairo_set_operator(_cairoContext, CAIRO_OPERATOR_CLEAR);
	cairo_rectangle(_cairoContext, 0, 0, w, h);
	cairo_fill(_cairoContext);

	cairo_set_operator(_cairoContext, cairo_operator_t(CairoUtils::toCairoOperator(_params.compOp)));
	cairo_set_line_width(_cairoContext, _params.strokeWidth);

	// Setup globals.
	_patternExtend = CAIRO_EXTEND_REPEAT;
	_patternFilter = CAIRO_FILTER_NEAREST;

	switch (style) {
		case kBenchStyleLinearPad      : _patternExtend = CAIRO_EXTEND_PAD     ; break;
		case kBenchStyleLinearRepeat   : _patternExtend = CAIRO_EXTEND_REPEAT  ; break;
		case kBenchStyleLinearReflect  : _patternExtend = CAIRO_EXTEND_REFLECT ; break;
		case kBenchStyleRadialPad      : _patternExtend = CAIRO_EXTEND_PAD     ; break;
		case kBenchStyleRadialRepeat   : _patternExtend = CAIRO_EXTEND_REPEAT  ; break;
		case kBenchStyleRadialReflect  : _patternExtend = CAIRO_EXTEND_REFLECT ; break;
		case kBenchStylePatternNN      : _patternFilter = CAIRO_FILTER_NEAREST ; break;
		case kBenchStylePatternBI      : _patternFilter = CAIRO_FILTER_BILINEAR; break;
	}
}

void SkiaModule::onAfterRun() {
	// Free the surface & the context.
	cairo_destroy(_cairoContext);
	cairo_surface_destroy(_cairoSurface);

	_cairoContext = NULL;
	_cairoSurface = NULL;

	// Free the sprites.
	for (uint32_t i = 0; i < kBenchNumSprites; i++) {
		cairo_surface_destroy(_cairoSprites[i]);
		_cairoSprites[i] = NULL;
	}
}

void SkiaModule::onDoRectAligned(bool stroke) {
	BLSizeI bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	int wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRectI rect(_rndCoord.nextRectI(bounds, wh, wh));
		setupStyle<BLRectI>(style, rect);

		if (stroke) {
			cairo_rectangle(_cairoContext, rect.x + 0.5, rect.y + 0.5, rect.w, rect.h);
			cairo_stroke(_cairoContext);
		}
		else {
			cairo_rectangle(_cairoContext, rect.x, rect.y, rect.w, rect.h);
			cairo_fill(_cairoContext);
		}
	}
}

void SkiaModule::onDoRectSmooth(bool stroke) {
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));

		setupStyle<BLRect>(style, rect);
		cairo_rectangle(_cairoContext, rect.x, rect.y, rect.w, rect.h);

		if (stroke)
			cairo_stroke(_cairoContext);
		else
			cairo_fill(_cairoContext);
	}
}

void SkiaModule::onDoRectRotated(bool stroke) {
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double cx = double(_params.screenW) * 0.5;
	double cy = double(_params.screenH) * 0.5;
	double wh = _params.shapeSize;
	double angle = 0.0;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));

		cairo_translate(_cairoContext, cx, cy);
		cairo_rotate(_cairoContext, angle);
		cairo_translate(_cairoContext, -cx, -cy);

		setupStyle<BLRect>(style, rect);
		cairo_rectangle(_cairoContext, rect.x, rect.y, rect.w, rect.h);

		if (stroke)
			cairo_stroke(_cairoContext);
		else
			cairo_fill(_cairoContext);

		cairo_identity_matrix(_cairoContext);
	}
}

void SkiaModule::onDoRoundSmooth(bool stroke) {
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));
		double radius = _rndExtra.nextDouble(4.0, 40.0);

		setupStyle<BLRect>(style, rect);
		CairoUtils::roundRect(_cairoContext, rect, radius);

		if (stroke)
			cairo_stroke(_cairoContext);
		else
			cairo_fill(_cairoContext);
	}
}

void SkiaModule::onDoRoundRotated(bool stroke) {
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double cx = double(_params.screenW) * 0.5;
	double cy = double(_params.screenH) * 0.5;
	double wh = _params.shapeSize;
	double angle = 0.0;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));
		double radius = _rndExtra.nextDouble(4.0, 40.0);

		cairo_translate(_cairoContext, cx, cy);
		cairo_rotate(_cairoContext, angle);
		cairo_translate(_cairoContext, -cx, -cy);

		setupStyle<BLRect>(style, rect);
		CairoUtils::roundRect(_cairoContext, rect, radius);

		if (stroke)
			cairo_stroke(_cairoContext);
		else
			cairo_fill(_cairoContext);

		cairo_identity_matrix(_cairoContext);
	}
}

void SkiaModule::onDoPolygon(uint32_t mode, uint32_t complexity) {
	BLSizeI bounds(_params.screenW - _params.shapeSize,
								 _params.screenH - _params.shapeSize);
	uint32_t style = _params.style;
	bool stroke = (mode == 2);

	double wh = double(_params.shapeSize);

	if (mode == 0) cairo_set_fill_rule(_cairoContext, CAIRO_FILL_RULE_WINDING);
	if (mode == 1) cairo_set_fill_rule(_cairoContext, CAIRO_FILL_RULE_EVEN_ODD);

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLPoint base(_rndCoord.nextPoint(bounds));

		double x = _rndCoord.nextDouble(base.x, base.x + wh);
		double y = _rndCoord.nextDouble(base.y, base.y + wh);

		cairo_move_to(_cairoContext, x, y);
		for (uint32_t p = 1; p < complexity; p++) {
			x = _rndCoord.nextDouble(base.x, base.x + wh);
			y = _rndCoord.nextDouble(base.y, base.y + wh);
			cairo_line_to(_cairoContext, x, y);
		}
		setupStyle<BLRect>(style, BLRect(base.x, base.y, wh, wh));

		if (stroke)
			cairo_stroke(_cairoContext);
		else
			cairo_fill(_cairoContext);
	}
}

void SkiaModule::onDoShape(bool stroke, const BLPoint* pts, size_t count) {
	BLSizeI bounds(_params.screenW - _params.shapeSize,
								 _params.screenH - _params.shapeSize);
	uint32_t style = _params.style;

	// No idea who invented this, but you need a `cairo_t` to create a `cairo_path_t`.
	cairo_path_t* path = nullptr;

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
			cairo_move_to(_cairoContext, x * wh, y * wh);
			start = false;
		}
		else {
			cairo_line_to(_cairoContext, x * wh, y * wh);
		}
	}

	path = cairo_copy_path(_cairoContext);
	cairo_new_path(_cairoContext);

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		cairo_save(_cairoContext);

		BLPoint base(_rndCoord.nextPoint(bounds));
		setupStyle<BLRect>(style, BLRect(base.x, base.y, wh, wh));

		cairo_translate(_cairoContext, base.x, base.y);
		cairo_append_path(_cairoContext, path);

		if (stroke)
			cairo_stroke(_cairoContext);
		else
			cairo_fill(_cairoContext);

		cairo_restore(_cairoContext);
	}

	cairo_path_destroy(path);
}

} // {blbench}

#endif // BLBENCH_ENABLE_CAIRO
