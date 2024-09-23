#ifndef CACHEHOUND_BACKENDS_IMPL_KERNEL_MEMORY_IPP
#define CACHEHOUND_BACKENDS_IMPL_KERNEL_MEMORY_IPP

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

#include "../kernel_memory.hpp"

int cachehound::kernel_memory::obtain_ch_fd()
{
    int fd = open("/dev/" CH_DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        std::string msg = "Failed to open file /dev/" CH_DEVICE_NAME ": ";
        msg += strerror(errno);
        throw std::runtime_error(msg);
    }
    return fd;
}

ch_channel* cachehound::kernel_memory::mmap_channels(int fd, int page_size)
{
    auto channels_ptr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (channels_ptr == MAP_FAILED) {
        throw std::runtime_error("mmap of channels failed");
    }
    return reinterpret_cast<ch_channel*>(channels_ptr);
}

bool cachehound::kernel_memory::alloc_region(int fd, ch_ioc_alloc_config& config)
{
    int ret = ioctl(fd, CH_IOC_ALLOC_MEMORY, &config);
    return !(ret < 0);
}

void cachehound::kernel_memory::read_cache_info(int fd, ch_ioc_cache_info& cache_info) {
    int ret = ioctl(fd, CH_IOC_CACHE_INFO, &cache_info);
    if (ret < 0) {
        std::string msg = "Failed to read cache info: ";
        msg += strerror(errno);
        throw std::runtime_error(msg);
    }
}

void cachehound::kernel_memory::start_agent(int fd, ch_ioc_start_config& config)
{
    int ret = ioctl(fd, CH_IOC_START_AGENT, &config);
    if (ret < 0) {
        std::string msg = "Failed to start agent: ";
        msg += strerror(errno);
        throw std::runtime_error(msg);
    }
}

ch_channel* cachehound::kernel_memory::active_channel() const
{
    return channels_ + active_channel_;
}

void cachehound::kernel_memory::switch_channel(std::size_t channel) noexcept
{
    assert(channel < channel_count_);

    flush();
    ch_channel_wait_for_idle(active_channel());
    active_channel()->data[0] = channel;
    ch_channel_set_access(active_channel(), 0b1000);
    active_channel_ = channel;
    ch_channel_wait_for_idle(active_channel());

    stats_.channel_switches++;
}

void cachehound::kernel_memory::internal_access(std::uintptr_t address)
{
    buffer_[buffered_++] = address;
    if (buffered_ == buffer_.size()) {
        flush();
    }
}

void cachehound::kernel_memory::defragment_regions()
{
    if (regions_.empty())
        return;

    std::vector<region_type> old_regions;
    std::swap(regions_, old_regions);
    std::sort(old_regions.begin(), old_regions.end(), [](auto& a, auto& b) {
        return a.base() < b.base();
    });

    std::uintptr_t base = old_regions[0].base();
    std::uintptr_t physical_base = old_regions[0].physical_base();
    std::size_t size = old_regions[0].size();
    for (std::size_t i = 1; i < old_regions.size(); i++) {
        auto& region = old_regions[i];
        if (base + size == region.base() && physical_base + size == region.physical_base()) {
            size += region.size();
        } else {
            regions_.emplace_back(base, physical_base, size);
            base = region.base();
            physical_base = region.physical_base();
            size = region.size();
        }
    }
    regions_.emplace_back(base, physical_base, size);
}

cachehound::kernel_memory::~kernel_memory() noexcept
{
    flush();
    munmap(channels_, sizeof(ch_channel));
}

[[nodiscard]] const std::vector<cachehound::kernel_memory::region_type>& cachehound::kernel_memory::regions() const noexcept
{
    return regions_;
}

[[nodiscard]] unsigned cachehound::kernel_memory::levels() const noexcept
{
    return cache_info_.levels;
}

void cachehound::kernel_memory::access(std::uintptr_t address) noexcept
{
    assert((address >> (8 * sizeof(address) - reserved_upper_bits)) == ((1 << reserved_upper_bits) - 1));
    assert(address_checker_(address));
    internal_access(address);
    stats_.accesses++;
}

