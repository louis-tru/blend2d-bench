#if defined(BENCH_ENABLE_CAIRO)

#include "./app.h"
#include "./module_cairo.h"

namespace bench {

// ============================================================================
// [bench::CairoUtil]
// ============================================================================

static B2D_INLINE double u8ToUniform(int x) {
  static const double kDiv255 = 1.0 / 255.0;
  return double(x) * kDiv255;
}

uint32_t CairoUtil::toCairoFormat(uint32_t pixelFormat) {
  switch (pixelFormat) {
    case b2d::PixelFormat::kPRGB32: return CAIRO_FORMAT_ARGB32;
    case b2d::PixelFormat::kXRGB32: return CAIRO_FORMAT_RGB24;
    case b2d::PixelFormat::kA8    : return CAIRO_FORMAT_A8;
    default:
      return 0xFFFFFFFFu;
  }
}

uint32_t CairoUtil::toCairoOperator(uint32_t compOp) {
  switch (compOp) {
    case b2d::CompOp::kSrc        : return CAIRO_OPERATOR_SOURCE;
    case b2d::CompOp::kSrcOver    : return CAIRO_OPERATOR_OVER;
    case b2d::CompOp::kSrcIn      : return CAIRO_OPERATOR_IN;
    case b2d::CompOp::kSrcOut     : return CAIRO_OPERATOR_OUT;
    case b2d::CompOp::kSrcAtop    : return CAIRO_OPERATOR_ATOP;
    case b2d::CompOp::kDst        : return CAIRO_OPERATOR_DEST;
    case b2d::CompOp::kDstOver    : return CAIRO_OPERATOR_DEST_OVER;
    case b2d::CompOp::kDstIn      : return CAIRO_OPERATOR_DEST_IN;
    case b2d::CompOp::kDstOut     : return CAIRO_OPERATOR_DEST_OUT;
    case b2d::CompOp::kDstAtop    : return CAIRO_OPERATOR_DEST_ATOP;
    case b2d::CompOp::kXor        : return CAIRO_OPERATOR_XOR;
    case b2d::CompOp::kClear      : return CAIRO_OPERATOR_CLEAR;
    case b2d::CompOp::kPlus       : return CAIRO_OPERATOR_ADD;
    case b2d::CompOp::kPlusAlt    : return CAIRO_OPERATOR_ADD;
    case b2d::CompOp::kMultiply   : return CAIRO_OPERATOR_MULTIPLY;
    case b2d::CompOp::kScreen     : return CAIRO_OPERATOR_SCREEN;
    case b2d::CompOp::kOverlay    : return CAIRO_OPERATOR_OVERLAY;
    case b2d::CompOp::kDarken     : return CAIRO_OPERATOR_DARKEN;
    case b2d::CompOp::kLighten    : return CAIRO_OPERATOR_LIGHTEN;
    case b2d::CompOp::kColorDodge : return CAIRO_OPERATOR_COLOR_DODGE;
    case b2d::CompOp::kColorBurn  : return CAIRO_OPERATOR_COLOR_BURN;
    case b2d::CompOp::kHardLight  : return CAIRO_OPERATOR_HARD_LIGHT;
    case b2d::CompOp::kSoftLight  : return CAIRO_OPERATOR_SOFT_LIGHT;
    case b2d::CompOp::kDifference : return CAIRO_OPERATOR_DIFFERENCE;
    case b2d::CompOp::kExclusion  : return CAIRO_OPERATOR_EXCLUSION;

    default:
      return 0xFFFFFFFFu;
  }
}

void CairoUtil::round(cairo_t* ctx, const b2d::Rect& rect, double radius) {
  double rw2 = rect._w * 0.5;
  double rh2 = rect._h * 0.5;

  double rx = std::min<double>(std::abs(radius), rw2);
  double ry = std::min<double>(std::abs(radius), rh2);

  double rxKappaInv = rx * b2d::Math::k1MinusKappa;
  double ryKappaInv = ry * b2d::Math::k1MinusKappa;

  double x0 = rect._x;
  double y0 = rect._y;
  double x1 = rect._x + rect._w;
  double y1 = rect._y + rect._h;

  cairo_move_to(ctx, x0 + rx, y0);
  cairo_line_to(ctx, x1 - rx, y0);
  cairo_curve_to(ctx, x1 - rxKappaInv, y0, x1, y0 + ryKappaInv, x1, y0 + ry);

  cairo_line_to(ctx, x1, y1 - ry);
  cairo_curve_to(ctx, x1, y1 - ryKappaInv, x1 - rxKappaInv, y1, x1 - rx, y1);

  cairo_line_to(ctx, x0 + rx, y1);
  cairo_curve_to(ctx, x0 + rxKappaInv, y1, x0, y1 - ryKappaInv, x0, y1 - ry);

  cairo_line_to(ctx, x0, y0 + ry);
  cairo_curve_to(ctx, x0, y0 + ryKappaInv, x0 + rxKappaInv, y0, x0 + rx, y0);

  cairo_close_path(ctx);
}

// ============================================================================
// [bench::CairoModule - Construction / Destruction]
// ============================================================================

CairoModule::CairoModule() {
  ::strcpy(_name, "Cairo");
  _cairoSurface = NULL;
  _cairoContext = NULL;
  std::memset(_cairoSprites, 0, sizeof(_cairoSprites));
}
CairoModule::~CairoModule() {}

// ============================================================================
// [bench::CairoModule - Helpers]
// ============================================================================

template<typename RectT>
void CairoModule::setupStyle(uint32_t style, const RectT& rect) {
  switch (style) {
    case kBenchStyleSolid: {
      b2d::Argb32 c(_rndColor.nextArgb32());
      cairo_set_source_rgba(_cairoContext, u8ToUniform(c.r()), u8ToUniform(c.g()), u8ToUniform(c.b()), u8ToUniform(c.a()));
      return;
    }

    case kBenchStyleLinearPad:
    case kBenchStyleLinearRepeat:
    case kBenchStyleLinearReflect:
    case kBenchStyleRadialPad:
    case kBenchStyleRadialRepeat:
    case kBenchStyleRadialReflect: {
      double x = rect._x;
      double y = rect._y;

      b2d::Argb32 c0(_rndColor.nextArgb32());
      b2d::Argb32 c1(_rndColor.nextArgb32());
      b2d::Argb32 c2(_rndColor.nextArgb32());

      cairo_pattern_t* pattern = NULL;
      if (style < kBenchStyleRadialPad) {
        // Linear gradient.
        double x0 = rect._x + rect._w * 0.2;
        double y0 = rect._y + rect._h * 0.2;
        double x1 = rect._x + rect._w * 0.8;
        double y1 = rect._y + rect._h * 0.8;
        pattern = cairo_pattern_create_linear(x0, y0, x1, y1);

        cairo_pattern_add_color_stop_rgba(pattern, 0.0, u8ToUniform(c0.r()), u8ToUniform(c0.g()), u8ToUniform(c0.b()), u8ToUniform(c0.a()));
        cairo_pattern_add_color_stop_rgba(pattern, 0.5, u8ToUniform(c1.r()), u8ToUniform(c1.g()), u8ToUniform(c1.b()), u8ToUniform(c1.a()));
        cairo_pattern_add_color_stop_rgba(pattern, 1.0, u8ToUniform(c2.r()), u8ToUniform(c2.g()), u8ToUniform(c2.b()), u8ToUniform(c2.a()));
      }
      else {
        // Radial gradient.
        x += double(rect._w) / 2.0;
        y += double(rect._h) / 2.0;

        double r = double(rect._w + rect._h) / 4.0;
        pattern = cairo_pattern_create_radial(x, y, r, x - r / 2, y - r / 2, 0.0);

        // Color stops in Cairo's radial gradient are reverse to Blend/Qt.
        cairo_pattern_add_color_stop_rgba(pattern, 0.0, u8ToUniform(c2.r()), u8ToUniform(c2.g()), u8ToUniform(c2.b()), u8ToUniform(c2.a()));
        cairo_pattern_add_color_stop_rgba(pattern, 0.5, u8ToUniform(c1.r()), u8ToUniform(c1.g()), u8ToUniform(c1.b()), u8ToUniform(c1.a()));
        cairo_pattern_add_color_stop_rgba(pattern, 1.0, u8ToUniform(c0.r()), u8ToUniform(c0.g()), u8ToUniform(c0.b()), u8ToUniform(c0.a()));
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
      cairo_matrix_init_translate(&matrix, -rect._x, -rect._y);

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
// [bench::CairoModule - Interface]
// ============================================================================

bool CairoModule::supportsCompOp(uint32_t compOp) const {
  return CairoUtil::toCairoOperator(compOp) != 0xFFFFFFFFu;
}

bool CairoModule::supportsStyle(uint32_t style) const {
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

void CairoModule::onBeforeRun() {
  int w = int(_params.screenW);
  int h = int(_params.screenH);
  uint32_t style = _params.style;

  // Initialize the sprites.
  for (uint32_t i = 0; i < kBenchNumSprites; i++) {
    const b2d::Image& sprite = _sprites[i];

    int stride = int(sprite.impl()->stride());
    int format = CairoUtil::toCairoFormat(sprite.pixelFormat());
    unsigned char* pixels = static_cast<unsigned char*>(sprite.impl()->pixelData());

    cairo_surface_t* cairoSprite = cairo_image_surface_create_for_data(
      pixels, cairo_format_t(format), sprite.width(), sprite.height(), stride);

    _cairoSprites[i] = cairoSprite;
  }

  // Initialize the surface and the context.
  {
    _surface.create(w, h, _params.pixelFormat);
    _surface.lock(_lockedSurface, this);

    int stride = int(_lockedSurface.stride());
    int format = CairoUtil::toCairoFormat(_lockedSurface.pixelFormat());
    unsigned char* pixels = (unsigned char*)_lockedSurface.pixelData();

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

  cairo_set_operator(_cairoContext, cairo_operator_t(CairoUtil::toCairoOperator(_params.compOp)));
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

void CairoModule::onAfterRun() {
  // Free the surface & the context.
  cairo_destroy(_cairoContext);
  cairo_surface_destroy(_cairoSurface);

  _cairoContext = NULL;
  _cairoSurface = NULL;
  _surface.unlock(_lockedSurface, this);

  // Free the sprites.
  for (uint32_t i = 0; i < kBenchNumSprites; i++) {
    cairo_surface_destroy(_cairoSprites[i]);
    _cairoSprites[i] = NULL;
  }
}

void CairoModule::onDoRectAligned(bool stroke) {
  b2d::IntSize bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;

  int wh = _params.shapeSize;

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::IntRect rect(_rndCoord.nextIntRect(bounds, wh, wh));
    setupStyle<b2d::IntRect>(style, rect);

    if (stroke) {
      cairo_rectangle(_cairoContext, rect._x + 0.5, rect._y + 0.5, rect._w, rect._h);
      cairo_stroke(_cairoContext);
    }
    else {
      cairo_rectangle(_cairoContext, rect._x, rect._y, rect._w, rect._h);
      cairo_fill(_cairoContext);
    }
  }
}

void CairoModule::onDoRectSmooth(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;

  double wh = _params.shapeSize;

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));

    setupStyle<b2d::Rect>(style, rect);
    cairo_rectangle(_cairoContext, rect._x, rect._y, rect._w, rect._h);

    if (stroke)
      cairo_stroke(_cairoContext);
    else
      cairo_fill(_cairoContext);
  }
}

void CairoModule::onDoRectRotated(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;

  double cx = double(_params.screenW) * 0.5;
  double cy = double(_params.screenH) * 0.5;
  double wh = _params.shapeSize;
  double angle = 0.0;

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
    b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));

    cairo_translate(_cairoContext, cx, cy);
    cairo_rotate(_cairoContext, angle);
    cairo_translate(_cairoContext, -cx, -cy);

    setupStyle<b2d::Rect>(style, rect);
    cairo_rectangle(_cairoContext, rect._x, rect._y, rect._w, rect._h);

    if (stroke)
      cairo_stroke(_cairoContext);
    else
      cairo_fill(_cairoContext);

    cairo_identity_matrix(_cairoContext);
  }
}

void CairoModule::onDoRoundSmooth(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;

  double wh = _params.shapeSize;

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
    double radius = _rndExtra.nextDouble(4.0, 40.0);

    setupStyle<b2d::Rect>(style, rect);
    CairoUtil::round(_cairoContext, rect, radius);

    if (stroke)
      cairo_stroke(_cairoContext);
    else
      cairo_fill(_cairoContext);
  }
}

