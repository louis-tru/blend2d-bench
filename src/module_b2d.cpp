#include "./app.h"
#include "./module_b2d.h"

namespace bench {

// ============================================================================
// [bench::BlendModule - Construction / Destruction]
// ============================================================================

BlendModule::BlendModule(uint32_t optLevel) {
  _optLevel = optLevel;

  const char* name = "Blend2D";
  switch (optLevel) {
    case b2d::Runtime::kOptLevel_X86_SSE2  : name = "Blend2D [SSE2]"  ; break;
    case b2d::Runtime::kOptLevel_X86_SSE3  : name = "Blend2D [SSE3]"  ; break;
    case b2d::Runtime::kOptLevel_X86_SSSE3 : name = "Blend2D [SSSE3]" ; break;
    case b2d::Runtime::kOptLevel_X86_SSE4_1: name = "Blend2D [SSE4.1]"; break;
    case b2d::Runtime::kOptLevel_X86_SSE4_2: name = "Blend2D [SSE4.2]"; break;
    case b2d::Runtime::kOptLevel_X86_AVX   : name = "Blend2D [AVX]"   ; break;
    case b2d::Runtime::kOptLevel_X86_AVX2  : name = "Blend2D [AVX2]"  ; break;
  }
  ::strcpy(_name, name);
}
BlendModule::~BlendModule() {}

// ============================================================================
// [bench::BlendModule - Helpers]
// ============================================================================

template<typename RectT>
static void BlendUtil_setupGradient(BlendModule* self, b2d::Gradient& gradient, uint32_t style, const RectT& rect) {
  switch (style) {
    case kBenchStyleLinearPad:
    case kBenchStyleLinearRepeat:
    case kBenchStyleLinearReflect: {
      double x0 = rect._x + rect._w * 0.2;
      double y0 = rect._y + rect._h * 0.2;
      double x1 = rect._x + rect._w * 0.8;
      double y1 = rect._y + rect._h * 0.8;

      gradient.setScalars(x0, y0, x1, y1);
      gradient.resetStops();

      gradient.addStop(0.0, self->_rndColor.nextArgb32());
      gradient.addStop(0.5, self->_rndColor.nextArgb32());
      gradient.addStop(1.0, self->_rndColor.nextArgb32());
      break;
    }

    case kBenchStyleRadialPad:
    case kBenchStyleRadialRepeat:
    case kBenchStyleRadialReflect: {
      double cx = rect._x + (rect._w / 2);
      double cy = rect._y + (rect._h / 2);
      double cr = (rect._w + rect._h) / 4;

      gradient.setScalars(cx, cy, cx - cr / 2, cy - cr / 2, cr);
      gradient.resetStops();

      gradient.addStop(0.0, self->_rndColor.nextArgb32());
      gradient.addStop(0.5, self->_rndColor.nextArgb32());
      gradient.addStop(1.0, self->_rndColor.nextArgb32());
      break;
    }

    default: {
      double cx = rect._x + (rect._w / 2);
      double cy = rect._y + (rect._h / 2);

      gradient.setScalars(cx, cy, 0.0, 0.0);
      gradient.resetStops();

      b2d::Argb32 c(self->_rndColor.nextArgb32());

      gradient.addStop(0.00, c);
      gradient.addStop(0.33, self->_rndColor.nextArgb32());
      gradient.addStop(0.66, self->_rndColor.nextArgb32());
      gradient.addStop(1.00, c);
      break;
    }
  }
}

// ============================================================================
// [bench::BlendModule - Interface]
// ============================================================================

bool BlendModule::supportsCompOp(uint32_t compOp) const {
  return true;
}

bool BlendModule::supportsStyle(uint32_t style) const {
  return style == kBenchStyleSolid         ||
         style == kBenchStyleLinearPad     ||
         style == kBenchStyleLinearRepeat  ||
         style == kBenchStyleLinearReflect ||
         style == kBenchStyleRadialPad     ||
         style == kBenchStyleRadialRepeat  ||
         style == kBenchStyleRadialReflect ||
         style == kBenchStyleConical       ||
         style == kBenchStylePatternNN     ||
         style == kBenchStylePatternBI     ;
}

void BlendModule::onBeforeRun() {
  int w = int(_params.screenW);
  int h = int(_params.screenH);
  uint32_t style = _params.style;

  b2d::Context2D::InitParams initParams;
  if (_optLevel != b2d::Runtime::kOptLevel_None) {
    initParams.setFlags(b2d::Context2D::kInitFlagIsolated);
    initParams.setOptLevel(_optLevel);
  }

  _surface.create(w, h, _params.pixelFormat);
  _context.begin(_surface, initParams);

  _context.setCompOp(b2d::CompOp::kSrc);
  _context.setFillStyle(b2d::Argb32(0x00000000));
  _context.fillAll();

  _context.setCompOp(_params.compOp);
  _context.setStrokeWidth(_params.strokeWidth);

  _context.setPatternFilter(
    _params.style == kBenchStylePatternNN
      ? b2d::Pattern::kFilterNearest
      : b2d::Pattern::kFilterBilinear);

  // Setup globals.
  _gradientType = b2d::Gradient::kTypeLinear;
  _gradientExtend = b2d::Gradient::kExtendPad;

  switch (style) {
    case kBenchStyleLinearPad      : _gradientType = b2d::Gradient::kTypeLinear ; _gradientExtend = b2d::Gradient::kExtendPad    ; break;
    case kBenchStyleLinearRepeat   : _gradientType = b2d::Gradient::kTypeLinear ; _gradientExtend = b2d::Gradient::kExtendRepeat ; break;
    case kBenchStyleLinearReflect  : _gradientType = b2d::Gradient::kTypeLinear ; _gradientExtend = b2d::Gradient::kExtendReflect; break;
    case kBenchStyleRadialPad      : _gradientType = b2d::Gradient::kTypeRadial ; _gradientExtend = b2d::Gradient::kExtendPad    ; break;
    case kBenchStyleRadialRepeat   : _gradientType = b2d::Gradient::kTypeRadial ; _gradientExtend = b2d::Gradient::kExtendRepeat ; break;
    case kBenchStyleRadialReflect  : _gradientType = b2d::Gradient::kTypeRadial ; _gradientExtend = b2d::Gradient::kExtendReflect; break;
    case kBenchStyleConical        : _gradientType = b2d::Gradient::kTypeConical; break;
  }
}

void BlendModule::onAfterRun() {
  _context.end();
}

void BlendModule::onDoRectAligned(bool stroke) {
  b2d::IntSize bounds(_params.screenW, _params.screenH);

  uint32_t style = _params.style;
  uint32_t styleSlot = stroke ? b2d::Context2D::kStyleSlotStroke : b2d::Context2D::kStyleSlotFill;

  int wh = _params.shapeSize;

  switch (style) {
    case kBenchStyleSolid: {
      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        b2d::IntRect rect(_rndCoord.nextIntRect(bounds, wh, wh));
        b2d::Argb32 color(_rndColor.nextArgb32());

        _context.setStyle(styleSlot, color);
        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRect(b2d::Rect(rect._x + 0.5, rect._y + 0.5, rect._w, rect._h));
        else
          _context.fillRect(rect);
      }
      break;
    }

    case kBenchStyleLinearPad:
    case kBenchStyleLinearRepeat:
    case kBenchStyleLinearReflect:
    case kBenchStyleRadialPad:
    case kBenchStyleRadialRepeat:
    case kBenchStyleRadialReflect:
    case kBenchStyleConical: {
      b2d::Gradient gradient(b2d::Gradient::typeIdFromGradientType(_gradientType));
      gradient.setExtend(_gradientExtend);

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        b2d::IntRect rect(_rndCoord.nextIntRect(bounds, wh, wh));
        BlendUtil_setupGradient<b2d::IntRect>(this, gradient, style, rect);

        _context.save();
        _context.setStyle(styleSlot, gradient);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRect(b2d::Rect(rect._x + 0.5, rect._y + 0.5, rect._w, rect._h));
        else
          _context.fillRect(rect);

        _context.restore();
      }
      break;
    }

