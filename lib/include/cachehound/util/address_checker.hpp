#ifndef CACHEHOUND_UTIL_ADDRESS_CHECKER_HPP
#define CACHEHOUND_UTIL_ADDRESS_CHECKER_HPP

#include "../concepts/memory_region_range.hpp"

#include <cassert>
#include <map>

namespace cachehound {

class address_checker {
    std::map<std::uintptr_t, std::size_t> regions_;

public:
    void add_region(memory_region auto const& region) {
        regions_.emplace(region.base(), region.size());
    }

    bool operator()(std::uintptr_t address) {
        auto it = regions_.upper_bound(address);
        if(it == regions_.begin()) return false;
        std::advance(it, -1);
        assert(address >= it->first);
        return address < (it->first + it->second);
    }
};

}

#endif /* CACHEHOUND_UTIL_ADDRESS_CHECKER_HPP */
