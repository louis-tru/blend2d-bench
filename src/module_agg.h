#ifndef BLBENCH_MODULE_AGG_H
#define BLBENCH_MODULE_AGG_H

#include "./module.h"

#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa_nogamma.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"

namespace blbench {

// ============================================================================
// [bench::AGGModule]
// ============================================================================

struct AGGModule : public BenchModule {
  typedef agg::pixfmt_bgra32_pre PixelFormat;
  typedef agg::rasterizer_scanline_aa_nogamma<> Rasterizer;
  typedef agg::renderer_base<PixelFormat> RendererBase;
  typedef agg::renderer_scanline_aa_solid<RendererBase> RendererSolid;

  agg::rendering_buffer _aggSurface;
  agg::scanline_p8 _scanline;

  PixelFormat _pixfmt;
  Rasterizer _rasterizer;

  RendererBase _rendererBase;
  RendererSolid _rendererSolid;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  AGGModule();
  virtual ~AGGModule();

  // --------------------------------------------------------------------------
  // [AGG]
  // --------------------------------------------------------------------------

  void renderScanlines(const BLRect& rect, uint32_t style);
  void fillRectAA(int x, int y, int w, int h, uint32_t style);

  template<typename T>
  void rasterizePath(T& path, bool stroke);

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

#endif // BLBENCH_MODULE_AGG_H
