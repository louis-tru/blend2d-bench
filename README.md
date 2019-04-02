BL-Bench - Blend2D Benchmarking Tool
====================================

This is a Blend2D benchmarking application that can be used to test Blend2D performance and compare it with AGG, Cairo, and Qt. Cairo and Qt backends require these libraries installed so the build script can autodetect them, otherwise the backends will be disabled.

The renderings should always be the same regardless of the engine used so the numbers reported by the tool should be considered precise. There are some limitations of some engines (like Qt doesn't support reflecting textures, etc) so all tests were designed in a way to use only common features found in all 2D engines tested.
