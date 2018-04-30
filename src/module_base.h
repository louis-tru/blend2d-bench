#ifndef _B2D_BENCH_MODULE_BASE_H
#define _B2D_BENCH_MODULE_BASE_H

#include <b2d/b2d.h>

#include "./module_base.h"

namespace bench {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct BenchApp;

// ============================================================================
// [bench::kBenchMisc]
// ============================================================================

enum BenchMisc {
  kBenchNumSprites = 4
};

// ============================================================================
// [bench::BenchId]
// ============================================================================

enum BenchId {
  kBenchIdFillAlignedRect,
  kBenchIdFillSmoothRect,
  kBenchIdFillRotatedRect,
  kBenchIdFillSmoothRound,
  kBenchIdFillRotatedRound,
  kBenchIdFillPolygon10NZ,
  kBenchIdFillPolygon10EO,
  kBenchIdFillPolygon20NZ,
  kBenchIdFillPolygon20EO,
  kBenchIdFillPolygon40NZ,
  kBenchIdFillPolygon40EO,
  kBenchIdFillShapeWorld,

  kBenchIdStrokeAlignedRect,
  kBenchIdStrokeSmoothRect,
  kBenchIdStrokeRotatedRect,
  kBenchIdStrokeSmoothRound,
  kBenchIdStrokeRotatedRound,
  kBenchIdStrokePolygon10,
  kBenchIdStrokePolygon20,
  kBenchIdStrokePolygon40,
  kBenchIdStrokeShapeWorld,

  kBenchIdCount
};

// ============================================================================
// [bench::BenchStyle]
// ============================================================================

enum BenchStyle {
  kBenchStyleSolid,
  kBenchStyleLinearPad,
  kBenchStyleLinearRepeat,
  kBenchStyleLinearReflect,
  kBenchStyleRadialPad,
  kBenchStyleRadialRepeat,
  kBenchStyleRadialReflect,
  kBenchStyleConical,
  kBenchStylePatternNN,
  kBenchStylePatternBI,

  kBenchStyleCount
};

// ============================================================================
// [bench::BenchParams]
// ============================================================================

struct BenchParams {
  uint32_t screenW;
  uint32_t screenH;

  uint32_t pixelFormat;
  uint32_t quantity;

  uint32_t benchId;
  uint32_t compOp;
  uint32_t style;
  uint32_t shapeSize;

  double strokeWidth;
};

// ============================================================================
// [bench::BenchRandom]
// ============================================================================

struct BenchRandom {
  B2D_INLINE BenchRandom(uint64_t seed)
    : _prng(seed) {}

  // --------------------------------------------------------------------------
  // [Rewind]
  // --------------------------------------------------------------------------

  B2D_INLINE void rewind() { _prng.rewind(); }

  // --------------------------------------------------------------------------
  // [Next]
  // --------------------------------------------------------------------------

  B2D_INLINE int nextInt() {
    return int(_prng.nextUInt32() & 0x7FFFFFFFu);
  }

  B2D_INLINE int nextInt(int a, int b) {
    return int(nextDouble(double(a), double(b)));
  }

  B2D_INLINE double nextDouble() {
    return _prng.nextDouble();
  }

  B2D_INLINE double nextDouble(double a, double b) {
    return a + _prng.nextDouble() * (b - a);
  }

  B2D_INLINE b2d::Point nextPoint(const b2d::IntSize& bounds) {
    double x = nextDouble(0.0, double(bounds._w));
    double y = nextDouble(0.0, double(bounds._h));
    return b2d::Point(x, y);
  }

  B2D_INLINE b2d::IntPoint nextIntPoint(const b2d::IntSize& bounds) {
    int x = nextInt(0, bounds._w);
    int y = nextInt(0, bounds._h);
    return b2d::IntPoint(x, y);
  }

  B2D_INLINE void nextRectT(b2d::Rect& out, const b2d::Size& bounds, double w, double h) {
    double x = nextDouble(0.0, bounds._w - w);
    double y = nextDouble(0.0, bounds._h - h);
    out.reset(x, y, w, h);
  }

  B2D_INLINE void nextRectT(b2d::IntRect& out, const b2d::IntSize& bounds, int w, int h) {
    int x = nextInt(0, bounds._w - w);
    int y = nextInt(0, bounds._h - h);
    out.reset(x, y, w, h);
  }

  B2D_INLINE b2d::Rect nextRect(const b2d::Size& bounds, double w, double h) {
    double x = nextDouble(0.0, bounds._w - w);
    double y = nextDouble(0.0, bounds._h - h);
    return b2d::Rect(x, y, w, h);
  }

  B2D_INLINE b2d::IntRect nextIntRect(const b2d::IntSize& bounds, int w, int h) {
    int x = nextInt(0, bounds._w - w);
    int y = nextInt(0, bounds._h - h);
    return b2d::IntRect(x, y, w, h);
  }

  B2D_INLINE b2d::Argb32 nextRgb32() {
    return b2d::Argb32(_prng.nextUInt32() | 0xFF000000u);
  }

  B2D_INLINE b2d::Argb32 nextArgb32() {
    return b2d::Argb32(_prng.nextUInt32());
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  b2d::Random _prng;
};

// ============================================================================
// [bench::BenchModule]
// ============================================================================

struct BenchModule {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  BenchModule();
  virtual ~BenchModule();

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  void run(const BenchApp& app, const BenchParams& params);

  // --------------------------------------------------------------------------
  // [Misc]
  // --------------------------------------------------------------------------

  B2D_INLINE uint32_t nextSpriteId() {
    uint32_t i = _rndSpriteId;
    if (++_rndSpriteId >= kBenchNumSprites)
      _rndSpriteId = 0;
    return i;
  };

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  virtual bool supportsCompOp(uint32_t compOp) const = 0;
  virtual bool supportsStyle(uint32_t style) const = 0;

  virtual void onBeforeRun() = 0;
  virtual void onAfterRun() = 0;

  virtual void onDoRectAligned(bool stroke) = 0;
  virtual void onDoRectSmooth(bool stroke) = 0;
  virtual void onDoRectRotated(bool stroke) = 0;
  virtual void onDoRoundSmooth(bool stroke) = 0;
  virtual void onDoRoundRotated(bool stroke) = 0;
  virtual void onDoPolygon(uint32_t mode, uint32_t complexity) = 0;
  virtual void onDoShape(bool stroke, const b2d::Point* pts, size_t count) = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Module name.
  char _name[64];
  //! Current parameters.
  BenchParams _params;
  //! Current ticks.
  uint32_t _ticks;

  //! Random number generator for coordinates (points or rectangles).
  BenchRandom _rndCoord;
  //! Random number generator for colors.
  BenchRandom _rndColor;
  //! Random number generator for extras (radius).
  BenchRandom _rndExtra;
  //! Random number generator for sprites.
  uint32_t _rndSpriteId;

  //! Blend surface (used by all modules).
  b2d::Image _surface;
  //! Sprites.
  b2d::Image _sprites[kBenchNumSprites];
};

} // namespace bench

#endif // _B2D_BENCH_MODULE_BASE_H
