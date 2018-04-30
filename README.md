B2D-Bench - Blend2D Benchmarking Tool
=====================================

This is a Blend2D benchmarking application that can be used to test Blend2D performance and compare it with Cairo and Qt. The tool repeatedly renders a set of commands and records the time spent. The renderings should always be the same regardless of the engine used so the numbers reported by the tool should be consistent and usable in comparisons. There are some limitations of some engines (like Qt doesn't support reflecting textures, etc) so all tests were designed in a way to use only common features found in all 2D engines.
