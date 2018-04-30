#if defined(BENCH_ENABLE_QT)

#include "./app.h"
#include "./module_qt.h"

namespace bench {

// ============================================================================
// [bench::QtUtil]
// ============================================================================

uint32_t QtUtil::toQtFormat(uint32_t pixelFormat) {
  switch (pixelFormat) {
    case b2d::PixelFormat::kPRGB32: return QImage::Format_ARGB32_Premultiplied;
    case b2d::PixelFormat::kXRGB32: return QImage::Format_RGB32;

    default:
      return 0xFFFFFFFFu;
  }
}

uint32_t QtUtil::toQtOperator(uint32_t compOp) {
  switch (compOp) {
    case b2d::CompOp::kSrc        : return QPainter::CompositionMode_Source;
    case b2d::CompOp::kSrcOver    : return QPainter::CompositionMode_SourceOver;
    case b2d::CompOp::kSrcIn      : return QPainter::CompositionMode_SourceIn;
    case b2d::CompOp::kSrcOut     : return QPainter::CompositionMode_SourceOut;
    case b2d::CompOp::kSrcAtop    : return QPainter::CompositionMode_SourceAtop;
    case b2d::CompOp::kDst        : return QPainter::CompositionMode_Destination;
    case b2d::CompOp::kDstOver    : return QPainter::CompositionMode_DestinationOver;
    case b2d::CompOp::kDstIn      : return QPainter::CompositionMode_DestinationIn;
    case b2d::CompOp::kDstOut     : return QPainter::CompositionMode_DestinationOut;
    case b2d::CompOp::kDstAtop    : return QPainter::CompositionMode_DestinationAtop;
    case b2d::CompOp::kXor        : return QPainter::CompositionMode_Xor;
    case b2d::CompOp::kClear      : return QPainter::CompositionMode_Clear;
    case b2d::CompOp::kPlus       : return QPainter::CompositionMode_Plus;
    case b2d::CompOp::kPlusAlt    : return QPainter::CompositionMode_Plus;
    case b2d::CompOp::kMultiply   : return QPainter::CompositionMode_Multiply;
    case b2d::CompOp::kScreen     : return QPainter::CompositionMode_Screen;
    case b2d::CompOp::kOverlay    : return QPainter::CompositionMode_Overlay;
    case b2d::CompOp::kDarken     : return QPainter::CompositionMode_Darken;
    case b2d::CompOp::kLighten    : return QPainter::CompositionMode_Lighten;
    case b2d::CompOp::kColorDodge : return QPainter::CompositionMode_ColorDodge;
    case b2d::CompOp::kColorBurn  : return QPainter::CompositionMode_ColorBurn;
    case b2d::CompOp::kHardLight  : return QPainter::CompositionMode_HardLight;
    case b2d::CompOp::kSoftLight  : return QPainter::CompositionMode_SoftLight;
    case b2d::CompOp::kDifference : return QPainter::CompositionMode_Difference;
    case b2d::CompOp::kExclusion  : return QPainter::CompositionMode_Exclusion;

    default:
      return 0xFFFFFFFFu;
  }
}

template<typename RectT>
B2D_INLINE QBrush QtModule::setupStyle(uint32_t style, const RectT& rect) {
  switch (style) {
    case kBenchStyleLinearPad:
    case kBenchStyleLinearRepeat:
    case kBenchStyleLinearReflect: {
      double x0 = rect._x + rect._w * 0.2;
      double y0 = rect._y + rect._h * 0.2;
      double x1 = rect._x + rect._w * 0.8;
      double y1 = rect._y + rect._h * 0.8;

      QLinearGradient g((qreal)x0, (qreal)y0, (qreal)x1, (qreal)y1);
      g.setColorAt(qreal(0.0), QtUtil::toQColor(_rndColor.nextArgb32()));
      g.setColorAt(qreal(0.5), QtUtil::toQColor(_rndColor.nextArgb32()));
      g.setColorAt(qreal(1.0), QtUtil::toQColor(_rndColor.nextArgb32()));
      g.setSpread(static_cast<QGradient::Spread>(_gradientSpread));
      return QBrush(g);
    }

    case kBenchStyleRadialPad:
    case kBenchStyleRadialRepeat:
    case kBenchStyleRadialReflect: {
      double cx = rect._x + rect._w / 2;
      double cy = rect._y + rect._h / 2;
      double cr = (rect._w + rect._h) / 4;

      QRadialGradient g(qreal(cx), qreal(cy), qreal(cr), qreal(cx - cr / 2), qreal(cy - cr / 2), qreal(0));
      g.setColorAt(qreal(0.0), QtUtil::toQColor(_rndColor.nextArgb32()));
      g.setColorAt(qreal(0.5), QtUtil::toQColor(_rndColor.nextArgb32()));
      g.setColorAt(qreal(1.0), QtUtil::toQColor(_rndColor.nextArgb32()));
      g.setSpread(static_cast<QGradient::Spread>(_gradientSpread));
      return QBrush(g);
    }

    case kBenchStyleConical: {
      double cx = rect._x + rect._w / 2;
      double cy = rect._y + rect._h / 2;
      QColor c(QtUtil::toQColor(_rndColor.nextArgb32()));

      QConicalGradient g(qreal(cx), qreal(cy), qreal(0));
      g.setColorAt(qreal(0.00), c);
      g.setColorAt(qreal(0.33), QtUtil::toQColor(_rndColor.nextArgb32()));
      g.setColorAt(qreal(0.66), QtUtil::toQColor(_rndColor.nextArgb32()));
      g.setColorAt(qreal(1.00), c);
      return QBrush(g);
    }

    case kBenchStylePatternNN:
    case kBenchStylePatternBI:
    default: {
      QBrush brush(*_qtSprites[nextSpriteId()]);

      // FIXME: It seems that Qt will never use subpixel filtering when drawing
      // an unscaled image. The test suite, however, expects that path to be
      // triggered. To fix this, we scale the image slightly (it should have no
      // visual impact) to prevent Qt using nearest-neighbor fast-path.
      qreal scale =  style == kBenchStylePatternNN ? 1.0 : 1.00001;

      brush.setMatrix(QMatrix(scale, qreal(0), qreal(0), scale, qreal(rect._x), qreal(rect._y)));
      return brush;
    }
  }
}

// ============================================================================
// [bench::QtModule - Construction / Destruction]
// ============================================================================

QtModule::QtModule() {
  ::strcpy(_name, "Qt");
  _qtSurface = NULL;
  _qtContext = NULL;
  std::memset(_qtSprites, 0, sizeof(_qtSprites));
}
QtModule::~QtModule() {}

// ============================================================================
// [bench::QtModule - Interface]
// ============================================================================

bool QtModule::supportsCompOp(uint32_t compOp) const {
  return QtUtil::toQtOperator(compOp) != 0xFFFFFFFFu;
}

bool QtModule::supportsStyle(uint32_t style) const {
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

void QtModule::onBeforeRun() {
  int w = int(_params.screenW);
  int h = int(_params.screenH);
  uint32_t style = _params.style;

  // Initialize the sprites.
  for (uint32_t i = 0; i < kBenchNumSprites; i++) {
    const b2d::Image& sprite = _sprites[i];
    QImage* qtSprite = new QImage(
      static_cast<unsigned char*>(sprite.impl()->pixelData()),
      sprite.width(),
      sprite.height(),
      int(sprite.impl()->stride()), QImage::Format_ARGB32_Premultiplied);

    _qtSprites[i] = qtSprite;
  }

  // Initialize the surface and the context.
  _surface.create(w, h, _params.pixelFormat);
  _surface.lock(_lockedSurface, this);

  int stride = int(_lockedSurface.stride());
  int qtFormat = QtUtil::toQtFormat(_lockedSurface.pixelFormat());

  _qtSurface = new QImage(
    (unsigned char*)_lockedSurface.pixelData(), w, h,
    stride, static_cast<QImage::Format>(qtFormat));
  if (_qtSurface == NULL)
    return;

  _qtContext = new QPainter(_qtSurface);
  if (_qtContext == NULL)
    return;

  // Setup the context.
  _qtContext->setCompositionMode(QPainter::CompositionMode_Source);
  _qtContext->fillRect(0, 0, w, h, QColor(0, 0, 0, 0));

  _qtContext->setCompositionMode(
    static_cast<QPainter::CompositionMode>(
      QtUtil::toQtOperator(_params.compOp)));

  _qtContext->setRenderHint(QPainter::Antialiasing, true);
  _qtContext->setRenderHint(QPainter::SmoothPixmapTransform, _params.style != kBenchStylePatternNN);

  // Setup globals.
  _gradientSpread = QGradient::PadSpread;

  switch (style) {
    case kBenchStyleLinearPad      : _gradientSpread = QGradient::PadSpread    ; break;
    case kBenchStyleLinearRepeat   : _gradientSpread = QGradient::RepeatSpread ; break;
    case kBenchStyleLinearReflect  : _gradientSpread = QGradient::ReflectSpread; break;
    case kBenchStyleRadialPad      : _gradientSpread = QGradient::PadSpread    ; break;
    case kBenchStyleRadialRepeat   : _gradientSpread = QGradient::RepeatSpread ; break;
    case kBenchStyleRadialReflect  : _gradientSpread = QGradient::ReflectSpread; break;
  }
}

void QtModule::onAfterRun() {
  // Free the surface & the context.
  delete _qtContext;
  delete _qtSurface;

  _qtContext = NULL;
  _qtSurface = NULL;
  _surface.unlock(_lockedSurface, this);

  // Free the sprites.
  for (uint32_t i = 0; i < kBenchNumSprites; i++) {
    delete _qtSprites[i];
    _qtSprites[i] = NULL;
  }
}

void QtModule::onDoRectAligned(bool stroke) {
  b2d::IntSize bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;
  int wh = _params.shapeSize;

  if (stroke)
    _qtContext->setBrush(Qt::NoBrush);

  if (style == kBenchStyleSolid) {
    for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
      b2d::IntRect rect(_rndCoord.nextIntRect(bounds, wh, wh));
      QColor color(QtUtil::toQColor(_rndColor.nextArgb32()));

      if (stroke) {
        _qtContext->setPen(color);
        _qtContext->drawRect(QRectF(rect._x + 0.5, rect._y + 0.5, rect._w, rect._h));
      }
      else {
        _qtContext->fillRect(QRect(rect._x, rect._y, rect._w, rect._h), color);
      }
    }
  }
  else {
    if ((style == kBenchStylePatternNN || style == kBenchStylePatternBI) && !stroke) {
      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        b2d::IntRect rect(_rndCoord.nextIntRect(bounds, wh, wh));
        const QImage& sprite = *_qtSprites[nextSpriteId()];

        _qtContext->drawImage(QPoint(rect._x, rect._y), sprite);
      }
    }
    else {
      for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
        b2d::IntRect rect(_rndCoord.nextIntRect(bounds, wh, wh));
        QBrush brush(setupStyle<b2d::IntRect>(style, rect));
        if (stroke) {
          _qtContext->setPen(QPen(brush, qreal(_params.strokeWidth)));
          _qtContext->drawRect(QRectF(rect._x + 0.5, rect._y + 0.5, rect._w, rect._h));
        }
        else {
          _qtContext->fillRect(QRect(rect._x, rect._y, rect._w, rect._h), brush);
        }
      }
    }
  }
}