unsigned cachehound::kernel_memory::instrumented_access(std::uintptr_t address)
{
    assert((address >> (8 * sizeof(address) - reserved_upper_bits)) == ((1 << reserved_upper_bits) - 1));
    assert(address_checker_(address));

    buffer_[buffered_++] = address;
    flush();
    ch_channel_wait_for_idle(active_channel());

    stats_.instrumented_accesses++;

    auto evt0_before = active_channel()->data[0];
    auto evt1_before = active_channel()->data[1];
    auto evt2_before = active_channel()->data[2];
    auto evt0_after  = active_channel()->data[3];
    auto evt1_after  = active_channel()->data[4];
    auto evt2_after  = active_channel()->data[5];
    return pmu_handler_(evt0_before, evt1_before, evt2_before, evt0_after, evt1_after, evt2_after);
}

inline void cachehound::kernel_memory::flush() noexcept
{
    if (buffered_) {
        assert(buffered_ <= buffer_.size());
        ch_channel_wait_for_idle(active_channel());
        std::copy(buffer_.begin(), buffer_.begin() + buffered_, active_channel()->data);
        ch_channel_set_access(active_channel(), buffered_);
        buffered_ = 0;
        stats_.flushes++;
    }
}

cachehound::kernel_memory::stats_type cachehound::kernel_memory::stats() const noexcept {
    return stats_;
}

std::uint8_t cachehound::kernel_memory::offset_bits() const noexcept
{
    return cache_info_.offset_bits;
}

std::uint8_t cachehound::kernel_memory::index_bits(unsigned level) const noexcept
{
    assert(level < 3);
    auto sets = cache_info_.sets[level];
    assert(sets != 0);
    if((sets & (sets - 1)) != 0) {
        // Adjust for power-of-two for now
        // TODO: Rename index_bits(level) -> sets(level) to allow for non-power-of-two sets.
        --sets;
        sets |= (sets >> 1);
        sets |= (sets >> 2);
        sets |= (sets >> 4);
        sets |= (sets >> 8);
        sets |= (sets >> 16);
        // sets |= (sets >> 32);
        ++sets;
    }
    return std::countr_zero(sets);
}

std::size_t cachehound::kernel_memory::ways(unsigned level) const noexcept
{
    assert(level < 3);
    return cache_info_.ways[level];
}

#if defined(__x86_64__) || defined(_M_X64)
void cachehound::kernel_memory::clflush(std::uintptr_t address) noexcept
{
    internal_access(address & 0x7FFFFFFFFFFFFFFF);
    stats_.clflushes++;
}

void cachehound::kernel_memory::wbinvd() noexcept
{
    flush();
    ch_channel_wait_for_idle(active_channel());
    ch_channel_set_access(active_channel(), 0b10000);
    stats_.wbinvds++;
}

#elif defined(__aarch64__) || defined(_M_ARM64)
void cachehound::kernel_memory::internal_cisw(bool clean, bool invalidate, unsigned level, std::size_t set, std::size_t way)
{
    assert(clean || invalidate);
    assert(level < levels());

    auto way_bits = std::countr_zero(std::bit_ceil(ways(level)));

    std::uint64_t address = static_cast<std::uint32_t>(level) << 1;
    address |= (static_cast<std::uint32_t>(way) << (32 - way_bits));
    address |= (static_cast<std::uint32_t>(set) << offset_bits());

    address |= (static_cast<std::uint64_t>(!clean) << 63);
    address |= (static_cast<std::uint64_t>(!invalidate) << 62);

    internal_access(address);
}

void cachehound::kernel_memory::csw(unsigned level, std::size_t set, std::size_t way)
{
    internal_cisw(true, false, level, set, way);
    stats_.csws++;
}

void cachehound::kernel_memory::isw(unsigned level, std::size_t set, std::size_t way)
{
    internal_cisw(false, true, level, set, way);
    stats_.isws++;
}

void cachehound::kernel_memory::cisw(unsigned level, std::size_t set, std::size_t way)
{
    internal_cisw(true, true, level, set, way);
    stats_.cisws++;
}

void cachehound::kernel_memory::reset()
{
    for (unsigned level = 0; level < levels(); level++) {
        for (unsigned set = 0; set < (1 << index_bits(level)); set++) {
            for (unsigned way = 0; way < ways(level); way++) {
                cisw(level, set, way);
            }
        }
    }
    flush();
}
#endif

void cachehound::kernel_memory::switch_channel() noexcept
{
    switch_channel((active_channel_ + 5) % channel_count_);
}

#endif /* CACHEHOUND_BACKENDS_IMPL_KERNEL_MEMORY_IPP */
