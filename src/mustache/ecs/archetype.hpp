#pragma once

#include <mustache/ecs/chunk.hpp>
#include <mustache/ecs/entity.hpp>
#include <mustache/ecs/id_deff.hpp>
#include <mustache/ecs/job_arg_parcer.hpp>
#include <mustache/ecs/archetype_operation_helper.hpp>
#include <mustache/ecs/component_factory.hpp>
#include <mustache/ecs/entity_group.hpp>
#include <mustache/utils/uncopiable.hpp>
#include <mustache/utils/type_info.hpp>
#include <cstdint>
#include <string>
#include <stdexcept>

namespace mustache {

    class World;
    class EntityManager;
    /// Location in world's entity manager
    struct EntityLocationInWorld { // TODO: add version here?
        enum : uint32_t {
            kDefaultArchetype = static_cast<uint32_t>(-1)
        };
        EntityLocationInWorld() = default;
        EntityLocationInWorld(ArchetypeEntityIndex i, ArchetypeIndex arch ) noexcept :
            index{i},
            archetype{arch} {

        }
        ArchetypeEntityIndex index = ArchetypeEntityIndex::make(0u);
        ArchetypeIndex archetype = ArchetypeIndex::make(kDefaultArchetype);
    };

    /**
     * Stores Entities with same component set
     */
    class Archetype : public Uncopiable {
    public:
        Archetype(World& world, ArchetypeIndex id, const ComponentMask& mask);

        [[nodiscard]] Entity create();

        /// Entity must belong to default(empty) archetype
        ArchetypeEntityIndex insert(Entity entity);

        void clear();

        [[nodiscard]] EntityGroup createGroup(size_t count);

        [[nodiscard]] uint32_t size() const noexcept {
            return size_;
        }
        [[nodiscard]] uint32_t capacity() const noexcept {
            return capacity_;
        }
        [[nodiscard]] ArchetypeIndex id() const noexcept {
            return id_;
        }

        void reserve(size_t capacity);

        bool isMatch(const ComponentMask& mask) const noexcept {
            return mask_.isMatch(mask);
        }

        bool hasComponent(ComponentId component_id) const noexcept {
            return mask_.has(component_id);
        }
        template<typename T>
        bool hasComponent() const noexcept {
            static const auto component_id = ComponentFactory::registerComponent<T>();
            return mask_.has(component_id);
        }
        template<FunctionSafety _Safety = FunctionSafety::kDefault>
        Chunk* getChunk(uint32_t chunk_index) const noexcept { // TODO: use ChunkIndex
            if constexpr (isSafe(_Safety)) {
                if (chunk_index >= chunks_.size()) {
                    return nullptr;
                }
            }
            return chunks_[chunk_index];
        }
        size_t chunkCount() const noexcept {
            return chunks_.size();
        }
        const ArchetypeOperationHelper& operations() const noexcept {
            return operation_helper_;
        }
        MUSTACHE_INLINE ArchetypeInternalEntityLocation entityIndexToInternalLocation(ArchetypeEntityIndex index) const noexcept {
            const auto capacity = operation_helper_.capacity;
            return ArchetypeInternalEntityLocation {
                    chunks_[index.toInt() / capacity],
                    ChunkEntityIndex::make(index.toInt() % capacity)
            };
        }

        uint32_t worldVersion() const noexcept;

    private:
        friend class EntityManager;

        MUSTACHE_INLINE ComponentIndex componentIndex(ComponentId id) const noexcept {
            auto result = ComponentIndex::null();
            if (operation_helper_.component_id_to_component_index.size() > id.toInt()) {
                result = operation_helper_.component_id_to_component_index[id.toInt()];
            }
            return result;
        }
        template<FunctionSafety _Safety = FunctionSafety::kDefault>
        MUSTACHE_INLINE void* getComponentFromArchetype(ArchetypeEntityIndex entity, ComponentIndex component) const noexcept {
            const auto location = entityIndexToInternalLocation(entity);
            return operation_helper_.getComponent<_Safety>(component, location);
        }
        template<FunctionSafety _Safety = FunctionSafety::kDefault>
        MUSTACHE_INLINE void* getComponentFromArchetype(ArchetypeEntityIndex entity, ComponentId component) const noexcept {
            return getComponentFromArchetype<_Safety>(entity, componentIndex(component));
        }
        void allocateChunk();
        World& world_;
        ComponentMask mask_;
        ArchetypeOperationHelper operation_helper_;

        std::vector<Chunk*> chunks_;
        uint32_t size_{0};
        uint32_t capacity_{0};
        ArchetypeIndex id_;
        std::string name_;
    };
}
