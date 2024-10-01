#pragma once

#include <cstdint>
#include <limits>

namespace secs
{

    typedef uint32_t eid_t;

    /**
     * @brief The Entity class represents an entity in the ECS (Entity Component System).
     */
    class Entity {
    public:
        using eid_t = uint64_t;
        static constexpr eid_t InvalidEntity = std::numeric_limits<eid_t>::max();

        /**
         * @brief Constructs an Entity with a given ID.
         * @param id The ID of the entity.
         */
        Entity(eid_t id);

        /**
         * @brief Constructs an invalid Entity.
         */
        Entity();

        /**
         * @brief Gets the ID of the entity.
         * @return The ID of the entity.
         */
        eid_t GetEID() const;

        /**
         * @brief Checks if the entity is valid.
         * @return True if the entity is valid, false otherwise.
         */
        bool IsValid() const;

        /**
         * @brief Invalidates the entity.
         */
        void Invalidate();

        /**
         * @brief Equality operator for comparing two entities.
         * @param other The other entity to compare with.
         * @return True if the entities are equal, false otherwise.
         */
        bool operator==(const Entity& other);

        /**
         * @brief Less-than operator for comparing two entities.
         * @param other The other entity to compare with.
         * @return True if this entity is less than the other entity, false otherwise.
         */
        bool operator<(const Entity& other) const;

    private:
        eid_t m_eid;

    };

} // namespace secs