void QtModule::onDoRectSmooth(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;
  double wh = _params.shapeSize;

  if (stroke)
    _qtContext->setBrush(Qt::NoBrush);

  if (style == kBenchStyleSolid) {
    for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
      b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
      QColor color(QtUtil::toQColor(_rndColor.nextArgb32()));

      if (stroke) {
        _qtContext->setPen(color);
        _qtContext->drawRect(QRectF(rect._x, rect._y, rect._w, rect._h));
      }
      else {
        _qtContext->fillRect(QRectF(rect._x, rect._y, rect._w, rect._h), color);
      }
    }
  }
  else {
    for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
      b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
      QBrush brush(setupStyle<b2d::Rect>(style, rect));

      if (stroke) {
        _qtContext->setPen(QPen(brush, qreal(_params.strokeWidth)));
        _qtContext->drawRect(QRectF(rect._x, rect._y, rect._w, rect._h));
      }
      else {
        _qtContext->fillRect(QRectF(rect._x, rect._y, rect._w, rect._h), brush);
      }
    }
  }
}

void QtModule::onDoRectRotated(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;

  double cx = double(_params.screenW) * 0.5;
  double cy = double(_params.screenH) * 0.5;
  double wh = _params.shapeSize;
  double angle = 0.0;

  if (stroke)
    _qtContext->setBrush(Qt::NoBrush);

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
    b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));

    QTransform transform;
    transform.translate(cx, cy);
    transform.rotateRadians(angle);
    transform.translate(-cx, -cy);
    _qtContext->setTransform(transform, false);

    if (style == kBenchStyleSolid) {
      QColor color(QtUtil::toQColor(_rndColor.nextArgb32()));

      if (stroke) {
        _qtContext->setPen(color);
        _qtContext->drawRect(QRectF(rect._x, rect._y, rect._w, rect._h));
      }
      else {
        _qtContext->fillRect(QRectF(rect._x, rect._y, rect._w, rect._h), color);
      }
    }
    else {
      QBrush brush(setupStyle<b2d::Rect>(style, rect));

      if (stroke) {
        _qtContext->setPen(QPen(brush, qreal(_params.strokeWidth)));
        _qtContext->drawRect(QRectF(rect._x, rect._y, rect._w, rect._h));
      }
      else {
        _qtContext->fillRect(QRectF(rect._x, rect._y, rect._w, rect._h), brush);
      }
    }

    _qtContext->resetMatrix();
  }
}