    case kBenchStylePatternNN:
    case kBenchStylePatternBI: {
      b2d::Pattern pattern;

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        b2d::IntRect rect(_rndCoord.nextIntRect(bounds, wh, wh));

        if (styleSlot == b2d::Context2D::kStyleSlotStroke) {
          pattern.setPattern(_sprites[nextSpriteId()]);
          pattern.setMatrix(b2d::Matrix2D::translation(rect._x + 0.5, rect._y + 0.5));

          _context.save();
          _context.setStrokeStyle(pattern);
          _context.strokeRect(b2d::Rect(rect._x + 0.5, rect._y + 0.5, rect._w, rect._h));
          _context.restore();
        }
        else {
          _context.blitImage(b2d::IntPoint(rect._x, rect._y), _sprites[nextSpriteId()]);
        }
      }
      break;
    }
  }
}

void BlendModule::onDoRectSmooth(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);

  uint32_t style = _params.style;
  uint32_t styleSlot = stroke ? b2d::Context2D::kStyleSlotStroke : b2d::Context2D::kStyleSlotFill;

  double wh = _params.shapeSize;

  switch (style) {
    case kBenchStyleSolid: {
      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        b2d::Argb32 color(_rndColor.nextArgb32());

        _context.setStyle(styleSlot, color);
        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRect(rect);
        else
          _context.fillRect(rect);
      }
      break;
    }

    case kBenchStyleLinearPad:
    case kBenchStyleLinearRepeat:
    case kBenchStyleLinearReflect:
    case kBenchStyleRadialPad:
    case kBenchStyleRadialRepeat:
    case kBenchStyleRadialReflect:
    case kBenchStyleConical: {
      b2d::Gradient gradient(b2d::Gradient::typeIdFromGradientType(_gradientType));
      gradient.setExtend(_gradientExtend);

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        BlendUtil_setupGradient<b2d::Rect>(this, gradient, style, rect);

        _context.save();
        _context.setStyle(styleSlot, gradient);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRect(rect);
        else
          _context.fillRect(rect);

        _context.restore();
      }
      break;
    }

    case kBenchStylePatternNN:
    case kBenchStylePatternBI: {
      b2d::Pattern pattern;

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));

        if (styleSlot == b2d::Context2D::kStyleSlotStroke) {
          pattern.setPattern(_sprites[nextSpriteId()]);
          pattern.setMatrix(b2d::Matrix2D::translation(rect._x + 0.5, rect._y + 0.5));

          _context.save();
          _context.setStrokeStyle(pattern);
          _context.strokeRect(b2d::Rect(rect._x + 0.5, rect._y + 0.5, rect._w, rect._h));
          _context.restore();
        }
        else {
          _context.blitImage(b2d::Point(rect._x, rect._y), _sprites[nextSpriteId()]);
        }
      }
      break;
    }
  }
}

