#ifndef CACHEHOUND_CLI_PLACEMENT_AFFINE_PLACEMENT_REVERSER_HPP
#define CACHEHOUND_CLI_PLACEMENT_AFFINE_PLACEMENT_REVERSER_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "./bit.hpp"
#include "./affine_solver.hpp"
#include "./affinely_independent.hpp"

#include <cachehound/cachehound.hpp>
#include <spdlog/spdlog.h>

namespace cachehound::cli {

class affine_placement_reverser {
    std::uint8_t offset_bits_;
    std::uint8_t index_bits_;
    std::vector<std::uintptr_t> addresses_;
    std::vector<std::size_t> sets_;

    template<typename Range>
    static decltype(auto) uints_to_bit_vectors(Range&& uint_range, std::size_t offset, std::size_t bits) {
        assert(offset + bits <= 8 * sizeof(std::ranges::range_value_t<Range>));

        return std::forward<Range>(uint_range)
            | std::views::transform([=](auto num){
                num >>= offset;
                Eigen::Vector<bit, Eigen::Dynamic> vec(bits);
                for(std::size_t b = 0; b < bits; b++) {
                    vec(bits - b - 1) = num & 0b1;
                    num >>= 1;
                }
                return vec;
            });
    }

    template<std::unsigned_integral T>
    static T bit_vector_to_uint(Eigen::Vector<bit, Eigen::Dynamic> vec, std::size_t offset) {
        T value{};
        for(std::size_t i = 0; i < vec.size(); i++) {
            bit b = vec(i);
            value <<= 1;
            value |= (int(b) & 0b1);
        }
        value <<= offset;
        return value;
    }

    template<std::unsigned_integral T, typename Range>
        requires (std::ranges::input_range<Range> and std::convertible_to<std::ranges::range_value_t<Range>, Eigen::Vector<bit, Eigen::Dynamic>>)
    static decltype(auto) bit_vectors_to_uints(Range&& bit_vectors, std::size_t offset) {
        return bit_vectors | std::views::transform([=](const auto& row){
            return bit_vector_to_uint<T>(row, offset);
        });
    }

public:
    affine_placement_reverser(std::uint8_t offset_bits, std::uint8_t index_bits)
        : offset_bits_(offset_bits)
        , index_bits_(index_bits) {}

    int fail = 0;

    bool feed_mapping(std::uintptr_t aligned_address, std::size_t set) {
        assert((aligned_address & ((1 << offset_bits_) - 1)) == 0);

        if(addresses_.empty()) {
            if(((aligned_address >> offset_bits_) & 0b11) != 0) {
                spdlog::info("First address: {:#x}", aligned_address);
                addresses_.emplace_back(aligned_address);
                sets_.emplace_back(set);
            }
            return false;
        }

        addresses_.emplace_back(aligned_address);
        if(
            (
                addresses_.size() == 2 &&
                !linearly_independent<bit>(uints_to_bit_vectors(addresses_, offset_bits_, addresses_.size()))
            ) ||
            (
                addresses_.size() > 2 &&
                !affinely_independent<bit>(uints_to_bit_vectors(addresses_, offset_bits_, addresses_.size() - 1))
            )
        ) {
            addresses_.pop_back();
            fail++;
            if(fail == 100'000) {
        spdlog::error("{:#x}", fmt::join(addresses_, ", "));
                assert(!"Boom");
            }
            return false;
        }
        if(addresses_.size() == 2)
            spdlog::info("Second address: {:#x}", aligned_address);
        sets_.emplace_back(set);
        return true;
    }

    std::size_t reversed_bits() const {
        if(addresses_.size() <= 2) return 0;
        return addresses_.size() - 1;
    }

    affine_placement_policy reverse() const {
        assert(reversed_bits() > 0);

        spdlog::info("Addresses: {:#x}", fmt::join(addresses_, ", "));
        spdlog::info("Sets: {:#x}", fmt::join(sets_, ", "));

        affine_solver<bit> solver{reversed_bits(), index_bits_};

        auto&& inputs = uints_to_bit_vectors(addresses_, offset_bits_, reversed_bits());
        assert(affinely_independent<bit>(inputs));

        auto&& outputs = uints_to_bit_vectors(sets_, 0, index_bits_);

        auto [A, t] = solver.solve(inputs, outputs);

        // Adjusted matrix/vector for offset bits
        auto A_adjusted = A;
        auto t_adjusted = t;
        A_adjusted.resize(A.rows() + offset_bits_, A.cols() + offset_bits_);
        A_adjusted.setZero();
        A_adjusted.block(0, 0, A.rows(), A.cols()) = A;
        t_adjusted.resize(t.size() + offset_bits_);
        t_adjusted.setZero();
        t_adjusted.block(0, 0, t.rows(), t.cols()) = t;
    
        return affine_placement_policy(
            bit_vectors_to_uints<std::uintptr_t>(A_adjusted.rowwise(), 0),
            bit_vector_to_uint<std::size_t>(t_adjusted, 0),
            offset_bits_
        );
    }
};

}

#endif /* CACHEHOUND_CLI_PLACEMENT_AFFINE_PLACEMENT_REVERSER_HPP */
