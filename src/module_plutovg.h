// Blend2D - 2D Vector Graphics Powered by a JIT Compiler
//
//  * Official Blend2D Home Page: https://blend2d.com
//  * Official Github Repository: https://github.com/blend2d/blend2d
//
// Copyright (c) 2017-2020 The Blend2D Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef BLBENCH_MODULE_PLUTOVG_H
#define BLBENCH_MODULE_PLUTOVG_H

#include "./module.h"

#include <plutovg.h>

namespace blbench {

struct PlutovgModule : public BenchModule {

	plutovg_surface_t* _PlutovgSurface;
	plutovg_surface_t* _PlutovgSprites[kBenchNumSprites];
	plutovg_t* _PlutovgContext;
	
	// --------------------------------------------------------------------------
	// [Construction / Destruction]
	// --------------------------------------------------------------------------

	PlutovgModule();
	virtual ~PlutovgModule();

	// --------------------------------------------------------------------------
	// [Helpers]
	// --------------------------------------------------------------------------

	template<typename RectT>
	void setupStyle(uint32_t style, const RectT& rect);

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

#endif // BLBENCH_MODULE_Plutovg_H