void CairoModule::onDoRoundRotated(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;

  double cx = double(_params.screenW) * 0.5;
  double cy = double(_params.screenH) * 0.5;
  double wh = _params.shapeSize;
  double angle = 0.0;

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
    b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
    double radius = _rndExtra.nextDouble(4.0, 40.0);

    cairo_translate(_cairoContext, cx, cy);
    cairo_rotate(_cairoContext, angle);
    cairo_translate(_cairoContext, -cx, -cy);

    setupStyle<b2d::Rect>(style, rect);
    CairoUtil::round(_cairoContext, rect, radius);

    if (stroke)
      cairo_stroke(_cairoContext);
    else
      cairo_fill(_cairoContext);

    cairo_identity_matrix(_cairoContext);
  }
}

void CairoModule::onDoPolygon(uint32_t mode, uint32_t complexity) {
  b2d::IntSize bounds(_params.screenW - _params.shapeSize,
                      _params.screenH - _params.shapeSize);
  uint32_t style = _params.style;
  bool stroke = (mode == 2);

  double wh = double(_params.shapeSize);

  if (mode == 0) cairo_set_fill_rule(_cairoContext, CAIRO_FILL_RULE_WINDING);
  if (mode == 1) cairo_set_fill_rule(_cairoContext, CAIRO_FILL_RULE_EVEN_ODD);

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::Point base(_rndCoord.nextPoint(bounds));

    double x = _rndCoord.nextDouble(base._x, base._x + wh);
    double y = _rndCoord.nextDouble(base._y, base._y + wh);

    cairo_move_to(_cairoContext, x, y);
    for (uint32_t p = 1; p < complexity; p++) {
      x = _rndCoord.nextDouble(base._x, base._x + wh);
      y = _rndCoord.nextDouble(base._y, base._y + wh);
      cairo_line_to(_cairoContext, x, y);
    }
    setupStyle<b2d::Rect>(style, b2d::Rect(base._x, base._y, wh, wh));

    if (stroke)
      cairo_stroke(_cairoContext);
    else
      cairo_fill(_cairoContext);
  }
}

void CairoModule::onDoShape(bool stroke, const b2d::Point* pts, size_t count) {
  b2d::IntSize bounds(_params.screenW - _params.shapeSize,
                      _params.screenH - _params.shapeSize);
  uint32_t style = _params.style;

  // No idea who invented this, but you need a `cairo_t` to create a `cairo_path_t`.
  cairo_path_t* path = nullptr;

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

    b2d::Point base(_rndCoord.nextPoint(bounds));
    setupStyle<b2d::Rect>(style, b2d::Rect(base._x, base._y, wh, wh));

    cairo_translate(_cairoContext, base._x, base._y);
    cairo_append_path(_cairoContext, path);

    if (stroke)
      cairo_stroke(_cairoContext);
    else
      cairo_fill(_cairoContext);

    cairo_restore(_cairoContext);
  }

  cairo_path_destroy(path);
}

} // bench namespace

#endif // BENCH_ENABLE_CAIRO
