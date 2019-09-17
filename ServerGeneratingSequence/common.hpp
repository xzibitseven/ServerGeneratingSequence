#ifndef COMMON_HPP
#define COMMON_HPP

#include <shared_mutex>
#include <array>
#include <unordered_map>

namespace sgs {

    namespace constants {
        constexpr std::uint8_t numberSubSequence{3};
    }

    struct SubSequence {
        std::uint8_t flag;
        std::uint16_t initial;
        std::uint16_t step;
        std::uint64_t part;
    };

    using Sequence = std::array<SubSequence,constants::numberSubSequence>;
    using storage = std::unordered_map<std::int32_t, Sequence>;

#if (__cplusplus == 201402L)
     using SharedMutex = std::shared_timed_mutex;
#elif(__cplusplus == 201703L)
     using SharedMutex = std::shared_mutex;
#endif

} /* sgs */

#endif // COMMON_HPP