void BlendModule::onDoRectRotated(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);

  uint32_t style = _params.style;
  uint32_t styleSlot = stroke ? b2d::Context2D::kStyleSlotStroke : b2d::Context2D::kStyleSlotFill;

  double cx = double(_params.screenW) * 0.5;
  double cy = double(_params.screenH) * 0.5;
  double wh = _params.shapeSize;
  double angle = 0.0;

  switch (style) {
    case kBenchStyleSolid: {
      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        b2d::Argb32 color(_rndColor.nextArgb32());

        _context.save();
        _context.rotate(angle, b2d::Point(cx, cy));
        _context.setStyle(styleSlot, color);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRect(rect);
        else
          _context.fillRect(rect);

        _context.restore();
      }
      break;
    }

    case kBenchStyleLinearPad:
    case kBenchStyleLinearRepeat:
    case kBenchStyleLinearReflect:
    case kBenchStyleRadialPad:
    case kBenchStyleRadialRepeat:
    case kBenchStyleRadialReflect:
    case kBenchStyleConical: {
      b2d::Gradient gradient(b2d::Gradient::typeIdFromGradientType(_gradientType));
      gradient.setExtend(_gradientExtend);

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        BlendUtil_setupGradient<b2d::Rect>(this, gradient, style, rect);

        _context.save();
        _context.rotate(angle, b2d::Point(cx, cy));
        _context.setStyle(styleSlot, gradient);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRect(rect);
        else
          _context.fillRect(rect);

        _context.restore();
      }
      break;
    }

    case kBenchStylePatternNN:
    case kBenchStylePatternBI: {
      b2d::Pattern pattern;

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));

        _context.save();
        if (styleSlot == b2d::Context2D::kStyleSlotStroke) {
          pattern.setPattern(_sprites[nextSpriteId()]);
          pattern.setMatrix(b2d::Matrix2D::translation(rect._x + 0.5, rect._y + 0.5));

          _context.setStrokeStyle(pattern);
          _context.rotate(angle, b2d::Point(cx, cy));
          _context.strokeRect(b2d::Rect(rect._x + 0.5, rect._y + 0.5, rect._w, rect._h));
        }
        else {
          _context.rotate(angle, b2d::Point(cx, cy));
          _context.blitImage(b2d::Point(rect._x, rect._y), _sprites[nextSpriteId()]);
        }
        _context.restore();
      }
      break;
    }
  }
}

