#pragma once

#include "bits.h"

namespace secs {

typedef std::uint32_t ctflags_t;
typedef std::uint32_t ctindex_t;

class ComponentTraits
{
public:
	template <typename T>
	static ctflags_t getFlag() {
		static const ctflags_t flag = pow(2, ComponentTraits::getIndex<T>());
		return flag;
	}

	template <typename T>
	static ctindex_t getIndex() {
		static const ctindex_t index = nextTypeIndex++;
		return index;
	}

	static ctindex_t getNumComponents() {
		return nextTypeIndex;
	}

private:
	static ctindex_t nextTypeIndex;

};


template <typename... Rest>
struct ComponentBitsBuilder;

template <typename First, typename... Rest>
struct ComponentBitsBuilder<First, Rest...>
{
	static ComponentBits buildBits()
	{
		ComponentBits b;
		b.set(ComponentTraits::getIndex<First>());
		return b | ComponentBitsBuilder<Rest...>::buildBits();
	}
};

template <>
struct ComponentBitsBuilder<>
{
	static ComponentBits buildBits()
	{
		return ComponentBits();
	}
};

}
