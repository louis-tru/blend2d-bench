#ifndef BLBENCH_MODULE_BLEND2D_H
#define BLBENCH_MODULE_BLEND2D_H

#include <blend2d.h>
#include "./module.h"

namespace blbench {

// ============================================================================
// [bench::BlendModule]
// ============================================================================

struct BlendModule : public BenchModule {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  explicit BlendModule(uint32_t cpuFeatures = 0);
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
  virtual void onDoShape(bool stroke, const BLPoint* pts, size_t count);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  BLContext _context;
  uint32_t _cpuFeatures;

  // Initialized by onBeforeRun().
  uint32_t _gradientType;
  uint32_t _gradientExtend;
};

} // {blbench}

#endif // BLBENCH_MODULE_BLEND2D_H
