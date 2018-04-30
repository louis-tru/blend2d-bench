#ifndef _B2D_BENCH_SHAPES_DATA_H
#define _B2D_BENCH_SHAPES_DATA_H

#include "./module_base.h"

namespace bench {

struct ShapesData {
  enum Id {
    kIdWorld = 0,
    kIdCount
  };

  const b2d::Point* data;
  size_t count;
};

bool getShapesData(ShapesData& dst, uint32_t id);

} // namespace bench

#endif // _B2D_BENCH_SHAPES_DATA_H
