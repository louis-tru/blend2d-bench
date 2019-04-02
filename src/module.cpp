#include "./app.h"
#include "./module.h"
#include "./shapes_data.h"

namespace blbench {

// ============================================================================
// [bench::BenchModule - Construction / Destruction]
// ============================================================================

BenchModule::BenchModule()
  : _name(),
    _params(),
    _ticks(0),
    _rndCoord(0x19AE0DDAE3FA7391ull),
    _rndColor(0x94BD7A499AD10011ull),
    _rndExtra(0x1ABD9CC9CAF0F123ull),
    _rndSpriteId(0) {}
BenchModule::~BenchModule() {}

// ============================================================================
// [bench::BenchModule - Run]
// ============================================================================

static void BenchModule_onDoShapeHelper(BenchModule* module, bool stroke, uint32_t shapeId) {
  ShapesData shape;
  getShapesData(shape, shapeId);
  module->onDoShape(stroke, shape.data, shape.count);
}

void BenchModule::run(const BenchApp& app, const BenchParams& params) {
  _params = params;

  _rndCoord.rewind();
  _rndColor.rewind();
  _rndExtra.rewind();
  _rndSpriteId = 0;

  // Initialize the sprites.
  for (uint32_t i = 0; i < kBenchNumSprites; i++) {
    BLImage::scale(
      _sprites[i],
      app._sprites[i],
      BLSizeI(params.shapeSize, params.shapeSize), BL_IMAGE_SCALE_FILTER_BILINEAR);
  }

  onBeforeRun();
  _ticks = BLRuntime::getTickCount();

  switch (_params.benchId) {
    case kBenchIdFillAlignedRect   : onDoRectAligned(false); break;
    case kBenchIdFillSmoothRect    : onDoRectSmooth(false); break;
    case kBenchIdFillRotatedRect   : onDoRectRotated(false); break;
    case kBenchIdFillSmoothRound   : onDoRoundSmooth(false); break;
    case kBenchIdFillRotatedRound  : onDoRoundRotated(false); break;
    case kBenchIdFillPolygon10NZ   : onDoPolygon(0, 10); break;
    case kBenchIdFillPolygon10EO   : onDoPolygon(1, 10); break;
    case kBenchIdFillPolygon20NZ   : onDoPolygon(0, 20); break;
    case kBenchIdFillPolygon20EO   : onDoPolygon(1, 20); break;
    case kBenchIdFillPolygon40NZ   : onDoPolygon(0, 40); break;
    case kBenchIdFillPolygon40EO   : onDoPolygon(1, 40); break;
    case kBenchIdFillShapeWorld    : BenchModule_onDoShapeHelper(this, false, ShapesData::kIdWorld); break;

    case kBenchIdStrokeAlignedRect : onDoRectAligned(true); break;
    case kBenchIdStrokeSmoothRect  : onDoRectSmooth(true); break;
    case kBenchIdStrokeRotatedRect : onDoRectRotated(true); break;
    case kBenchIdStrokeSmoothRound : onDoRoundSmooth(true); break;
    case kBenchIdStrokeRotatedRound: onDoRoundRotated(true); break;
    case kBenchIdStrokePolygon10   : onDoPolygon(2, 10); break;
    case kBenchIdStrokePolygon20   : onDoPolygon(2, 20); break;
    case kBenchIdStrokePolygon40   : onDoPolygon(2, 40); break;
    case kBenchIdStrokeShapeWorld  : BenchModule_onDoShapeHelper(this, true, ShapesData::kIdWorld); break;
  }

  _ticks = BLRuntime::getTickCount() - _ticks;
  onAfterRun();
}

} // {blbench}
