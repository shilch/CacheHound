#ifndef CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_COMMAND_HPP
#define CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_COMMAND_HPP

#include "../reverse_command.hpp"

#include "cachehound/adapters/armv8_bypass_adapter.hpp"
#include "cachehound/adapters/physical_adapter.hpp"
#include "cachehound/backends/kernel_memory.hpp"
#include "cachehound/concepts/instrumented_memory.hpp"
#include "cachehound/concepts/physically_indexable_memory.hpp"
#include "cachehound/policies/placement/modular_placement_policy.hpp"

#include <argparse/argparse.hpp>
#include <cachehound/cachehound.hpp>
#include <cachehound/sim/sim.hpp>
#include <stdexcept>

template<std::size_t MaxDepth>
int cachehound::cli::reverse_command::provide_memory(auto&& func, unsigned levels_remaining) const {
    if constexpr(MaxDepth == 0) {
        spdlog::error("Maximum bypass depth reached");
        return 1;
    } else {
        if(levels_remaining > 0) {
            return provide_memory<MaxDepth - 1>([func = std::forward<decltype(func)>(func)](instrumented_memory auto& memory) mutable -> int {
                // TODO: Assuming modular placement policy for now
                modular_placement_policy placement{memory.offset_bits(), 1ULL << memory.index_bits(0)};
                if constexpr(armv8_memory<decltype(memory)>) {
                    armv8_bypass_adapter adapter{memory, placement};
                    return std::forward<decltype(func)>(func)(adapter);
                } else {
                    bypass_adapter adapter{memory, placement};
                    return std::forward<decltype(func)>(func)(adapter);
                }
            }, levels_remaining - 1);
        } else {
            if(simulate_) {
                static constexpr std::size_t ways = 4;
                static constexpr std::size_t index_bits = 6;
                static constexpr std::size_t offset_bits = 6;

                using namespace cachehound::sim;

                sim::simulated_memory memory {
                    0x0,
                    memory_size_,
                    offset_bits,
                    sim::set_associative_cache {
                        sim::basic_set_associative_policy {
                            index_bits,
                            modular_placement_policy{offset_bits, 1 << index_bits},
                            // generic_placement_policy{1 << index_bits, [](std::uintptr_t address){
                            //     return ((address >> 6) % 64) ^ ((address >> 12) & 0b1100);
                            // }},
                            lru_replacement_policy{ways}
                        }
                    },
                    sim::set_associative_cache {
                        sim::basic_set_associative_policy {
                            1 + index_bits,
                            modular_placement_policy{offset_bits, 1 << (1 + index_bits)},
                            lru_replacement_policy{2 * ways}
                        }
                    }
                };
                return std::forward<decltype(func)>(func)(memory);
            } else {
                kernel_memory memory{memory_size_, cpu_, pmu_events_, pmu_handler_, isolation_};
                return std::forward<decltype(func)>(func)(memory);
            }
        }
    }
}

int cachehound::cli::reverse_command::adapt_indexing(memory auto& memory, auto&& func) const {
    if constexpr(physically_indexable_memory<decltype(memory)>) {
        if(physical_) {
            physical_adapter adapter{memory};
            return std::forward<decltype(func)>(func)(adapter);
        }
    } else if(physical_) {
        spdlog::error("This type of memory does not support physical indexing");
        return 1;
    }
    return std::forward<decltype(func)>(func)(memory);
}


void cachehound::cli::reverse_command::reverse_command::setup_arguments(argparse::ArgumentParser& args) {
    multicommand_base::setup_arguments(args);

    args.add_description("Main reverse-engineering routines of CacheHound");

    args.add_argument("--level", "-L")
        .help("Cache level to reverse")
        .default_value(unsigned{1})
        .scan<'d', unsigned>();
    args.add_argument("--physical")
        .help("Use physical instead of virtual indexing")
        .default_value(false)
        .implicit_value(true);
    args.add_argument("--memory-size", "-m")
        .help("Specify the memory size")
        .default_value(std::size_t{1} << 20)
        .scan<'d', std::size_t>();
    auto& backend = args.add_mutually_exclusive_group();
    args.add_argument("--kernel")
        .help("Allocate memory in kernel space (default)")
        .flag();
    args.add_argument("--kernel-cpu")
        .help("Specify the operating CPU when allocating memory in the kernel (using the --kernel flag)")
        .scan<'d', unsigned>();
    args.add_argument("--kernel-isolation")
        .help("Specify the isolation level when allocating memory in the kernel (using the --kernel flag)")
        .default_value("no-preempt")
        .choices("off", "no-preempt", "disable-irq");
    backend.add_argument("--simulate")
        .help("Simulate a cache hierarchy (useful for testing purposes)")
        .flag();

    // TODO: Generalize this
    args.add_argument("--pmu")
        .help("Specify the method of utilizing the PMU in the kernel memory")
        .choices("intel", "amd-zen2", "rpi5", "a64fx");
}

