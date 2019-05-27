#ifndef BLBENCH_MODULE_CAIRO_H
#define BLBENCH_MODULE_CAIRO_H

#include "./module.h"

#include <cairo.h>

namespace blbench {

// ============================================================================
// [bench::CairoUtils]
// ============================================================================

struct CairoUtils {
  static uint32_t toCairoFormat(uint32_t format);
  static uint32_t toCairoOperator(uint32_t compOp);

  static void roundRect(cairo_t* ctx, const BLRect& rect, double radius);
};

// ============================================================================
// [bench::CairoModule]
// ============================================================================

struct CairoModule : public BenchModule {
  cairo_surface_t* _cairoSurface;
  cairo_surface_t* _cairoSprites[kBenchNumSprites];
  cairo_t* _cairoContext;

  // Initialized by onBeforeRun().
  uint32_t _patternExtend;
  uint32_t _patternFilter;

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
  virtual void onDoShape(bool stroke, const BLPoint* pts, size_t count);
};

} // {blbench}

#endif // BLBENCH_MODULE_CAIRO_H
