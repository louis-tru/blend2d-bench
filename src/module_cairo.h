#ifndef _B2D_BENCH_MODULE_CAIRO_H
#define _B2D_BENCH_MODULE_CAIRO_H

#include "./module_base.h"

#include <cairo.h>

namespace bench {

// ============================================================================
// [bench::CairoUtil]
// ============================================================================

struct CairoUtil {
  static uint32_t toCairoFormat(uint32_t pixelFormat);
  static uint32_t toCairoOperator(uint32_t compOp);

  static void round(cairo_t* ctx, const b2d::Rect& rect, double radius);
};

// ============================================================================
// [bench::CairoModule]
// ============================================================================

struct CairoModule : public BenchModule {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  CairoModule();
  virtual ~CairoModule();

  // --------------------------------------------------------------------------
  // [Helpers]
  // --------------------------------------------------------------------------

  template<typename RectT>
  void setupStyle(uint32_t style, const RectT& rect);

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
  cairo_surface_t* _cairoSurface;
  cairo_surface_t* _cairoSprites[kBenchNumSprites];
  cairo_t* _cairoContext;

  // Initialized by onBeforeRun().
  uint32_t _patternExtend;
  uint32_t _patternFilter;
};

} // namespace bench

#endif // _B2D_BENCH_MODULE_CAIRO_H
