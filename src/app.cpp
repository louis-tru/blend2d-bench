#include <stdio.h>
#include <string.h>

#include "./app.h"
#include "./images_data.h"
#include "./module_b2d.h"

#if defined(BENCH_ENABLE_CAIRO)
  #include "./module_cairo.h"
#endif // BENCH_ENABLE_CAIRO

#if defined(BENCH_ENABLE_QT)
  #include "./module_qt.h"
#endif // BENCH_ENABLE_QT

namespace bench {

// ============================================================================
// [bench::BenchApp - Constants]
// ============================================================================

static const char* benchIdNameList[] = {
  "FillRectAA",
  "FillRectNA",
  "FillRectRot",
  "FillRoundNA",
  "FillRoundRot",
  "FillPolyNZi10",
  "FillPolyEOi10",
  "FillPolyNZi20",
  "FillPolyEOi20",
  "FillPolyNZi40",
  "FillPolyEOi40",
  "FillWorld",
  "StrokeRectAA",
  "StrokeRectNA",
  "StrokeRectRot",
  "StrokeRoundNA",
  "StrokeRoundRot",
  "StrokePoly10",
  "StrokePoly20",
  "StrokePoly40",
  "StrokeWorld"
};

static const char* benchCompOpList[] = {
  "Src",
  "SrcOver",
  "SrcIn",
  "SrcOut",
  "SrcAtop",
  "Dst",
  "DstOver",
  "DstIn",
  "DstOut",
  "DstAtop",
  "Xor",
  "Clear",
  "Plus",
  "Merge",
  "Minus",
  "Multiply",
  "Screen",
  "Overlay",
  "Darken",
  "Lighten",
  "ColorDodge",
  "ColorBurn",
  "LinearBurn",
  "LinearLight",
  "PinLight",
  "HardLight",
  "SoftLight",
  "Difference",
  "Exclusion"
};

static const char* benchStyleModeList[] = {
  "Solid",
  "Linear@Pad",
  "Linear@Repeat",
  "Linear@Reflect",
  "Radial@Pad",
  "Radial@Repeat",
  "Radial@Reflect",
  "Conical",
  "Pattern_NN",
  "Pattern_BI"
};

static const int benchShapeSizeList[] = {
  8, 16, 32, 64, 128, 256
};

const char benchBorderStr[] = "+--------------------+-------------+---------------+-------+-------+-------+-------+-------+-------+\n";
const char benchHeaderStr[] = "|%-20s"             "| CompOp      | Style         | 8x8   | 16x16 | 32x32 | 64x64 |128x128|256x256|\n";
const char benchDataFmt[]   = "|%-20s"             "| %-12s"     "| %-14s"       "| %-6u""| %-6u""| %-6u""| %-6u""| %-6u""| %-6u""|\n";

static uint32_t searchStringList(const char** listData, size_t listSize, const char* key) {
  for (size_t i = 0; i < listSize; i++)
    if (::strcmp(listData[i], key) == 0)
      return uint32_t(i);
  return 0xFFFFFFFFu;
}

// ============================================================================
// [bench::BenchApp - Construction / Destruction]
// ============================================================================

BenchApp::BenchApp(int argc, char** argv)
  : _argc(argc),
    _argv(argv),
    _saveImages(false),
    _b2dTune(false) {}
BenchApp::~BenchApp() {}

// ============================================================================
// [bench::BenchApp - Args]
// ============================================================================

bool BenchApp::hasArg(const char* key) const {
  int argc = _argc;
  char** argv = _argv;

  for (int i = 1; i < argc; i++) {
    if (::strcmp(key, argv[i]) == 0)
      return true;
  }

  return false;
}

const char* BenchApp::valueOf(const char* key) const {
  int argc = _argc;
  char** argv = _argv;

  size_t keySize = std::strlen(key);
  for (int i = 1; i < argc; i++) {
    const char* val = argv[i];
    if (::strlen(val) >= keySize + 1 && val[keySize] == '=')
      return val + keySize + 1;
  }

  return NULL;
}

// ============================================================================
// [bench::BenchApp - Init]
// ============================================================================

bool BenchApp::init() {
  _deepBench = hasArg("--deep");
  _saveImages = hasArg("--save");
  _b2dTune = hasArg("--b2d-tune");
  _compOp = 0xFFFFFFFFu;

  const char* compOpName = valueOf("--compOp");
  if (compOpName != NULL)
    _compOp = searchStringList(benchCompOpList, B2D_ARRAY_SIZE(benchCompOpList), compOpName);

  info();

  b2d::ImageCodecArray codecs = b2d::ImageCodec::builtinCodecs();
  b2d::ImageUtils::readImageFromData(_sprites[0], codecs, _resource_babelfish_png, sizeof(_resource_babelfish_png));
  b2d::ImageUtils::readImageFromData(_sprites[1], codecs, _resource_ksplash_png  , sizeof(_resource_ksplash_png  ));
  b2d::ImageUtils::readImageFromData(_sprites[2], codecs, _resource_ktip_png     , sizeof(_resource_ktip_png     ));
  b2d::ImageUtils::readImageFromData(_sprites[3], codecs, _resource_firewall_png , sizeof(_resource_firewall_png ));

  return !_sprites[0].empty() &&
         !_sprites[1].empty() &&
         !_sprites[2].empty() &&
         !_sprites[3].empty() ;
}

void BenchApp::info() {
  const char checked[][2] = { " ", "x" };

  printf(
    "b2d_bench - Blend2D benchmarking tool.\n"
    "\n"
    "The following options are available:\n"
    "  --deep     [%s] More fetch-tests that use gradients and textures\n"
    "  --save     [%s] Save all generated images as .bmp files\n"
    "  --b2d-tune [%s] Run Blend2D tests with various optimization levels\n"
    "\n",
    checked[_deepBench],
    checked[_saveImages],
    checked[_b2dTune]);
}

// ============================================================================
// [bench::BenchApp - Helpers]
// ============================================================================

bool BenchApp::isStyleEnabled(uint32_t style) {
  if (_deepBench)
    return true;

  // If this is not a deep run we just select the following styles to be tested:
  return style == kBenchStyleSolid          ||
         style == kBenchStyleLinearPad      ||
         style == kBenchStyleRadialPad      ||
         style == kBenchStyleConical        ||
         style == kBenchStylePatternNN      ||
         style == kBenchStylePatternBI      ;
}

// ============================================================================
// [bench::BenchApp - Run]
// ============================================================================

int BenchApp::run() {
  BenchParams params;
  std::memset(&params, 0, sizeof(params));

  params.screenW = 600;
  params.screenH = 480;

  params.pixelFormat = b2d::PixelFormat::kPRGB32;
  params.quantity = 1000;
  params.strokeWidth = 2.0;

  if (_b2dTune) {
    uint32_t minOptLevel = b2d::Runtime::kOptLevel_X86_SSE2;
    uint32_t maxOptLevel = b2d::Runtime::hwInfo().optLevel();

    for (uint32_t optLevel = minOptLevel; optLevel <= maxOptLevel; optLevel++) {
      // Skip these as they don't provide any benefit.
      if (optLevel == b2d::Runtime::kOptLevel_X86_SSE3 ||
          optLevel == b2d::Runtime::kOptLevel_X86_SSE4_2)
        continue;

      BlendModule module(optLevel);
      runModule(module, params);
    }
  }
  else {
    {
      BlendModule module;
      runModule(module, params);
    }

    #if defined(BENCH_ENABLE_CAIRO)
    {
      CairoModule module;
      runModule(module, params);
    }
    #endif

    #if defined(BENCH_ENABLE_QT)
    {
      QtModule module;
      runModule(module, params);
    }
    #endif
  }

  return 0;
}

int BenchApp::runModule(BenchModule& module, BenchParams& params) {
  char fileName[256];
  char styleString[128];

  uint32_t ticksLocal[B2D_ARRAY_SIZE(benchShapeSizeList)];
  uint32_t ticksTotal[B2D_ARRAY_SIZE(benchShapeSizeList)];

  uint32_t compOpFirst = b2d::CompOp::kSrc;
  uint32_t compOpLast  = b2d::CompOp::kSrcOver;

  if (_compOp != 0xFFFFFFFFu) {
    compOpFirst = compOpLast = _compOp;
  }

  for (uint32_t compOp = compOpFirst; compOp <= compOpLast; compOp++) {
    if (!module.supportsCompOp(compOp))
      continue;
    params.compOp = compOp;

    for (uint32_t style = 0; style < kBenchStyleCount; style++) {
      if (!isStyleEnabled(style) || !module.supportsStyle(style))
        continue;
      params.style = style;

      // Remove '@' from the style name if not running a deep benchmark.
      ::strcpy(styleString, benchStyleModeList[style]);
      if (!_deepBench) {
        char* x = ::strchr(styleString, '@');
        if (x != NULL) x[0] = '\0';
      }

      std::memset(ticksTotal, 0, sizeof(ticksTotal));

      printf(benchBorderStr);
      printf(benchHeaderStr, module._name);
      printf(benchBorderStr);

      for (uint32_t testId = 0; testId < kBenchIdCount; testId++) {
        params.benchId = testId;

        for (uint32_t sizeId = 0; sizeId < B2D_ARRAY_SIZE(benchShapeSizeList); sizeId++) {
          params.shapeSize = benchShapeSizeList[sizeId];
          module.run(*this, params);

          ticksLocal[sizeId]  = module._ticks;
          ticksTotal[sizeId] += module._ticks;

          if (_saveImages) {
            // Save only the last as it's easier to compare visually.
            if (sizeId == B2D_ARRAY_SIZE(benchShapeSizeList) - 1) {
              sprintf(fileName, "%s-%s-%s-%s-%c.bmp",
                module._name,
                benchIdNameList[params.benchId],
                benchCompOpList[params.compOp],
                styleString,
                'A' + sizeId);
              b2d::ImageUtils::writeImageToFile(
                fileName, b2d::ImageCodec::codecByName(b2d::ImageCodec::builtinCodecs(), "BMP"), module._surface);
            }
          }
        }

        printf(benchDataFmt,
          benchIdNameList[params.benchId],
          benchCompOpList[params.compOp],
          styleString,
          ticksLocal[0],
          ticksLocal[1],
          ticksLocal[2],
          ticksLocal[3],
          ticksLocal[4],
          ticksLocal[5]);
      }

      printf(benchBorderStr);
      printf(benchDataFmt,
        "Total",
        benchCompOpList[params.compOp],
        styleString,
        ticksTotal[0],
        ticksTotal[1],
        ticksTotal[2],
        ticksTotal[3],
        ticksTotal[4],
        ticksTotal[5]);
      printf(benchBorderStr);
      printf("\n");
    }
  }

  return 0;
}

} // bench namespace

// ============================================================================
// [Main]
// ============================================================================

int main(int argc, char* argv[]) {
  bench::BenchApp app(argc, argv);

  if (!app.init()) {
    printf("Failed to initialize b2d_bench.\n");
    return 1;
  }

  return app.run();
}
