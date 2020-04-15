Blend2D Benchmarking Tool
=========================

This is a Blend2D benchmarking tool that can be used to test Blend2D performance and compare it with AGG, Cairo, and Qt libraries. Cairo and Qt backends require these libraries installed so the cmake script can find them, otherwise these backends will be disabled during the configure step. AGG backend is bundled with bl_bench so it's always available and built.

The renderings should always be the same regardless of the backend used so the numbers reported by the tool should be considered precise. There are some limitations of some backends (like Qt doesn't support reflecting textures, etc) so all tests were designed in a way to use only common features found in all 2D backends tested.

Motivation
----------

bl_bench tool was written to benchmark raw performance of Blend2D and other 2D vector graphics libraries. It implements a series of tests that benchmark various parts of such libraries:

  - FillRectA - Axis aligned rectangle fill. This is one of the most optimized operations in most 2D libraries and it's definitely one of the simplest things to render. The performance of this test can reveal whether solid fills and image blits are well optimized, and the performance of very small fills and blits can reveal how much overhead there is between calling the fill/blit function and the actual pixel filling.
  - FillRectU - Axis unaligned rectangle fill. Since 2D vector engines work with floating point coordinates it's possible that a renctangle to fill or blit is not axis aligned. In that case it's expected that the corners of such rectangle would be "blurry" (antialiased). Blend2D has a specialized filler for such case and this tests the filler and compares it with other libraries.
  - FillRectRot - Fills a rotated rectangle. This case generally tests how efficiently the rendering engine can use the rasterizer and rasterize something simple. In many engines filling rotated rectangle means just transforming the coordinates and feeding the rasterizer with the output polygon. In addition, this tests the performance of rotated blits.
  - FillRoundU - Fills a rounded rectangle (not aligned to a pixel boundary). In general this should benchmark two things - how efficiently a simple path/shape can be rendered and how efficiently the engine flattens such path into a polyline or a set of edges.
  - FillRoundRot - Fills a rounded rectangle, which is rotated. This should in general be a bit slower than FillRoundU, because the rotation would spread the shape into more scanlines, which means more work for the rasterizer and the pipeline as well. So this test can be used to compare a rendering of a simple shape (FillRoundU) vs the same shape rotated.
  - FillPoly - Fills a polygon with the specified filling rule (non-zero or even-odd) and with the specified number of vectices. This test should in general reveal the performance of rasterization as polygons do not need flattning (curves do). So the tests can be used to compare the performance of rasterizing polyhons of 10, 20, and 40 vertices with both fill rules. At the moment Blend2D uses a parametrized pipeline so the fill rule doesn't matter, but other libraries may show a difference.
  - FillWorld - Fills a world example (the same path shown on Blend2D homepage). This is a very complex shape (actually it's only poly-polygon, it has no curves) to render, but doesn't have intersections and it's actually considered a real-world case as it renders something practical.
  - StrokeXXX - Strokes the input. All stroke tests are generally the same just with different inputs. They are complementary to fill tests.

Although the tests may seem simple they really test the building blocks of 2D libraries as all the rendering requests are usually simplified into rectangles or polygons and the question is how fast this can be done and how fast can the engine start compositing pixels.

Building
--------

Use the following commands to fetch asmjit, blend2d, and bl-bench:

```bash
# Download source packages from Git.
$ git clone --depth=1 https://github.com/asmjit/asmjit
$ git clone --depth=1 https://github.com/blend2d/blend2d
$ git clone --depth=1 https://github.com/blend2d/blend2d-bench

# Create build directory and build 'bl-bench'.
$ cd blend2d-bench
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ cmake --build .

# Run some benchmarks!
$ ./bl_bench
```

Alternatively you can pick a configure script from `tools` directory if there is a suitable one for your configuration.
