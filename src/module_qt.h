#ifndef BLBENCH_MODULE_QT_H
#define BLBENCH_MODULE_QT_H

#include "./module.h"

#include <QtCore>
#include <QtGui>

namespace blbench {

// ============================================================================
// [bench::QtUtil]
// ============================================================================

struct QtUtil {
  static uint32_t toQtFormat(uint32_t format);
  static uint32_t toQtOperator(uint32_t compOp);

  static inline QColor toQColor(const BLRgba32& rgba) {
    return QColor(rgba.r, rgba.g, rgba.b, rgba.a);
  }
};

// ============================================================================
// [bench::QtModule]
// ============================================================================

struct QtModule : public BenchModule {
  QImage* _qtSurface;
  QImage* _qtSprites[kBenchNumSprites];
  QPainter* _qtContext;

  // Initialized by onBeforeRun().
  uint32_t _gradientSpread;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  QtModule();
  virtual ~QtModule();

  // --------------------------------------------------------------------------
  // [Helpers]
  // --------------------------------------------------------------------------

  template<typename RectT>
  inline QBrush setupStyle(uint32_t style, const RectT& rect);

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
  virtual void onDoShape(bool stroke, const BLPoint* pts, size_t count);
};

} // {blbench}

#endif // BLBENCH_MODULE_QT_H
