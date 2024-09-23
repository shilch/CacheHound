#ifndef CACHEHOUND_BACKENDS_IMPL_KERNEL_MEMORY_HPP
#define CACHEHOUND_BACKENDS_IMPL_KERNEL_MEMORY_HPP

namespace cachehound {

cachehound::kernel_memory::kernel_memory(
    std::size_t min_size,
    unsigned cpu,
    auto&& pmu_events,
    std::function<pmu_handler_type> pmu_handler,
    isolation_level isolation,
    unsigned max_order) : pmu_handler_(pmu_handler)
{
    assert(min_size > 0);

    std::vector<std::uint64_t> pmu_events_vec(std::forward<decltype(pmu_events)>(pmu_events));
    assert(pmu_events_vec.size() <= 3);

    auto page_size = sysconf(_SC_PAGE_SIZE);

    auto fd = obtain_ch_fd();
    try {
        unsigned order = max_order;

        while (min_size) {
            // auto pages_required = (min_size + page_size - 1) / page_size;
            // unsigned order = std::countr_zero(std::bit_ceil(pages_required));

            ch_ioc_alloc_config config {
                .order = order
            };
            if(!alloc_region(fd, config)){
                // failed to allocate 2^order pages
                // Try to decrement order
                if(order == 0) {
                    throw std::runtime_error("Failed to allocate memory");
                }
                order--;
                continue;
            }

            std::size_t size = (1 << order) * page_size;
            regions_.emplace_back(config.virtual_base, config.physical_base, size);
            min_size -= std::min(min_size, size);
        }
        defragment_regions();

        for(auto& region : regions_) {
            address_checker_.add_region(region);
        }

        read_cache_info(fd, cache_info_);

        ch_ioc_start_config config {
            .cpu = cpu,
            .isolation_level = static_cast<std::underlying_type_t<isolation_level>>(isolation)
        };
        std::copy(pmu_events.begin(), pmu_events.end(), config.evts);
        start_agent(fd, config);
        channels_ = mmap_channels(fd, page_size);
        active_channel_ = config.active_channel;
        channel_count_ = config.channel_count;
        close(fd);
    } catch (std::exception& ex) {
        close(fd);
        throw ex;
    }
}



}

#endif
