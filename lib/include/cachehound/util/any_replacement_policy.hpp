#ifndef CACHEHOUND_UTIL_ANY_REPLACEMENT_POLICY_HPP
#define CACHEHOUND_UTIL_ANY_REPLACEMENT_POLICY_HPP

#include <cstddef>
#include <concepts>
#include <memory>

#include "../concepts/replacement_policy.hpp"

namespace cachehound {

class any_replacement_policy {
    struct base_class {
        virtual ~base_class() = default;
        virtual std::size_t ways() const noexcept = 0;
        virtual void hit(std::size_t way) = 0;
        virtual std::size_t miss() = 0;

        virtual std::unique_ptr<base_class> copy() const = 0;
    };

    template<replacement_policy Policy>
        requires (std::copy_constructible<Policy> and std::move_constructible<Policy>)
    class impl_class : public base_class {
        Policy policy_;

    public:
        impl_class(const Policy& policy) : policy_(policy) {}
        impl_class(Policy&& policy) : policy_(std::move(policy)) {}

        std::size_t ways() const noexcept override {
            return policy_.ways();
        }

        void hit(std::size_t way) override {
            policy_.hit(way);
        }

        std::size_t miss() override {
            return policy_.miss();
        }

        std::unique_ptr<base_class> copy() const override {
            return std::make_unique<impl_class>(policy_);
        }
    };

    std::unique_ptr<base_class> base_;

public:
    any_replacement_policy(const any_replacement_policy& other)
        : base_(other.base_->copy()) {}

    any_replacement_policy(any_replacement_policy&&) = default;

    template<replacement_policy Policy>
        requires (not std::same_as<Policy, any_replacement_policy>)
    any_replacement_policy(Policy policy)
        : base_(std::make_unique<impl_class<Policy>>(std::move(policy))) {}

    std::size_t ways() const noexcept {
        return base_->ways();
    }

    void hit(std::size_t way) {
        base_->hit(way);
    }

    std::size_t miss() {
        return base_->miss();
    }
};
static_assert(replacement_policy<any_replacement_policy>);

}

#endif /* CACHEHOUND_UTIL_ANY_REPLACEMENT_POLICY_HPP */
