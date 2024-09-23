#ifndef CACHEHOUND_POLICIES_PLACEMENT_AFFINE_PLACEMENT_POLICY_HPP
#define CACHEHOUND_POLICIES_PLACEMENT_AFFINE_PLACEMENT_POLICY_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <vector>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "../../concepts/placement_policy.hpp"

namespace cachehound {

class affine_placement_policy {
    std::vector<std::uintptr_t> matrix_;
    std::size_t vector_;
    std::uint8_t offset_bits_;

public:
    affine_placement_policy(std::ranges::input_range auto&& matrix, std::size_t vector, std::uint8_t offset_bits)
        : matrix_(std::forward<decltype(matrix)>(matrix).begin(), std::forward<decltype(matrix)>(matrix).end())
        , vector_(vector)
        , offset_bits_(offset_bits) {
            assert(vector_ == (vector_ & ((1 << matrix_.size()) - 1)));
        }

    std::uint8_t index_bits() const noexcept {
        return matrix_.size() - offset_bits_;
    }

    std::size_t operator()(std::uintptr_t aligned_address) const noexcept {
        std::size_t set = 0;
        for(std::uintptr_t row : matrix_) {
            set <<= 1;
            set |= (std::popcount(row & aligned_address) % 2);
        }
        return (set ^ vector_) >> offset_bits_;
    }

    std::string matrix_string() const {
        return fmt::format("[{:#b}]", fmt::join(matrix_, ", "));
    }

    std::string vector_string() const {
        return fmt::format("{:#b}", vector_);
    }

    std::string to_string() const {
        auto find_groups = [&](){
            std::size_t rows = matrix_.size();
            std::size_t cols = 8 * sizeof(std::uintptr_t);

            auto read_matrix = [&](std::size_t r, std::size_t c) -> bool {
                return (matrix_[r] >> (cols - c - 1)) & 0b1;
            };

            std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> groups;

            for(std::size_t r = 0; r < rows; r++) {
                for(std::size_t c = 0; c < cols; c++) {
                    if(read_matrix(r, c)) {
                        // Check if this is the start of a diagonal
                        if (r == 0 || c == 0 || !read_matrix(r - 1, c - 1)) {
                            // Trace the diagonal
                            std::size_t length = 1;
                            std::size_t temp_r = r, temp_c = c;
                            while (temp_r < rows && temp_c < cols && read_matrix(temp_r, temp_c)) {
                                length++;
                                temp_r++;
                                temp_c++;
                            }
                            // Shift, First bit (inclusive), Last bit (exclusive)
                            // groups.append((64 - temp_c + length - 1, 64 - temp_c, 10 - temp_r))
                            groups.emplace_back(index_bits() - temp_r, cols - temp_c + length - 1, cols - temp_c);
                        }
                    }
                }
            }

            return groups;
        };

        auto groups = find_groups();

        // Order by shift descending
        std::sort(groups.rbegin(), groups.rend());

        std::vector<std::string> group_strings;
        std::string current_group_string{};
        for(std::size_t i = 0; i < groups.size(); i++) {
            if(i == 0 || std::get<0>(groups[i]) == std::get<0>(groups[i - 1])) {
                if(!current_group_string.empty()) {
                    current_group_string += " ^ ";
                }
            } else {
                if(std::get<0>(groups[i - 1]) != 0) {
                    current_group_string = fmt::format("(({}) << {})", current_group_string, std::get<0>(groups[i - 1]));
                } else {
                    current_group_string = fmt::format("({})", current_group_string);
                }
                group_strings.emplace_back(std::move(current_group_string));
                current_group_string = "";
            }
            current_group_string += fmt::format("a[{}:{}]", std::get<1>(groups[i]), std::get<2>(groups[i]));
        }
        if(std::get<0>(groups.back()) != 0) {
            current_group_string = fmt::format("(({}) << {})", current_group_string, std::get<0>(groups.back()));
        } else {
            current_group_string = fmt::format("({})", current_group_string);
        }
        group_strings.emplace_back(std::move(current_group_string));

        std::string extra = "";
        if(vector_) {
            extra += fmt::format(" ^ {:#b}", vector_ >> offset_bits_);
        }
        return fmt::format("I(a) = {}{}", fmt::join(group_strings, " ^ "), extra);
    }
};
static_assert(placement_policy<affine_placement_policy>);

}

#endif /* CACHEHOUND_POLICIES_PLACEMENT_AFFINE_PLACEMENT_POLICY_HPP */