void QtModule::onDoRoundSmooth(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;
  double wh = _params.shapeSize;

  if (stroke)
    _qtContext->setBrush(Qt::NoBrush);
  else
    _qtContext->setPen(QPen(Qt::NoPen));

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
    double radius = _rndExtra.nextDouble(4.0, 40.0);

    if (style == kBenchStyleSolid) {
      QColor color(QtUtil::toQColor(_rndColor.nextArgb32()));

      if (stroke)
        _qtContext->setPen(QPen(color, qreal(_params.strokeWidth)));
      else
        _qtContext->setBrush(QBrush(color));
    }
    else {
      QBrush brush(setupStyle<b2d::Rect>(style, rect));

      if (stroke)
        _qtContext->setPen(QPen(brush, qreal(_params.strokeWidth)));
      else
        _qtContext->setBrush(brush);
    }

    _qtContext->drawRoundedRect(
      QRectF(rect._x, rect._y, rect._w, rect._h),
      std::min(rect._w * 0.5, radius),
      std::min(rect._h * 0.5, radius));
  }
}

void QtModule::onDoRoundRotated(bool stroke) {
  b2d::Size bounds(_params.screenW, _params.screenH);
  uint32_t style = _params.style;

  double cx = double(_params.screenW) * 0.5;
  double cy = double(_params.screenH) * 0.5;
  double wh = _params.shapeSize;
  double angle = 0.0;

  if (stroke)
    _qtContext->setBrush(Qt::NoBrush);
  else
    _qtContext->setPen(QPen(Qt::NoPen));

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++, angle += 0.01) {
    b2d::Rect rect(_rndCoord.nextRect(bounds, wh, wh));
    double radius = _rndExtra.nextDouble(4.0, 40.0);

    QTransform transform;
    transform.translate(cx, cy);
    transform.rotateRadians(angle);
    transform.translate(-cx, -cy);
    _qtContext->setTransform(transform, false);

    if (style == kBenchStyleSolid) {
      QColor color(QtUtil::toQColor(_rndColor.nextArgb32()));
      if (stroke)
        _qtContext->setPen(QPen(color, qreal(_params.strokeWidth)));
      else
        _qtContext->setBrush(QBrush(color));
    }
    else {
      QBrush brush(setupStyle<b2d::Rect>(style, rect));
      if (stroke)
        _qtContext->setPen(QPen(brush, qreal(_params.strokeWidth)));
      else
        _qtContext->setBrush(brush);
    }

    _qtContext->drawRoundedRect(
      QRectF(rect._x, rect._y, rect._w, rect._h),
      std::min(rect._w * 0.5, radius),
      std::min(rect._h * 0.5, radius));

    _qtContext->resetMatrix();
  }
}

