#pragma once

#include <cstdlib>

namespace secs
{


class Config
{
public:

	// ONLY NEEDED BECAUSE BITSET IS NOT DYNAMIC!!
	static constexpr size_t MaxNumEntities = 2048;
	static constexpr size_t MaxNumComponents = 64;

};

}

#define SECS_UNUSED(v) (void)v;
