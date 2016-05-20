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

template <typename ComponentType>
/**
 * @brief Component addition and deletion processes are splitted in two steps.
 * 1) set/clear the bit associated to the entity
 * 2) allocate/free the component associated to the entity
 */
class ComponentStorage : public BaseComponentStorage
{
public:

	using UniquePtr = std::unique_ptr<ComponentStorage<ComponentType>>;
	using SharedPtr = std::shared_ptr<ComponentStorage>;
	using Ptr = ComponentStorage<ComponentType>*;

	/**
	 * @brief Allocates and return a component for the entity passed as parameter.
	 * Resizes the container if entity does not fit
	 * @param e
	 * @return the allocated component
	 */
	ComponentType& allocComponent( const Entity& e )
	{
		if( m_storage.size() < e.eid() )
		{
			m_storage.resize(e.eid());
		}
		return m_storage[e.eid()];
	}

	/**
	 * @brief replaces old ComponentType associated to the entity with a fresh one
	 * @param e
	 */
	void freeComponent( const Entity& e )
	{
		assert( e.eid() < m_storage.size() );
		m_storage[ e.eid() ] = {};
	}

private:

	/**
	 * @brief m_storage number of ComponentType. The container is resized when a ComponentType is
	 * requested for an entity and it does not fit
	 */
	std::vector<ComponentType> m_storage;


};

}
