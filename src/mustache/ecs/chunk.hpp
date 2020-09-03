#pragma once

#include <mustache/utils/default_settings.hpp>
#include <mustache/ecs/id_deff.hpp>
#include <cstdint>
#include <cstddef>
#include <array>

namespace mustache {

    class Chunk {
    public:
        Chunk() = default;
        Chunk(const Chunk&) = delete;
        enum : uint32_t {
            kChunkSize = 1024 * 1024
        };
        [[nodiscard]] MUSTACHE_INLINE std::byte* data() noexcept {
            return data_.data();
        }
        [[nodiscard]] MUSTACHE_INLINE const std::byte* data() const noexcept {
            return data_.data();
        }
        template <typename T = void>
        [[nodiscard]] MUSTACHE_INLINE T* dataPointerWithOffset(ComponentOffset offset) noexcept {
            return reinterpret_cast<T*>(offset.apply(data_.data()));
        }
        template <typename T = void>
        [[nodiscard]] MUSTACHE_INLINE const T* dataPointerWithOffset(ComponentOffset offset) const noexcept {
            return reinterpret_cast<const T*>(offset.apply(data_.data()));
        }

        [[nodiscard]] MUSTACHE_INLINE uint32_t componentVersion(ComponentIndex index) const noexcept {
            return dataPointerWithOffset<uint32_t>(ComponentOffset::make(0u))[index.toInt()];
        }

        MUSTACHE_INLINE void updateVersion(ComponentIndex index, uint32_t version) noexcept {
            if(index.isValid()) {
                dataPointerWithOffset<uint32_t>(ComponentOffset::make(0u))[index.toInt()] = version;
            }
        }

        /*
        MUSTACHE_INLINE void updateVersion(const ChunkStruct& chunk_struct, uint32_t version) noexcept {
            for(uint32_t i = 0; i <  chunk_struct.numComponents(); ++i) {
                updateVersion(ComponentIndex::make(i), version);
            }
        }

        [[nodiscard]] MUSTACHE_INLINE uint32_t version(const ChunkStruct& chunk_struct) const noexcept {
            uint32_t max = 0;
            for(uint32_t i = 0; i < chunk_struct.numComponents(); ++i) {
                const uint32_t version = componentVersion(ComponentIndex::make(i));
                if(max < version) {
                    max = version;
                }
            }
            return max;
        }
         */
    private:
        std::array<std::byte, kChunkSize> data_;
    };
}
