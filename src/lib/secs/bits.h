#pragma once

#include <bitset>

#include "config.h"

namespace secs
{

typedef std::bitset<Config::MaxNumEntities> EntityBits;
typedef std::bitset<Config::MaxNumComponents> ComponentBits;

}
