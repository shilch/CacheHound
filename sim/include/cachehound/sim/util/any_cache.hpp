#ifndef CACHEHOUND_SIM_UTIL_ANY_CACHE_HPP
#define CACHEHOUND_SIM_UTIL_ANY_CACHE_HPP

#include <cstdint>
#include <optional>
#include <concepts>

#include "../concepts/cache.hpp"

namespace cachehound::sim {

class any_cache {
    struct base_class {
        virtual ~base_class() = default;
        virtual const void* lookup(std::uintptr_t) const noexcept = 0;
        virtual void hit(const void*) noexcept = 0;
        virtual void invalidate(const void*) noexcept = 0;
        virtual std::optional<std::uintptr_t> fill(std::uintptr_t) noexcept = 0;
    };

    template<cache Cache>
        requires std::copy_constructible<Cache>
    class impl_class : public base_class {
        Cache cache_;

    public:
        impl_class(Cache&& cache) : cache_(std::move(cache)) {}

        impl_class(const Cache& other) : cache_(other.cache_) {}

        const void* lookup(std::uintptr_t address) const noexcept override {
            return cache_.lookup(address);
        }

        void hit(const void* handle) noexcept override {
            cache_.hit(handle);
        }

        void invalidate(const void* handle) noexcept override {
            cache_.invalidate(handle);
        }

        std::optional<std::uintptr_t> fill(std::uintptr_t address) noexcept override {
            return cache_.fill(address);
        }
    };

    std::unique_ptr<base_class> base_;

public:
    template<cache Cache>
    any_cache(Cache&& cache)
        : base_(std::make_unique<impl_class<Cache>>(std::forward<decltype(cache)>(cache))) {}

    any_cache(const any_cache& other)
        : base_(other.base_.get()) {}


    const void* lookup(std::uintptr_t address) const noexcept {
        return base_->lookup(address);
    }

    void hit(const void* handle) noexcept {
        base_->hit(handle);
    }

    void invalidate(const void* handle) noexcept {
        base_->invalidate(handle);
    }

    std::optional<std::uintptr_t> fill(std::uintptr_t address) noexcept {
        return base_->fill(address);
    }
};
static_assert(cache<any_cache>);

}

#endif /* CACHEHOUND_SIM_UTIL_ANY_CACHE_HPP */
