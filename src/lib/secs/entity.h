#pragma once

#include <cstdint>
#include <limits>

namespace secs
{

    typedef uint32_t eid_t;

    /**
     * @brief The Entity class represents an entity in the ECS (Entity Component System).
     *
     * An entity handle packs two 32-bit values into one 64-bit id:
     *   - Index():      the slot used to address component storage and flags. Indices
     *                   are recycled when entities are removed.
     *   - Generation(): incremented every time the index is recycled, so a handle to a
     *                   removed entity never compares equal to the entity that later
     *                   reuses its index. Use EntityProcessor::IsAlive() to check a handle.
     */
    class Entity {
    public:
        using eid_t = uint64_t;
        using index_t = uint32_t;
        using generation_t = uint32_t;

        static constexpr eid_t InvalidEntity = std::numeric_limits<eid_t>::max();

        /**
         * @brief Constructs an Entity from its raw 64-bit id (generation << 32 | index).
         * @param id The raw ID of the entity.
         */
        Entity(eid_t id);

        /**
         * @brief Constructs an Entity from an index and a generation.
         */
        Entity(index_t index, generation_t generation);

        /**
         * @brief Constructs an invalid Entity.
         */
        Entity();

        /**
         * @brief Gets the raw 64-bit ID of the entity (index and generation).
         * @return The raw ID of the entity.
         */
        eid_t GetEID() const;

        /**
         * @brief Gets the storage index of the entity (recycled across entities).
         */
        index_t Index() const;

        /**
         * @brief Gets the generation of the entity.
         */
        generation_t Generation() const;

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
         * @brief Equality operator for comparing two entities (index and generation).
         * @param other The other entity to compare with.
         * @return True if the entities are equal, false otherwise.
         */
        bool operator==(const Entity& other) const;

        bool operator!=(const Entity& other) const;

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
