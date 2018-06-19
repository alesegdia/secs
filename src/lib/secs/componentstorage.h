#pragma once

#include <vector>
#include <cassert>
#include <memory>

#include "entity.h"
#include "bits.h"

namespace secs {

class BaseComponentStorage {
public:

	typedef std::unique_ptr<BaseComponentStorage> UniquePtr;
	typedef BaseComponentStorage* Ptr;
	typedef std::shared_ptr<BaseComponentStorage> SharedPtr;

	virtual ~BaseComponentStorage()
	{

	}

};

/**
 * @brief Component addition and deletion processes are splitted in two steps.
 * 1) set/clear the bit associated to the entity
 * 2) allocate/free the component associated to the entity
 */

template <typename ComponentType>
class VectorComponentStorage : public BaseComponentStorage
{
public:

    using UniquePtr = std::unique_ptr<VectorComponentStorage<ComponentType>>;
    using SharedPtr = std::shared_ptr<VectorComponentStorage>;
    using Ptr = VectorComponentStorage<ComponentType>*;

    /**
     * @brief Allocates and return a component for the entity passed as parameter.
     * Resizes the container if entity does not fit
     * @param e
     * @return the allocated component
     */
    ComponentType& allocComponent( const Entity& e )
    {
        if( m_storage.size() < ( e.eid() + 1 ) )
        {
            m_storage.resize(e.eid() + 32);
        }
        m_storage[e.eid()] = {};
        return m_storage[e.eid()];
    }

    ComponentType& component( const Entity& e )
    {
        return m_storage[e.eid()];
    }

private:

    /**
     * @brief m_storage number of ComponentType. The container is resized when a ComponentType is
     * requested for an entity and it does not fit
     */
    std::vector<ComponentType> m_storage;


};


template <typename ComponentType>
class ArrayComponentStorage : public BaseComponentStorage
{
public:

    using UniquePtr = std::unique_ptr<ArrayComponentStorage<ComponentType>>;
    using SharedPtr = std::shared_ptr<ArrayComponentStorage>;
    using Ptr = ArrayComponentStorage<ComponentType>*;

    static constexpr size_t NumEntities = 1024;

    /**
     * @brief Allocates and return a component for the entity passed as parameter.
     * Resizes the container if entity does not fit
     * @param e
     * @return the allocated component
     */
    ComponentType& allocComponent( const Entity& e )
    {
        assert(e.eid() < ArrayComponentStorage::NumEntities);
        m_storage[e.eid()] = {};
        return m_storage[e.eid()];
    }

    ComponentType& component( const Entity& e )
    {
        return m_storage[e.eid()];
    }

private:

    /**
     * @brief m_storage number of ComponentType. The container is resized when a ComponentType is
     * requested for an entity and it does not fit
     */
    ComponentType m_storage[ArrayComponentStorage::NumEntities];


};

template <typename T>
using ComponentStorage = VectorComponentStorage<T>;

}
