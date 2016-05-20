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

	virtual EntityBits& ownerEntityBits() = 0 ;
	virtual void unsetComponent( const Entity& e ) = 0 ;
	virtual void setComponent( const Entity& e ) = 0 ;

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
	 * @brief sets the entity bit for ComponentType
	 * @param e
	 */
	void setComponent( const Entity& e ) override
	{
		m_ownerEntities.set(e.eid(), true);
	}

	/**
	 * @brief replaces old ComponentType associated to the entity with a fresh one
	 * @param e
	 */
	void freeComponent( const Entity& e )
	{
		assert( e.eid() < m_storage.size() );
		assert( true == m_ownerEntities.test( e.eid() ) );
		m_storage[ e.eid() ] = {};
	}

	/**
	 * @brief clears the entity bit for ComponentType
	 * @param e
	 */
	void unsetComponent( const Entity& e ) override
	{
		m_ownerEntities.set(e.eid(), false);
	}

	ComponentType& component( Entity& e )
	{
		assert( m_ownerEntities.test(e.eid())
				&& "Must be used in this component storage." );
		return m_storage[e.eid()];
	}

	bool hasComponent( const Entity& entity )
	{
		return m_ownerEntities.test( entity.eid() );
	}

private:

	/**
	 * @brief m_storage number of ComponentType. The container is resized when a ComponentType is
	 * requested for an entity and it does not fit
	 */
	std::vector<ComponentType> m_storage;

	/**
	 * @brief m_ownerEntities is a bitset that keeps track of which entities
	 * do have a ComponentType and which not.
	 */
	EntityBits m_ownerEntities;

};

}
