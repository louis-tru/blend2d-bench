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

// #ifdef BLBENCH_ENABLE_SKIA

#include "./app.h"
#include "./module_skia.h"
#include <skia/core/SkBlendMode.h>
#include <skia/core/SkImage.h>
#include <skia/effects/SkGradientShader.h>

#include <algorithm>

namespace blbench {

#define DEBUG 0

#if DEBUG
#define PRINTF(s, ...) printf(s, ##__VA_ARGS__)
#else
#define PRINTF(s, ...)
#endif

static SkColorType toSkiaFormat(uint32_t format, SkAlphaType* at) {
	switch (format) {
		case BL_FORMAT_PRGB32:
			*at = kPremul_SkAlphaType;
			//printf("SkColorType::kRGBA_8888_SkColorType\n");
			return SkColorType::kRGBA_8888_SkColorType;
		case BL_FORMAT_XRGB32:
			*at = kUnpremul_SkAlphaType;
			//printf("SkColorType::kRGB_888x_SkColorType\n");
			return SkColorType::kRGB_888x_SkColorType;
		default:
			return SkColorType::kUnknown_SkColorType;
	}
}

static SkBlendMode toSkiaOperator(BLCompOp compOp) {
	switch (compOp) {
		case BL_COMP_OP_SRC_OVER: return SkBlendMode::kSrcOver;
		case BL_COMP_OP_SRC_COPY: return SkBlendMode::kSrc;
		case BL_COMP_OP_DST_IN: return SkBlendMode::kDstIn;
		case BL_COMP_OP_DST_OUT: return SkBlendMode::kDstOut;
		default:
			return SkBlendMode::kSrc;
	}
}

// ============================================================================
// [bench::SkiaModule - Construction / Destruction]
// ============================================================================

SkiaModule::SkiaModule() {
	strcpy(_name, "Skia");
	_SkiaContext = nullptr;
	memset(_SkiaSprites, 0, sizeof(_SkiaSprites));
}
SkiaModule::~SkiaModule() {}

// ============================================================================
// [bench::SkiaModule - Helpers]
// ============================================================================

template<typename RectT>
bool SkiaModule::setupStyle(uint32_t style, const RectT& rect, bool stroke, double radius) {
	_Paint.setStyle(stroke ? SkPaint::kStroke_Style: SkPaint::kFill_Style);

	if ( style == kBenchStylePatternNN || style == kBenchStylePatternBI) {
		// _SkiaContext->save();

		SkImage* sp = _SkiaSprites[nextSpriteId()];

		// printf("kBenchStylePatternNN, %d, %d\n", sp->width(), sp->height());

		SkFilterMode mode = style == kBenchStylePatternNN ? SkFilterMode::kNearest: SkFilterMode::kLinear;

		_SkiaContext->drawImage(sp, rect.x, rect.y, SkSamplingOptions(mode, SkMipmapMode::kNone), &_Paint);

		// _SkiaContext->restore();
		return false;
	}

	_Paint.setShader(nullptr);
	
	if (style == kBenchStyleSolid) {
		BLRgba32 c(_rndColor.nextRgba32());
		_Paint.setColor(c.value);
		return true;
	}

	PRINTF("kBenchStyleLinearPad\n");

	SkTileMode mode = SkTileMode::kClamp;
	switch(style) {
		default: mode = SkTileMode::kClamp; break;
		case kBenchStyleLinearRepeat:
		case kBenchStyleRadialRepeat: mode = SkTileMode::kRepeat; break;
		case kBenchStyleLinearReflect:
		case kBenchStyleRadialReflect: mode = SkTileMode::kMirror; break;
	}

	switch (style) {
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

			SkScalar pos[3] = { 0.0, 0.5, 1.0 };

			sk_sp<SkShader> shader;
			if (style < kBenchStyleRadialPad) {
				// Linear gradient.
				double x0 = rect.x + rect.w * 0.2;
				double y0 = rect.y + rect.h * 0.2;
				double x1 = rect.x + rect.w * 0.8;
				double y1 = rect.y + rect.h * 0.8;

				SkPoint pts[2] = { SkPoint::Make(x0, y0), SkPoint::Make(x1, y1) };
				SkColor colors[3] = { c0.value, c1.value, c2.value };

				shader = SkGradientShader::MakeLinear(pts, colors, pos, 3, mode, 0, nullptr);
			}
			else {
				// Radial gradient.
				x += double(rect.w) / 2.0;
				y += double(rect.h) / 2.0;

				double r = double(rect.w + rect.h) / 4.0;
				SkColor colors[3] = { c2.value, c1.value, c0.value };
		
				shader = SkGradientShader::MakeRadial(SkPoint::Make(x, y), r, colors, pos, 3, mode, 0, nullptr);
			}

			_Paint.setShader(shader);
			break;
		}

		case kBenchStyleConical: {
			double cx = rect.x + rect.w / 2;
			double cy = rect.y + rect.h / 2;

			BLRgba32 c(_rndColor.nextRgba32());

			SkScalar pos[4] = { 0.0, 0.33, 0.66, 1.0 };
			SkColor colors[4] = { c.value, _rndColor.nextRgba32().value, _rndColor.nextRgba32().value, c.value };
			sk_sp<SkShader> shader = SkGradientShader::MakeSweep(cx, cy, colors, pos, 4, mode, 0, 360, 0, nullptr);

			_Paint.setShader(shader);

			break;
		}
	}

