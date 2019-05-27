#include <stdio.h>
#include <string.h>

#include "./app.h"
#include "./images_data.h"
#include "./module_blend2d.h"

#if defined(BLBENCH_ENABLE_AGG)
  #include "./module_agg.h"
#endif // BLBENCH_ENABLE_CAIRO

#if defined(BLBENCH_ENABLE_CAIRO)
  #include "./module_cairo.h"
#endif // BLBENCH_ENABLE_CAIRO

#if defined(BLBENCH_ENABLE_QT)
  #include "./module_qt.h"
#endif // BLBENCH_ENABLE_QT

#define ARRAY_SIZE(X) uint32_t(sizeof(X) / sizeof(X[0]))

namespace blbench {

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
  "SrcOver",
  "SrcCopy",
  "SrcIn",
  "SrcOut",
  "SrcAtop",
  "DstOver",
  "DstCopy",
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
    if (strcmp(listData[i], key) == 0)
      return uint32_t(i);
  return 0xFFFFFFFFu;
}

// ============================================================================
// [bench::BenchApp - Construction / Destruction]
// ============================================================================

BenchApp::BenchApp(int argc, char** argv)
  : _argc(argc),
    _argv(argv),
    _isolated(false),
    _deepBench(false),
    _saveImages(false),
    _repeat(1),
    _quantity(1000) {}
BenchApp::~BenchApp() {}

// ============================================================================
// [bench::BenchApp - Args]
// ============================================================================

bool BenchApp::hasArg(const char* key) const {
  int argc = _argc;
  char** argv = _argv;

  for (int i = 1; i < argc; i++) {
    if (strcmp(key, argv[i]) == 0)
      return true;
  }

  return false;
}

const char* BenchApp::valueOf(const char* key) const {
  int argc = _argc;
  char** argv = _argv;

  size_t keySize = strlen(key);
  for (int i = 1; i < argc; i++) {
    const char* val = argv[i];
    if (strlen(val) >= keySize + 1 && val[keySize] == '=' && memcmp(val, key, keySize) == 0)
      return val + keySize + 1;
  }

  return NULL;
}

int BenchApp::intValueOf(const char* key, int defaultValue) const {
  int argc = _argc;
  char** argv = _argv;

  size_t keySize = strlen(key);
  for (int i = 1; i < argc; i++) {
    const char* val = argv[i];
    if (strlen(val) >= keySize + 1 && val[keySize] == '=' && memcmp(val, key, keySize) == 0) {
      const char* s = val + keySize + 1;
      return atoi(s);
    }
  }

  return defaultValue;
}

// ============================================================================
// [bench::BenchApp - Init]
// ============================================================================

bool BenchApp::init() {
  _isolated = hasArg("--isolated");
  _deepBench = hasArg("--deep");
  _saveImages = hasArg("--save");
  _compOp = 0xFFFFFFFFu;
  _repeat = intValueOf("--repeat", 1);
  _quantity = intValueOf("--quantity", 1000);

  if (_repeat <= 0 || _repeat > 100) {
    printf("ERROR: Invalid repeat [%d] specified\n", _repeat);
    return false;
  }

  if (_quantity <= 0 || _quantity > 100000) {
    printf("ERROR: Invalid quantity [%d] specified\n", _quantity);
    return false;
  }

  const char* compOpName = valueOf("--compOp");
  if (compOpName != NULL)
    _compOp = searchStringList(benchCompOpList, ARRAY_SIZE(benchCompOpList), compOpName);

  info();

  return readImage(_sprites[0], "#0", _resource_babelfish_png, sizeof(_resource_babelfish_png)) &&
         readImage(_sprites[1], "#1", _resource_ksplash_png  , sizeof(_resource_ksplash_png  )) &&
         readImage(_sprites[2], "#2", _resource_ktip_png     , sizeof(_resource_ktip_png     )) &&
         readImage(_sprites[3], "#3", _resource_firewall_png , sizeof(_resource_firewall_png ));
}

void BenchApp::info() {
  BLRuntimeBuildInfo buildInfo;
  BLRuntime::queryBuildInfo(&buildInfo);

  const char no_yes[][4] = { "no", "yes" };

  printf(
    "Blend2D Benchmarking Tool\n"
    "\n"
    "Blend2D Information:\n"
    "  Version    : %u.%u.%u\n"
    "  Build Type : %s\n"
    "  Compiled By: %s\n",
    buildInfo.majorVersion,
    buildInfo.minorVersion,
    buildInfo.patchVersion,
    buildInfo.buildType == BL_RUNTIME_BUILD_TYPE_DEBUG ? "Debug" : "Release",
    buildInfo.compilerInfo);

  printf(
    "\n"
    "The following options are supported/used:\n"
    "  --save       [%s] Save all generated images as .bmp files\n"
    "  --deep       [%s] More tests that use gradients and textures\n"
    "  --isolated   [%s] Use Blend2D isolated context (useful for development)\n"
    "  --repeat=N   [%d] Number of repeats of each test to select the best time\n"
    "  --quantity=N [%d] Override the default quantity of each operation\n"
    "\n",
    no_yes[_deepBench],
    no_yes[_saveImages],
    no_yes[_isolated],
    _repeat,
    _quantity);
}