void BlendModule::onDoRoundSmooth(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);

  uint32_t style = _params.style;
  uint32_t styleSlot = stroke ? b2d::Context2D::kStyleSlotStroke : b2d::Context2D::kStyleSlotFill;

  double wh = _params.shapeSize;

  switch (style) {
    case kBenchStyleSolid: {
      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        double radius = _rndExtra.nextDouble(4.0, 40.0);

        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        b2d::Round round(rect, radius);
        b2d::Argb32 color(_rndColor.nextArgb32());

        _context.setStyle(styleSlot, color);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRound(round);
        else
          _context.fillRound(round);
      }
      break;
    }

    case kBenchStyleLinearPad:
    case kBenchStyleLinearRepeat:
    case kBenchStyleLinearReflect:
    case kBenchStyleRadialPad:
    case kBenchStyleRadialRepeat:
    case kBenchStyleRadialReflect:
    case kBenchStyleConical: {
      b2d::Gradient gradient(b2d::Gradient::typeIdFromGradientType(_gradientType));
      gradient.setExtend(_gradientExtend);

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        double radius = _rndExtra.nextDouble(4.0, 40.0);

        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        b2d::Round round(rect, radius);

        BlendUtil_setupGradient<b2d::Rect>(this, gradient, style, rect);
        _context.save();
        _context.setStyle(styleSlot, gradient);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRound(round);
        else
          _context.fillRound(round);

        _context.restore();
      }
      break;
    }

    case kBenchStylePatternNN:
    case kBenchStylePatternBI: {
      b2d::Pattern pattern;

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        double radius = _rndExtra.nextDouble(4.0, 40.0);

        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        b2d::Round round(rect, radius);

        pattern.setPattern(_sprites[nextSpriteId()]);
        pattern.setMatrix(b2d::Matrix2D::translation(rect._x, rect._y));

        _context.save();
        _context.setStyle(styleSlot, pattern);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRound(round);
        else
          _context.fillRound(round);

        _context.restore();
      }
      break;
    }
  }
}

void BlendModule::onDoRoundRotated(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);

  uint32_t style = _params.style;
  uint32_t styleSlot = stroke ? b2d::Context2D::kStyleSlotStroke : b2d::Context2D::kStyleSlotFill;

  double cx = double(_params.screenW) * 0.5;
  double cy = double(_params.screenH) * 0.5;
  double wh = _params.shapeSize;
  double angle = 0.0;

  switch (style) {
    case kBenchStyleSolid: {
      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
        double radius = _rndExtra.nextDouble(4.0, 40.0);

        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        b2d::Round round(rect, radius);
        b2d::Argb32 color(_rndColor.nextArgb32());

        _context.save();
        _context.rotate(angle, b2d::Point(cx, cy));
        _context.setStyle(styleSlot, color);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRound(round);
        else
          _context.fillRound(round);

        _context.restore();
      }
      break;
    }

    case kBenchStyleLinearPad:
    case kBenchStyleLinearRepeat:
    case kBenchStyleLinearReflect:
    case kBenchStyleRadialPad:
    case kBenchStyleRadialRepeat:
    case kBenchStyleRadialReflect:
    case kBenchStyleConical: {
      b2d::Gradient gradient(b2d::Gradient::typeIdFromGradientType(_gradientType));
      gradient.setExtend(_gradientExtend);

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
        double radius = _rndExtra.nextDouble(4.0, 40.0);

        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        b2d::Round round(rect, radius);

        BlendUtil_setupGradient<b2d::Rect>(this, gradient, style, rect);

        _context.save();
        _context.rotate(angle, b2d::Point(cx, cy));
        _context.setStyle(styleSlot, gradient);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRound(round);
        else
          _context.fillRound(round);

        _context.restore();
      }
      break;
    }

    case kBenchStylePatternNN:
    case kBenchStylePatternBI: {
      b2d::Pattern pattern;

      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
        double radius = _rndExtra.nextDouble(4.0, 40.0);

        b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
        b2d::Round round(rect, radius);

        pattern.setPattern(_sprites[nextSpriteId()]);
        pattern.setMatrix(b2d::Matrix2D::translation(rect._x, rect._y));

        _context.save();
        _context.rotate(angle, b2d::Point(cx, cy));
        _context.setStyle(styleSlot, pattern);

        if (styleSlot == b2d::Context2D::kStyleSlotStroke)
          _context.strokeRound(round);
        else
          _context.fillRound(round);

        _context.restore();
      }
      break;
    }
  }
}

