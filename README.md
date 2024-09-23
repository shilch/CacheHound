![CacheHound logo](./doc/images/logo.png)

# What is CacheHound?
CacheHound is a library and command-line tool for reverse-engineering CPU cache policies, specifically replacement and placement policies (cache set index function).
It has been tested on four x86- and ARM-based systems thus far.

This application was developed as part of my Master's thesis titled "CacheHound: Automated Reverse-Engineering of CPU Cache Policies in Modern Multiprocessors" at the Chair of Communication Systems and System Programming at the Ludwig Maximilian University of Munich.
The thesis has been published [here](https://www.mnm-team.org/pub/Diplomarbeiten/hilc24/).

**CPU** | **Operating System** | `uname -r` | **Compiler**
--------|----------------------|------------|-------------
BCM2712 | Debian 12 | `6.6.28+rpt-rpi-2712` | GCC 12.2.0
Intel Xeon E5-2680 v2 | Ubuntu 22.04.4 LTS | `5.15.0-107-generic` | GCC 12.3.0
AMD EPYC 7302 | Ubuntu 22.04.4 LTS | `5.15.0-119-generic` | GCC 11.4.0
Fujitsu A64FX | CentOS Linux 8 (Core) | `4.18.0-193.19.1.el8_2.aarch64` | GCC 13.2.0

# Building
CacheHound consists of a Linux kernel module and a main executable which must be built independently.

To build the kernel module:
```bash
cd kernel
make
```

To build the main executable:
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

# Running
The kernel module must be loaded once and will stay loaded until either a reboot occurs or it is unloaded.
```bash
sudo insmod cachehound.ko
sudo rmmod cachehound      # To unload the module
```

After the module has been loaded successfully, it should announce itself in the kernel log (check `sudo dmesg -w`).

To get familiar with the tool, you can use the `-h` flag to get detailed information about the available arguments and subcommands offered by the cli.
Two examples are provided below.

## Reverse-Engineering the Placement Policy
Example of reverse-engineering the L2 cache placement policy on the Raspberry Pi 5 using the CISW Eviction Strategy:

```bash
sudo ./cli/cli -VV                     \  # Enable trace output
    reverse                            \
        -L 2                           \  # Provide bypass to the L2 cache
        -m 1073741824                  \  # Allocate 1GiB of memory
        --kernel-cpu 3                 \  # Launch the kernel agent thread
                                       \  # on CPU core 3
        --kernel-isolation disable-irq \  # Disable IRQs in agent thread
        --pmu rpi5                     \  # Performance counter configuration
    placement                          \  # Reverse the placement policy
        -S cisw                           # CISW Eviction Strategy
```

This should take less than 60 seconds using the _CISW Eviction Strategy_ and several minutes to an hour using the _Eviction Set Strategy_.

## Reverse-Engineering the Replacement Policy
Example of reverse-engineering the L2 cache replacement policy on the Raspberry Pi 5:

```bash
sudo ./cli/cli -VV                     \  # Enable trace output
    reverse                            \
        -L 2                           \  # Provide bypass to the L2 cache
        -m 1073741824                  \  # Allocate 1GiB of memory
        --kernel-cpu 3                 \  # Launch the kernel agent thread
                                       \  # on CPU core 3
        --kernel-isolation disable-irq \  # Disable IRQs in agent thread
        --pmu rpi5                     \  # Performance counter configuration
    replacement                        \  # Reverse the replacement policy ...
        -i 37                             # ... for cache set #37
```

This should take just a few seconds.

# System Configuration
## Hardware prefetcher
We recommend to disable the hardware prefetcher on the system under test.
Usually, there is a BIOS option to do so.
Some systems have platform-specific interfaces to disable the hardware prefetcher, e.g., the Fujitsu A64FX and Raspberry Pi 5.
Please find more information on the disablement on these systems in the Master's thesis linked above.

## NMI-Watchdog
The Linux NMI-Watchdog available under x86 systems can interfere with the kernel module.
We recommend to disable it by setting `kernel.nmi_watchdog=0` in the `/etc/sysctl.conf` and booting Linux using the `nmi_watchdog=0` argument.

## Development
During development of the kernel module, we recommend to specify `sudo sysctl kernel/kptr_restrict=0` such that the kernel pointers printed to `dmesg` are not masked.