bool BenchApp::readImage(BLImage& image, const char* name, const void* data, size_t size) noexcept {
  BLResult result = image.readFromData(data, size);
  if (result != BL_SUCCESS) {
    printf("Failed to read an image '%s' used for benchmarking\n", name);
    return false;
  }
  else {
    return true;
  }
}

// ============================================================================
// [bench::BenchApp - Helpers]
// ============================================================================

bool BenchApp::isStyleEnabled(uint32_t style) {
  if (_deepBench)
    return true;

  // If this is not a deep run we just select the following styles to be tested:
  return style == kBenchStyleSolid     ||
         style == kBenchStyleLinearPad ||
         style == kBenchStyleRadialPad ||
         style == kBenchStyleConical   ||
         style == kBenchStylePatternNN ||
         style == kBenchStylePatternBI ;
}

// ============================================================================
// [bench::BenchApp - Run]
// ============================================================================

int BenchApp::run() {
  BenchParams params;
  memset(&params, 0, sizeof(params));

  params.screenW = 600;
  params.screenH = 512;

  params.format = BL_FORMAT_PRGB32;
  params.quantity = _quantity;
  params.strokeWidth = 2.0;

  if (_isolated) {
    BLRuntimeSystemInfo si;
    BLRuntime::querySystemInfo(&si);

    // Only use features that could actually make a difference.
    static const uint32_t x86Features[] = {
      BL_RUNTIME_CPU_FEATURE_X86_SSE2,
      BL_RUNTIME_CPU_FEATURE_X86_SSSE3,
      BL_RUNTIME_CPU_FEATURE_X86_SSE4_1
    };

    const uint32_t* features = x86Features;
    uint32_t featureCount = ARRAY_SIZE(x86Features);

    for (uint32_t i = 0; i < featureCount; i++) {
      if ((si.cpuFeatures & features[i]) == features[i]) {
        Blend2DModule module(features[i]);
        runModule(module, params);
      }
    }
  }
  else {
    {
      Blend2DModule module;
      runModule(module, params);
    }

    #if defined(BLBENCH_ENABLE_AGG)
    {
      AGGModule module;
      runModule(module, params);
    }
    #endif

    #if defined(BLBENCH_ENABLE_CAIRO)
    {
      CairoModule module;
      runModule(module, params);
    }
    #endif

    #if defined(BLBENCH_ENABLE_QT)
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

  uint32_t ticksLocal[ARRAY_SIZE(benchShapeSizeList)];
  uint32_t ticksTotal[ARRAY_SIZE(benchShapeSizeList)];

  uint32_t compOpFirst = BL_COMP_OP_SRC_OVER;
  uint32_t compOpLast  = BL_COMP_OP_SRC_COPY;

  if (_compOp != 0xFFFFFFFFu) {
    compOpFirst = compOpLast = _compOp;
  }

  BLImageCodec bmpCodec;
  bmpCodec.findByName("BMP");

  for (uint32_t compOp = compOpFirst; compOp <= compOpLast; compOp++) {
    if (!module.supportsCompOp(compOp))
      continue;
    params.compOp = compOp;

    for (uint32_t style = 0; style < kBenchStyleCount; style++) {
      if (!isStyleEnabled(style) || !module.supportsStyle(style))
        continue;
      params.style = style;

      // Remove '@' from the style name if not running a deep benchmark.
      strcpy(styleString, benchStyleModeList[style]);
      if (!_deepBench) {
        char* x = strchr(styleString, '@');
        if (x != NULL) x[0] = '\0';
      }

      memset(ticksTotal, 0, sizeof(ticksTotal));

      printf(benchBorderStr);
      printf(benchHeaderStr, module._name);
      printf(benchBorderStr);

      for (uint32_t testId = 0; testId < kBenchIdCount; testId++) {
        params.benchId = testId;

        for (uint32_t sizeId = 0; sizeId < ARRAY_SIZE(benchShapeSizeList); sizeId++) {
          params.shapeSize = benchShapeSizeList[sizeId];

          uint32_t ticks = 0xFFFFFFFFu;
          for (uint32_t attempt = 0; attempt < _repeat; attempt++) {
            module.run(*this, params);

            if (ticks > module._ticks)
              ticks = module._ticks;
          }

          ticksLocal[sizeId]  = ticks;
          ticksTotal[sizeId] += ticks;

          if (_saveImages) {
            // Save only the last two as these are easier to compare visually.
            if (sizeId >= ARRAY_SIZE(benchShapeSizeList) - 2) {
              sprintf(fileName, "%s-%s-%s-%s-%c.bmp",
                module._name,
                benchIdNameList[params.benchId],
                benchCompOpList[params.compOp],
                styleString,
                'A' + sizeId);
              module._surface.writeToFile(fileName, bmpCodec);
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

} // {blbench}

// ============================================================================
// [Main]
// ============================================================================

int main(int argc, char* argv[]) {
  blbench::BenchApp app(argc, argv);

  if (!app.init()) {
    printf("Failed to initialize bl_bench.\n");
    return 1;
  }

  return app.run();
}