void QtModule::onDoPolygon(uint32_t mode, uint32_t complexity) {
  b2d::IntSize bounds(_params.screenW - _params.shapeSize,
                      _params.screenH - _params.shapeSize);
  uint32_t style = _params.style;
  double wh = double(_params.shapeSize);

  bool stroke = (mode == 2);
  Qt::FillRule fillRule = (mode != 0) ? Qt::OddEvenFill : Qt::WindingFill;

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::Point base(_rndCoord.nextPoint(bounds));

    double x = _rndCoord.nextDouble(base._x, base._x + wh);
    double y = _rndCoord.nextDouble(base._y, base._y + wh);

    QPainterPath path;
    path.setFillRule(fillRule);
    path.moveTo(x, y);

    for (uint32_t p = 1; p < complexity; p++) {
      x = _rndCoord.nextDouble(base._x, base._x + wh);
      y = _rndCoord.nextDouble(base._y, base._y + wh);
      path.lineTo(x, y);
    }

    if (style == kBenchStyleSolid) {
      QColor color(QtUtil::toQColor(_rndColor.nextArgb32()));
      if (stroke)
        _qtContext->strokePath(path, QPen(color, qreal(_params.strokeWidth)));
      else
        _qtContext->fillPath(path, QBrush(color));
    }
    else {
      b2d::Rect rect(base._x, base._y, wh, wh);
      QBrush brush(setupStyle<b2d::Rect>(style, rect));

      if (stroke)
        _qtContext->strokePath(path, QPen(brush, qreal(_params.strokeWidth)));
      else
        _qtContext->fillPath(path, brush);
    }
  }
}

