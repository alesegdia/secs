#pragma once

#include "bits.h"

namespace secs {

typedef std::uint32_t ctflags_t;
typedef std::uint32_t ctindex_t;

class ComponentTraits
{
public:
	template <typename T>
	static ctflags_t GetFlagForComponentType() {
		static const ctflags_t flag = pow(2, ComponentTraits::GetIndexForComponentType<T>());
		return flag;
	}

	template <typename T>
	static ctindex_t GetIndexForComponentType() {
		static const ctindex_t index = nextTypeIndex++;
		return index;
	}

	static ctindex_t GetNumberOfComponents() {
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
	static ComponentBits BuildBits()
	{
		ComponentBits b;
		b.set(ComponentTraits::GetIndexForComponentType<First>());
		return b | ComponentBitsBuilder<Rest...>::BuildBits();
	}
};

template <>
struct ComponentBitsBuilder<>
{
	static ComponentBits BuildBits()
	{
		return ComponentBits();
	}
};

}
