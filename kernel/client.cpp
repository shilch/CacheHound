#include "ch_channel.h"
#include "ch_ioc.h"

#include <cstring>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>
#include <thread>

int main() {
    int fd = open("/dev/cachehound", O_RDWR);
    if(fd < 0) {
        perror("failed to open device");
        return 1;
    }

    auto channel = (ch_channel*)mmap(NULL, sizeof(ch_channel), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(channel == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    std::cout << "Channel address: " << (void*)channel << std::endl;

    ch_ioc_config config {
        .cpu = 1,
        .order = 12,
        .isolation_level = CH_ISOLATION_DISABLE_IRQ
    };

    int res = ioctl(fd, CH_IOC_START_AGENT, &config);
    if(res < 0) {
        perror("failed to start agent");
        return 1;
    }

    close(fd);

    std::cout << "Virtual base: " << (void*)config.virtual_base << std::endl;
    std::cout << "Physical base: " << (void*)config.physical_base << std::endl;

    for(int i = 1; i < 30; i += 2) {
        ch_channel_wait_for_idle(channel);
        std::cout << "set_access " << i << std::endl;
        ch_channel_set_access(channel, i);
    }

    ch_channel_wait_for_idle(channel);
    ch_channel_set_access(channel, 0);

    return 0;
}
