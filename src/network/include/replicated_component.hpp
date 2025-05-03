#pragma once

#include <absl/container/flat_hash_map.h>
#include <utility>
#include <string>

namespace velora::network
{
    template<typename T>
    struct ReplicatedComponent
    {
        T value;
        std::uint64_t last_changed_tick = 0;
    };

    struct ReplicationStateComponent
    {
        absl::flat_hash_map<std::string, std::uint64_t> last_sent_ticks;
        std::uint64_t last_acked_tick = 0;
    };
}