void cachehound::cli::reverse_command::parse_arguments(argparse::ArgumentParser& args) {
    multicommand_base::parse_arguments(args);

    if(uses_subcommand()) {
        level_ = args.get<unsigned>("--level");
        if(level_ == 0) {
            throw std::runtime_error("--level/-L must be greater than zero");
        }
        level_--;

        physical_ = args.get<bool>("--physical");

        memory_size_ = args.get<std::size_t>("--memory-size");

        simulate_ = args.get<bool>("--simulate");

        if(!simulate_) {
            // Read kernel CPU core
            cpu_ = args.get<unsigned>("--kernel-cpu");

            // Read isolation level
            auto isolation_str = args.get<std::string>("--kernel-isolation");
            if(isolation_str == "off") isolation_ = kernel_memory::isolation_level::off;
            else if(isolation_str == "no-preempt") isolation_ = kernel_memory::isolation_level::no_preempt;
            else isolation_ = kernel_memory::isolation_level::disable_irq;

            // PMU

            auto add_x86_pmu_event = [this](
                unsigned char event_select,
                unsigned char unit_mask,
                unsigned char counter_mask,
                bool edge_detect
            ){
                unsigned long val = 0;

                val |= (unsigned long)(event_select);
                val |= (((unsigned long)unit_mask) << 8);
                // user mode = 1
                val |= (1UL << 16);
                // os mode = 1
                val |= (1UL << 17);
                val |= (((unsigned long)edge_detect) << 18);
                // enable = 1
                val |= (1UL << 22);
                val |= (((unsigned long)counter_mask) << 24);

                pmu_events_.push_back(val);
            };

            auto pmu_str = args.get<std::string>("--pmu");
            if (pmu_str == "intel") {
                add_x86_pmu_event(0xD1, 0x01, 0x00, true);
                add_x86_pmu_event(0xD1, 0x02, 0x00, true);
                add_x86_pmu_event(0xD1, 0x04, 0x00, true);

                pmu_handler_ = [](
                    std::uint64_t l1_hits_before, std::uint64_t l2_hits_before, std::uint64_t l3_hits_before,
                    std::uint64_t l1_hits_after, std::uint64_t l2_hits_after, std::uint64_t l3_hits_after
                ) {
                    auto l1_hits = l1_hits_after - l1_hits_before;
                    auto l2_hits = l2_hits_after - l2_hits_before;
                    auto l3_hits = l3_hits_after - l3_hits_before;

                    // Note: Any inconsistencies in hit counters can probably be explained by interrupts during execution.
                    // Thus, try setting a higher isolation level.
                    // if(l1_hits > 1)
                    //     throw std::runtime_error(std::string{"Too many L1 hits per access: " + std::to_string(l1_hits)});
                    // if(l2_hits > 1)
                    //     throw std::runtime_error(std::string{"Too many L2 hits per access: " + std::to_string(l2_hits)});
                    // if(l3_hits > 1)
                    //     throw std::runtime_error(std::string{"Too many L3 hits per access: " + std::to_string(l3_hits)});

                    // if(l1_hits + l2_hits + l3_hits > 1)
                    //     throw std::runtime_error(std::string{"Sum of hit counters >1: L1=" + std::to_string(l1_hits) + ", L2=" + std::to_string(l2_hits) + ", L3=" + std::to_string(l3_hits)});

                    if(l1_hits) return 0;
                    if(l2_hits) return 1;
                    if(l3_hits) return 2;
                    return 3; // memory response
                };
            } else if(pmu_str == "amd-zen2") {
                add_x86_pmu_event(0x60, 0xC8, 0x00, false); // l2_cache_accesses_from_dc_misses -> L1D Misses
                add_x86_pmu_event(0x64, 0x40, 0x00, false); // l2_cache_req_stat.ls_rd_blk_l_hit_x -> L2 Hit

                pmu_handler_ = [](
                    std::uint64_t l1_misses_before, std::uint64_t l2_hits_before, std::uint64_t,
                    std::uint64_t l1_misses_after, std::uint64_t l2_hits_after, std::uint64_t
                ) {
                    auto l1_misses = l1_misses_after - l1_misses_before;
                    auto l2_hits = l2_hits_after - l2_hits_before;

                    if(l1_misses == 0) return 0;
                    if(l2_hits) return 1;
                    return 2; // memory response
                };
            } else if(pmu_str == "rpi5") {
                // arm64 architectural standard events
                pmu_events_.push_back(0x42); // L1D_CACHE_REFILL_RD
                pmu_events_.push_back(0x52); // L2D_CACHE_REFILL_RD
                pmu_events_.push_back(0x2A); // L3D_CACHE_REFILL
                // pmu_events_.push_back(0xA2); // L3D_CACHE_REFILL_RD (only on DynamIQ Shared Unit)
                // pmu_events_.push_back(0x37); // LL_CACHE_MISS_RD

                pmu_handler_ = [](
                    std::uint64_t l1_misses_before, std::uint64_t l2_misses_before, std::uint64_t l3_misses_before,
                    std::uint64_t l1_misses_after, std::uint64_t l2_misses_after, std::uint64_t l3_misses_after
                ) {
                    auto l1_misses = l1_misses_after - l1_misses_before;
                    auto l2_misses = l2_misses_after - l2_misses_before;
                    auto l3_misses = l3_misses_after - l3_misses_before;

                    // spdlog::info("{} {} {}", l1_misses, l2_misses, l3_misses);

                    if (l3_misses)
                        return 3;
                    if (l2_misses)
                        return 2;
                    if (l1_misses)
                        return 1;
                    return 0;
                };
            } else {
                assert(pmu_str == "a64fx");

                pmu_events_.push_back(0x0003); // L1D_CACHE_REFILL
                pmu_events_.push_back(0x0017); // L2D_CACHE_REFILL (or L2_MISS_COUNT 0x0309 ??)

                // NOTE: A64FX PMU Events Errata
                // 0x0017, L2D_CACHE_REFILL
                // 0x0059, L2D_CACHE_REFILL_PRF
                // 0x0300, L2D_CACHE_REFILL_DM
                // 0x0309, L2_MISS_COUNT
                // These events count more than they occur actually when the time distance between the demand request and the prefetch request is close.
                //
                // They can be corrected as follows:
                // L2D_CACHE_REFILL := L2D_CACHE_REFILL – L2D_SWAP_DM – L2D_CACHE_MIBMCH_PRF
                // L2D_CACHE_REFILL_DM := L2D_CACHE_REFILL_DM - L2D_SWAP_DM
                // L2D_CACHE_REFILL_PRF := L2D_CACHE_REFILL_PRF - L2D_CACHE_MIBMCH_PRF 
                // L2_MISS_COUNT := L2_MISS_COUNT - L2D_CACHE_SWAP_LOCAL - L2_PIPE_COMP_PF_L2MIB_MCH

                pmu_handler_ = [](
                    std::uint64_t l1_misses_before, std::uint64_t l2_misses_before, std::uint64_t,
                    std::uint64_t l1_misses_after, std::uint64_t l2_misses_after, std::uint64_t
                ) {
                    auto l1_misses = l1_misses_after - l1_misses_before;
                    auto l2_misses = l2_misses_after - l2_misses_before;

                    if (l2_misses)
                        return 2;
                    if (l1_misses)
                        return 1;
                    return 0;
                };
            }
        }
    }
}

int cachehound::cli::reverse_command::run_subcommand(auto &subcommand) const {
    return provide_memory([&](instrumented_memory auto& memory){
        return adapt_indexing(memory, [&](instrumented_memory auto& memory) mutable {
            spdlog::info("Allocated memory of size {}", memory_size(memory));
            size_t region_index = 0;
            for(auto& region : memory.regions()) {
                spdlog::trace("Region {} base: {:#x} (size: {})", region_index, region.base(), region.size());
                region_index++;
            }

            int res = subcommand.run(memory);

            if constexpr(stats_memory<decltype(memory)>) {
                spdlog::info("Stats:");
                auto stats = memory.stats();
                spdlog::info("Accesses:              {}", stats.accesses);
                spdlog::info("Instrumented accesses: {}", stats.instrumented_accesses);
                spdlog::info("Flushes:               {}", stats.flushes);
                spdlog::info("Channel switches:      {}", stats.channel_switches);
            }

            return res;
        });
    }, level_);
}

#endif /* CACHEHOUND_CLI_COMMANDS_IMPL_REVERSE_COMMAND_HPP */