	return true;
}

// ============================================================================
// [bench::SkiaModule - Interface]
// ============================================================================

bool SkiaModule::supportsCompOp(uint32_t compOp) const {
	return compOp == BL_COMP_OP_SRC_COPY ||
				compOp == BL_COMP_OP_SRC_OVER ||
				compOp == BL_COMP_OP_DST_IN ||
				compOp == BL_COMP_OP_DST_OUT;
}

bool SkiaModule::supportsStyle(uint32_t style) const {
	return style == kBenchStyleSolid          ||
					style == kBenchStyleLinearPad      ||
					style == kBenchStyleLinearRepeat   ||
					style == kBenchStyleLinearReflect  ||
					style == kBenchStyleRadialPad      ||
					style == kBenchStyleRadialRepeat   ||
					style == kBenchStyleRadialReflect  ||
					style == kBenchStyleConical        ||
					style == kBenchStylePatternNN      ||
					style == kBenchStylePatternBI      ;
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
		SkAlphaType alpha;
		SkColorType format = toSkiaFormat(spriteData.format, &alpha);

		unsigned char* pixels = static_cast<unsigned char*>(spriteData.pixelData);

		SkBitmap skiaSprite;
		skiaSprite.installPixels(SkImageInfo::Make(spriteData.size.w, spriteData.size.h, format, alpha), pixels, stride);

		sk_sp<SkImage> sksp = SkImage::MakeFromBitmap(skiaSprite);

		_SkiaSprites[i] = SkSafeRef(sksp.get());

		// printf("kBenchStylePatternNN, %d, %d, %d\n", _SkiaSprites[i]->width(), _SkiaSprites[i]->height(), stride);

	}

	// Initialize the surface and the context.
	{
		BLImageData surfaceData;
		_surface.create(w, h, _params.format);
		_surface.makeMutable(&surfaceData);

		int stride = int(surfaceData.stride);
		SkAlphaType alpha;
		SkColorType format = toSkiaFormat(surfaceData.format, &alpha);
		unsigned char* pixels = (unsigned char*)surfaceData.pixelData;

		// PRINTF("onBeforeRun, w: %d, h: %d \n", w, h);

		if (!_SkiaSurface.installPixels(SkImageInfo::Make(w, h, format, alpha), pixels, stride))
			return;

		_SkiaContext = new SkCanvas(_SkiaSurface);
	}

	// Setup the context.
	_SkiaContext->clear(0x00000000);

	_Paint = SkPaint();
	_Paint.setAntiAlias(true);
	_Paint.setBlendMode(toSkiaOperator(_params.compOp));
	_Paint.setStrokeWidth(_params.strokeWidth);
}

void SkiaModule::onAfterRun() {
	// Free the surface & the context.
	delete _SkiaContext;
	_SkiaContext = nullptr;
	// _Paint = SkPaint();
	for (uint32_t i = 0; i < kBenchNumSprites; i++) {
		SkSafeUnref(_SkiaSprites[i]);
		_SkiaSprites[i] = nullptr;
	}
}

void SkiaModule::onDoRectAligned(bool stroke) {//printf("%s\n", "onDoRectAligned");
	BLSizeI bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	int wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRectI rect(_rndCoord.nextRectI(bounds, wh, wh));
		if (setupStyle<BLRectI>(style, rect, stroke)) {
			PRINTF("\ronDoRectAligned x:%lf, y:%lf, w:%lf, h:%lf", rect.x, rect.y, rect.w, rect.h);
			if (stroke) {
				_SkiaContext->drawRect(SkRect::MakeXYWH(rect.x + 0.5, rect.y + 0.5, rect.w, rect.h), _Paint);
			} else {
				_SkiaContext->drawRect(SkRect::MakeXYWH(rect.x, rect.y, rect.w, rect.h), _Paint);
			}
		}
	}

	PRINTF("\n");
}

void SkiaModule::onDoRectSmooth(bool stroke) {//printf("%s\n", "onDoRectSmooth");
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));

		if (setupStyle<BLRect>(style, rect, stroke)) {
			PRINTF("\ronDoRectSmooth x:%lf, y:%lf, w:%lf, h:%lf", rect.x, rect.y, rect.w, rect.h);
			_SkiaContext->drawRect(SkRect::MakeXYWH(rect.x, rect.y, rect.w, rect.h), _Paint);
		}
	}

	PRINTF("\n");
}

