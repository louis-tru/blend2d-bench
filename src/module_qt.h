#ifndef _B2D_BENCH_MODULE_QT_H
#define _B2D_BENCH_MODULE_QT_H

#include "./module_base.h"

#include <QtCore>
#include <QtGui>

namespace bench {

// ============================================================================
// [bench::QtUtil]
// ============================================================================

struct QtUtil {
  static uint32_t toQtFormat(uint32_t pixelFormat);
  static uint32_t toQtOperator(uint32_t compOp);

  static B2D_INLINE QColor toQColor(const b2d::Argb32& argb) {
    return QColor(argb.r(), argb.g(), argb.b(), argb.a());
  }
};

// ============================================================================
// [bench::QtModule]
// ============================================================================

struct QtModule : public BenchModule {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  QtModule();
  virtual ~QtModule();

  // --------------------------------------------------------------------------
  // [Helpers]
  // --------------------------------------------------------------------------

  template<typename RectT>
  B2D_INLINE QBrush setupStyle(uint32_t style, const RectT& rect);

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  virtual bool supportsCompOp(uint32_t compOp) const;
  virtual bool supportsStyle(uint32_t style) const;

  virtual void onBeforeRun();
  virtual void onAfterRun();

  virtual void onDoRectAligned(bool stroke);
  virtual void onDoRectSmooth(bool stroke);
  virtual void onDoRectRotated(bool stroke);
  virtual void onDoRoundSmooth(bool stroke);
  virtual void onDoRoundRotated(bool stroke);
  virtual void onDoPolygon(uint32_t mode, uint32_t complexity);
  virtual void onDoShape(bool stroke, const b2d::Point* pts, size_t count);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  b2d::ImageBuffer _lockedSurface;
  QImage* _qtSurface;
  QImage* _qtSprites[kBenchNumSprites];
  QPainter* _qtContext;

  // Initialized by onBeforeRun().
  uint32_t _gradientSpread;
};

} // namespace bench

#endif // _B2D_BENCH_MODULE_QT_H