void QtModule::onDoShape(bool stroke, const b2d::Point* pts, size_t count) {
  b2d::IntSize bounds(_params.screenW - _params.shapeSize,
                      _params.screenH - _params.shapeSize);
  uint32_t style = _params.style;

  // No idea who invented this, but you need a `cairo_t` to create a `cairo_path_t`.
  QPainterPath path;

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
      path.moveTo(qreal(x * wh), qreal(y * wh));
      start = false;
    }
    else {
      path.lineTo(qreal(x * wh), qreal(y * wh));
    }
  }

  for (uint32_t i = 0, quantity = _params.quantity; i < quantity; i++) {
    b2d::Point base(_rndCoord.nextPoint(bounds));

    _qtContext->save();
    _qtContext->translate(qreal(base._x), qreal(base._y));

    if (style == kBenchStyleSolid) {
      QColor color(QtUtil::toQColor(_rndColor.nextArgb32()));
      if (stroke)
        _qtContext->strokePath(path, QPen(color, qreal(_params.strokeWidth)));
      else
        _qtContext->fillPath(path, QBrush(color));
    }
    else {
      b2d::Rect rect(0, 0, wh, wh);
      QBrush brush(setupStyle<b2d::Rect>(style, rect));

      if (stroke)
        _qtContext->strokePath(path, QPen(brush, qreal(_params.strokeWidth)));
      else
        _qtContext->fillPath(path, brush);
    }

    _qtContext->restore();
  }
}

} // bench namespace

#endif // BENCH_ENABLE_QT