void SkiaModule::onDoRectRotated(bool stroke) {//printf("%s\n", "onDoRectRotated");
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double cx = double(_params.screenW) * 0.5;
	double cy = double(_params.screenH) * 0.5;
	double wh = _params.shapeSize;
	double angle = 0.0;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));

		_SkiaContext->translate(cx, cy);
		_SkiaContext->rotate(angle);
		_SkiaContext->translate(-cx, -cy);

		if (setupStyle<BLRect>(style, rect, stroke)) {
			PRINTF("\ronDoRectRotated x:%lf, y:%lf, w:%lf, h:%lf", rect.x, rect.y, rect.w, rect.h);
			// PRINTF("onDoRoundRotated#setupStyle\n");
			_SkiaContext->drawRect(SkRect::MakeXYWH(rect.x, rect.y, rect.w, rect.h), _Paint);
		}
		_SkiaContext->resetMatrix();
	}

	PRINTF("\n");
}

void SkiaModule::onDoRoundSmooth(bool stroke) {//printf("%s\n", "onDoRoundSmooth");
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double wh = _params.shapeSize;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));
		double radius = _rndExtra.nextDouble(4.0, 40.0);

		if (setupStyle<BLRect>(style, rect, stroke, radius)) {
			PRINTF("\ronDoRoundSmooth x:%lf, y:%lf, w:%lf, h:%lf", rect.x, rect.y, rect.w, rect.h);
			SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeXYWH(rect.x, rect.y, rect.w, rect.h), radius, radius);
			_SkiaContext->drawRRect(rrect, _Paint);
		}
	}

	PRINTF("\n");
}

void SkiaModule::onDoRoundRotated(bool stroke) {//printf("%s\n", "onDoRoundRotated");
	BLSize bounds(_params.screenW, _params.screenH);
	uint32_t style = _params.style;

	double cx = double(_params.screenW) * 0.5;
	double cy = double(_params.screenH) * 0.5;
	double wh = _params.shapeSize;
	double angle = 0.0;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
		BLRect rect(_rndCoord.nextRect(bounds, wh, wh));
		double radius = _rndExtra.nextDouble(4.0, 40.0);

		_SkiaContext->translate(cx, cy);
		_SkiaContext->rotate(angle);
		_SkiaContext->translate(-cx, -cy);

		if (setupStyle<BLRect>(style, rect, stroke, radius)) {
			PRINTF("\ronDoRoundRotated x:%lf, y:%lf, w:%lf, h:%lf", rect.x, rect.y, rect.w, rect.h);
			SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeXYWH(rect.x, rect.y, rect.w, rect.h), radius, radius);
			_SkiaContext->drawRRect(rrect, _Paint);
		}

		_SkiaContext->resetMatrix();
	}

	PRINTF("\n");
}

void SkiaModule::onDoPolygon(uint32_t mode, uint32_t complexity) {//printf("%s\n", "onDoPolygon");
	BLSizeI bounds(_params.screenW - _params.shapeSize,
								 _params.screenH - _params.shapeSize);
	uint32_t style = _params.style;
	bool stroke = (mode == 2);

	double wh = double(_params.shapeSize);

	SkPathFillType fillRule = (mode != 0) ? SkPathFillType::kEvenOdd : SkPathFillType::kWinding;

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		BLPoint base(_rndCoord.nextPoint(bounds));

		double x = _rndCoord.nextDouble(base.x, base.x + wh);
		double y = _rndCoord.nextDouble(base.y, base.y + wh);

		SkPath path;
		path.setFillType(fillRule);
		path.moveTo(x, y);
		for (uint32_t p = 1; p < complexity; p++) {
			x = _rndCoord.nextDouble(base.x, base.x + wh);
			y = _rndCoord.nextDouble(base.y, base.y + wh);
			path.lineTo(x, y);
		}
		path.close();

		BLRect rect(base.x, base.y, wh, wh);

		if (setupStyle<BLRect>(style, rect, stroke)) {
			PRINTF("\ronDoPolygon x:%lf, y:%lf, w:%lf, h:%lf i: %d, quantity: %d", rect.x, rect.y, rect.w, rect.h, i, quantity);
			_SkiaContext->drawPath(path, _Paint);
		}
	}

	PRINTF("\n");
}

void SkiaModule::onDoShape(bool stroke, const BLPoint* pts, size_t count) {//printf("%s\n", "onDoShape");
	BLSizeI bounds(_params.screenW - _params.shapeSize,
								 _params.screenH - _params.shapeSize);
	uint32_t style = _params.style;

	// No idea who invented this, but you need a `cairo_t` to create a `cairo_path_t`.
	SkPath path;

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
			path.moveTo(x * wh, y * wh);
			start = false;
		}
		else {
			path.lineTo(x * wh, y * wh);
		}
	}

	for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
		_SkiaContext->save();

		BLPoint base(_rndCoord.nextPoint(bounds));
		_SkiaContext->translate(base.x, base.y);

		BLRect rect(base.x, base.y, wh, wh);

		if (setupStyle<BLRect>(style, rect, stroke)) {
			PRINTF("\ronDoShape x:%lf, y:%lf, w:%lf, h:%lf", rect.x, rect.y, rect.w, rect.h);
			_SkiaContext->drawPath(path, _Paint);
		}
		_SkiaContext->restore();
	}

	PRINTF("\n");
}

} // {blbench}

// #endif // BLBENCH_ENABLE_SKIA