void BlendModule::onDoPolygon(uint32_t mode, uint32_t complexity) {
  b2d::IntSize bounds(_params.screenW - _params.shapeSize,
                      _params.screenH - _params.shapeSize);
  uint32_t style = _params.style;
  uint32_t styleSlot = (mode == 2) ? b2d::Context2D::kStyleSlotStroke : b2d::Context2D::kStyleSlotFill;

  b2d::Wrap<b2d::Point> points[128];
  B2D_ASSERT(complexity < B2D_ARRAY_SIZE(points));

  b2d::Gradient gradient(b2d::Gradient::typeIdFromGradientType(_gradientType));
  b2d::Pattern pattern;
  gradient.setExtend(_gradientExtend);

  if (mode == 0) _context.setFillRule(b2d::FillRule::kNonZero);
  if (mode == 1) _context.setFillRule(b2d::FillRule::kEvenOdd);

  double wh = double(_params.shapeSize);

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::Point base(_rndCoord.nextPoint(bounds));

    for (uint32_t p = 0; p < complexity; p++) {
      double x = _rndCoord.nextDouble(base._x, base._x + wh);
      double y = _rndCoord.nextDouble(base._y, base._y + wh);
      points[p]->reset(x, y);
    }

    _context.save();

    switch (style) {
      case kBenchStyleSolid: {
        _context.setStyle(styleSlot, _rndColor.nextArgb32());
        break;
      }

      case kBenchStyleLinearPad:
      case kBenchStyleLinearRepeat:
      case kBenchStyleLinearReflect:
      case kBenchStyleRadialPad:
      case kBenchStyleRadialRepeat:
      case kBenchStyleRadialReflect:
      case kBenchStyleConical: {
        b2d::Rect rect(base._x, base._y, wh, wh);
        BlendUtil_setupGradient<b2d::Rect>(this, gradient, style, rect);
        _context.setStyle(styleSlot, gradient);
        break;
      }

      case kBenchStylePatternNN:
      case kBenchStylePatternBI: {
        pattern.setPattern(_sprites[nextSpriteId()]);
        pattern.setMatrix(b2d::Matrix2D::translation(base._x, base._y));
        _context.setStyle(styleSlot, pattern);
        break;
      }
    }

    if (styleSlot == b2d::Context2D::kStyleSlotStroke)
      _context.strokePolygon(&points[0], complexity);
    else
      _context.fillPolygon(&points[0], complexity);

    _context.restore();
  }
}

void BlendModule::onDoShape(bool stroke, const b2d::Point* pts, size_t count) {
  b2d::IntSize bounds(_params.screenW - _params.shapeSize,
                      _params.screenH - _params.shapeSize);
  uint32_t style = _params.style;
  uint32_t styleSlot = stroke ? b2d::Context2D::kStyleSlotStroke : b2d::Context2D::kStyleSlotFill;

  b2d::Path2D path;
  bool start = true;
  double wh = double(_params.shapeSize);

  for (size_t i = 0; i < count; i++) {
    double x = pts[i].x();
    double y = pts[i].y();

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

  b2d::Gradient gradient(b2d::Gradient::typeIdFromGradientType(_gradientType));
  b2d::Pattern pattern;
  gradient.setExtend(_gradientExtend);

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::Point base(_rndCoord.nextPoint(bounds));

    _context.save();

    switch (style) {
      case kBenchStyleSolid: {
        _context.setStyle(styleSlot, _rndColor.nextArgb32());
        break;
      }

      case kBenchStyleLinearPad:
      case kBenchStyleLinearRepeat:
      case kBenchStyleLinearReflect:
      case kBenchStyleRadialPad:
      case kBenchStyleRadialRepeat:
      case kBenchStyleRadialReflect:
      case kBenchStyleConical: {
        b2d::Rect rect(base._x, base._y, wh, wh);
        BlendUtil_setupGradient<b2d::Rect>(this, gradient, style, rect);
        _context.setStyle(styleSlot, gradient);
        break;
      }

      case kBenchStylePatternNN:
      case kBenchStylePatternBI: {
        pattern.setPattern(_sprites[nextSpriteId()]);
        pattern.setMatrix(b2d::Matrix2D::translation(base._x, base._y));
        _context.setStyle(styleSlot, pattern);
        break;
      }
    }

    _context.translate(base);
    if (styleSlot == b2d::Context2D::kStyleSlotStroke)
      _context.strokePath(path);
    else
      _context.fillPath(path);

    _context.restore();
  }
}

} // bench namespace
