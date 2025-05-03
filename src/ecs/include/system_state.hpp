#pragma once
#include <vector>
#include <typeindex>
#include <string>
#include <absl/container/flat_hash_map.h>

namespace velora
{
    class SystemState 
    {
        public:
            SystemState() = default;
            ~SystemState() = default;

            SystemState(const SystemState&) = delete;
            SystemState& operator=(const SystemState&) = delete;

            template<typename T>
            T* write(std::string name, const T& value) 
            {
                static_assert(std::is_trivially_copyable_v<T>, "Only trivially copyable types supported");

                std::size_t alignment = alignof(T);
                std::size_t size = sizeof(T);

                std::size_t offset = align_up(_buffer.size(), alignment);
                if (offset + size > _buffer.size()) {
                    _buffer.resize(offset + size);
                }

                std::memcpy(_buffer.data() + offset, &value, size);
                _entries[{typeid(T), std::move(name)}] = {offset, size};

                return reinterpret_cast<T*>(_buffer.data() + offset);
            }

            template<typename T>
            T* get(std::string name) 
            {
                auto it = _entries.find({typeid(T), std::move(name)});
                if (it == _entries.end()) return nullptr;

                return reinterpret_cast<T*>(_buffer.data() + it->second.offset);
            }

            template<typename T>
            const T * get(std::string name) const
            {
                auto it = _entries.find({typeid(T), std::move(name)});
                if (it == _entries.end()) return nullptr;

                return reinterpret_cast<const T*>(_buffer.data() + it->second.offset);
            }

            void clear() {
                _buffer.clear();
                _entries.clear();
            }

    private:
        std::vector<std::byte> _buffer;

        struct Entry {
            std::size_t offset;
            std::size_t size;
        };

        absl::flat_hash_map<std::pair<std::type_index, std::string>, Entry> _entries;

        static std::size_t align_up(std::size_t offset, std::size_t alignment);
    };
}