#ifndef _B2D_BENCH_MODULE_B2D_H
#define _B2D_BENCH_MODULE_B2D_H

#include <b2d/b2d.h>
#include "./module_base.h"

namespace bench {

// ============================================================================
// [bench::BlendModule]
// ============================================================================

struct BlendModule : public BenchModule {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  explicit BlendModule(uint32_t optLevel = b2d::Runtime::kOptLevel_None);
  virtual ~BlendModule();

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

  b2d::Context2D _context;
  uint32_t _optLevel;

  // Initialized by onBeforeRun().
  uint32_t _gradientType;
  uint32_t _gradientExtend;
};

} // namespace bench

#endif // _B2D_BENCH_MODULE_B2D_H
