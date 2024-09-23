#include <cassert>
#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>

#include <cachehound/cachehound.hpp>

using json = nlohmann::json;

using namespace cachehound;

TEST_CASE("Replacement policies") {
    json data;
    {
        // Origin: https://uops.info/cache/replPolicy_ICL_L3_0.html
        std::ifstream fd{"../../lib/test/data/replacement.json"};
        assert(fd);
        data = json::parse(fd);
    }
    assert(data.is_array());

    using sequence = std::vector<std::size_t>;
    std::vector<sequence> sequences;
    sequences.reserve(data.size());

    // associativity, sequence ref, expected hit_counter
    using test_case = std::tuple<std::size_t, const sequence&, std::size_t>;
    std::unordered_map<std::string, std::vector<test_case>> tests;

    for(auto item : data) {
        assert(item.contains("sequence"));
        assert(item["sequence"].is_array());
        sequence seq;
        seq.reserve(item["sequence"].size());
        for(auto addr : item["sequence"]) {
            assert(addr.is_number_unsigned());
            seq.push_back(addr);
        }
        sequences.push_back(std::move(seq));

        assert(item.contains("associativity"));
        assert(item["associativity"].is_number_unsigned());
        std::size_t associativity = item["associativity"];

        assert(item.contains("hit_counters"));
        assert(item["hit_counters"].is_object());
        for(auto kv : item["hit_counters"].items()) {
            const auto& policy_name = kv.key();
            const auto hit_counter = kv.value();
            tests[policy_name].emplace_back(associativity, sequences.back(), hit_counter);
        }
    }

    for(auto& [policy_name, test_cases] : tests) {
        SECTION(policy_name) {
            for(const auto& test_case : test_cases) {
                std::size_t associativity = std::get<0>(test_case);
                auto& seq = std::get<1>(test_case);
                std::size_t expected = std::get<2>(test_case);

                auto pol = make_replacement_policy(policy_name, associativity);
                if(!pol) continue;
                assert(pol->ways() == associativity);

                INFO("Associativity: " << fmt::format("{}", associativity));
                INFO("Sequence: " << fmt::format("{}", fmt::join(seq, ", ")));
                CHECK(simulate_sequence(seq, *pol) == expected);
            }
        }
    }
}
