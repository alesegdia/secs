#pragma once

#include "bits.h"

namespace secs {

    typedef std::uint32_t ctflags_t;
    typedef std::uint32_t ctindex_t;

    /**
     * @class ComponentTraits
     * @brief Provides traits and utilities for component types.
     */
    class ComponentTraits
    {
    public:
        /**
         * @brief Gets the flag for a given component type.
         * @tparam T The component type.
         * @return The flag associated with the component type.
         */
        template <typename T>
        static ctflags_t GetFlagForComponentType() {
            static const ctflags_t flag = pow(2, ComponentTraits::GetIndexForComponentType<T>());
            return flag;
        }

        /**
         * @brief Gets the index for a given component type.
         * @tparam T The component type.
         * @return The index associated with the component type.
         */
        template <typename T>
        static ctindex_t GetIndexForComponentType() {
            static const ctindex_t index = nextTypeIndex++;
            return index;
        }

        /**
         * @brief Gets the total number of component types.
         * @return The number of component types.
         */
        static ctindex_t GetNumberOfComponents() {
            return nextTypeIndex;
        }

    private:
        static ctindex_t nextTypeIndex;

    };

    /**
     * @brief Helper struct to build component bits for a list of component types.
     * @tparam Rest The remaining component types.
     */
    template <typename... Rest>
    struct ComponentBitsBuilder;

    /**
     * @brief Specialization of ComponentBitsBuilder for a non-empty list of component types.
     * @tparam First The first component type.
     * @tparam Rest The remaining component types.
     */
    template <typename First, typename... Rest>
    struct ComponentBitsBuilder<First, Rest...>
    {
        /**
         * @brief Builds the component bits for the given component types.
         * @return The component bits.
         */
        static ComponentBits BuildBits()
        {
            ComponentBits b;
            b.set(ComponentTraits::GetIndexForComponentType<First>());
            return b | ComponentBitsBuilder<Rest...>::BuildBits();
        }
    };

    /**
     * @brief Specialization of ComponentBitsBuilder for an empty list of component types.
     */
    template <>
    struct ComponentBitsBuilder<>
    {
        /**
         * @brief Builds the component bits for an empty list of component types.
         * @return The component bits.
         */
        static ComponentBits BuildBits()
        {
            return ComponentBits();
        }
    };

} // namespace secs