#pragma once

#include <vector>
#include <memory>

#include "componentstorage.h"
#include "componenttraits.h"

namespace secs {

class ComponentManager
{
public:

	template <typename ComponentType>
	typename ComponentStorage<ComponentType>::SharedPtr componentStorage()
	{
		typename ComponentStorage<ComponentType>::SharedPtr component_storage;
		auto component_index = ComponentTraits::getIndex<ComponentType>();
		if( component_index > m_componentStorageVector.size() )
		{
			auto base_storage = std::shared_ptr<BaseComponentStorage>(new ComponentStorage<ComponentType>());
			component_storage = std::static_pointer_cast<ComponentStorage<ComponentType>>(base_storage);
			m_componentStorageVector.resize( component_index );
			m_componentStorageVector[component_index] = base_storage;
		}
		else
		{
			auto base_storage = m_componentStorageVector[component_index];
			component_storage = std::static_pointer_cast<ComponentStorage<ComponentType>>(base_storage);
		}
		return component_storage;
	}

private:
	/**
	 * @brief m_componentStorageVector
	 */
	std::vector<BaseComponentStorage::SharedPtr> m_componentStorageVector;

};

}
