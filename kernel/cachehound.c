#include "ch_channel.h"
#include "ch_ioc.h"
#include "ch_pmc.h"
#include "ch_plat.h"
#include "ch_cache_info.h"

#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/atomic.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <asm/page.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/mutex.h>

#define CH_AGENT_NAME "kcachehound"

struct ch_state {
    atomic_t ref_counter;

    struct page* channels_page;
    struct ch_channel* channels;
    size_t active_channel;

    atomic_t started;
    unsigned isolation_level;
    unsigned long long evts[3];

    struct ch_memory_region* regions_head;
    struct ch_memory_region* regions_tail;
};

struct ch_memory_region {
    void* base;
    struct page* page;
    unsigned order;

    struct ch_memory_region* next;
};

struct ch_state* alloc_ch_state(void) {
    struct page* channels_page;
    struct ch_channel* channels;
    struct ch_state* state;

    channels_page = alloc_page(GFP_KERNEL);
    if(!channels_page) {
        pr_alert("Failed to allocate page for channels\n");
        return NULL;
    }

    channels = memremap(page_to_pfn(channels_page) << PAGE_SHIFT, PAGE_SIZE, MEMREMAP_WB);
    if(!channels) {
        __free_page(channels_page);
        pr_alert("Failed to map channels page into kernel address space\n");
        return NULL;
    }
    memset(channels, 0, PAGE_SIZE);

    state = kmalloc(sizeof(struct ch_state), GFP_KERNEL);
    if(!state) {
        memunmap(channels);
        __free_page(channels_page);
        pr_alert("Failed to kmalloc cachehound state\n");
        return NULL;
    }

    atomic_set(&state->ref_counter, 1);
    state->channels_page = channels_page;
    state->channels = channels;
    state->active_channel = 0;
    atomic_set(&state->started, 0);
    state->isolation_level = CH_ISOLATION_OFF;
    state->regions_head = NULL;
    state->regions_tail = NULL;

    pr_info("Cachehound state allocated\n");
    return state;
}

void ch_state_free(struct ch_state* state) {
    /* state->ref_count is 0 */
    void* region_base;
    struct page* region_page;
    unsigned region_order;
    struct page* channels_page = state->channels_page;
    struct ch_channel* channels = state->channels;
    struct ch_memory_region *region_it = state->regions_head, *prev_region;

    while(region_it) {
        region_base = region_it->base;
        region_page = region_it->page;
        region_order = region_it->order;

        prev_region = region_it;
        region_it = region_it->next;
        kfree(prev_region);

        memunmap(region_base);
        __free_pages(region_page, region_order);
    }
    pr_info("All cachehound kernel memory regions freed");

    kfree(state);
    memunmap(channels);
    __free_page(channels_page);

    pr_info("Cachehound state freed");
}

void ch_state_get(struct ch_state* state) {
    int old_ref, new_ref;

    do {
        old_ref = atomic_read(&state->ref_counter);
        new_ref = old_ref + 1;
    } while(atomic_cmpxchg_acquire(&state->ref_counter, old_ref, new_ref) != old_ref);
}

void ch_state_put(struct ch_state* state) {
    int old_ref, new_ref;

    do {
        old_ref = atomic_read(&state->ref_counter);
        new_ref = old_ref - 1;
    } while(atomic_cmpxchg_acquire(&state->ref_counter, old_ref, new_ref) != old_ref);

    if(!new_ref) {
        ch_state_free(state);
    }
}

int ch_state_try_start(struct ch_state* state) {
    return atomic_cmpxchg_acquire(&state->started, 0, 1) == 0;
}

void ch_state_stop(struct ch_state* state) {
    atomic_set_release(&state->started, 0);
}

int ch_state_alloc_memory(struct ch_state* state, unsigned order) {
    struct page* page;
    void* base;
    struct ch_memory_region* region;

    page = alloc_pages(GFP_KERNEL, order);
    if(!page) {
        pr_info("Allocation of kernel page of order %u failed\n", order);
        return -ENOMEM;
    }

    base = memremap(page_to_pfn(page) << PAGE_SHIFT, PAGE_SIZE, MEMREMAP_WB);
    if(!base) {
        __free_pages(page, order);
        pr_alert("Failed to map kernel hugepage into kernel address space\n");
        return -EIO;
    }

    region = kmalloc(sizeof(struct ch_memory_region), GFP_KERNEL);
    if(!region) {
        memunmap(base);
        __free_pages(page, order);
        pr_alert("Failed to kmalloc cachehound memory region\n");
        return -ENOMEM;
    }

    region->page = page;
    region->base = base;
    region->order = order;
    region->next = NULL;

    if(!state->regions_head) {
        state->regions_head = state->regions_tail = region;
    } else {
        state->regions_tail->next = region;
        state->regions_tail = region;
    }

    pr_info("Cachehound kernel memory region of order %u allocated and mapped\n", order);

    return 0;
}

int agent(void* type_erased_state) {
    int cpu;
    struct ch_state* state = type_erased_state;
    unsigned long flags;
    struct ch_local_pmc local_pmc;
    unsigned i;

    cpu = get_cpu();
    pr_info("Hello from " CH_AGENT_NAME " (PID %d) on CPU %d!\n", current->pid, cpu);

    if(state->isolation_level >= CH_ISOLATION_NO_PREEMPT) {
        preempt_disable();
        if(state->isolation_level >= CH_ISOLATION_DISABLE_IRQ) {
            local_irq_save(flags);
        }
    }

    /* Init PMC (important on Intel/AMD) */
    ch_local_pmc_init();

    /* Save current PMC configuration */
    ch_local_pmc_save(&local_pmc);

    /* Configure PMC */
    for(i = 0; i < 3; i++) {
        if(state->evts[i] == 0) break;
        pr_info("Programming counter %d to track event %llu\n", i, state->evts[i]);
        ch_local_pmc_configure(i, state->evts[i]);
    }

#if defined(__x86_64__) || defined(_M_X64)
    asm volatile(
        /* Nop slide to mark begin to verify code in objdump -d */
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"

        /* Calculate channel_ptr */
        "movq (%[active_channel]), %%r15\n"
        ".calculate_channel_ptr:\n"
        "salq $6, %%r15\n"
        "movq %[channels_ptr], %[channel_ptr]\n"
        "addq %%r15, %[channel_ptr]\n"

        /* Backup channel_ptr */
        "movq %[channel_ptr], %%rsi\n"

        /* Loop that waits for channel access (acquire semantics) */
        ".retry:\n"
        "pause\n"
        "movq (%[channel_ptr]), %%rax\n"
        "testq %%rax, %%rax\n"
        "je .retry\n"

        /* Extract count from request */
        "shrq %%rax\n"

        /* Exit if count == 0 && switch_channel == 0 && wbinvd == 0*/
        "je .exit\n"

        /* Check for wbinvd */
        "movq %%rax, %%rbx\n"
        "shrq $4, %%rbx\n"
        "test %%rbx, %%rbx\n"
        "jz .handle_switch_channel\n"
        "wbinvd\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "lfence\n"
        "xor %%rax, %%rax\n"
        "movq %%rax, (%[channel_ptr])\n"
        "jmp .retry\n"

        /* Check for switch_channel */
        ".handle_switch_channel:\n"
        "movq %%rax, %%r15\n"
        "shrq $3, %%r15\n"
        "test %%r15, %%r15\n"
        "jz .handle_count\n"
        /* Put switched-out channel into idle */
        "movq $0, (%[channel_ptr])\n"
        /* Read new active_channel from data[0] */
        "addq $8, %[channel_ptr]\n"
        /* Read new active_channel into %r15 as used by .calculate_channel_ptr */
        "movq (%[channel_ptr]), %%r15\n"
        "movq %%r15, (%[active_channel])\n"
        "mfence\n"
        "jmp .calculate_channel_ptr\n"

        /* This is where the action happens */
        ".handle_count:\n"
        /* Address mask */
        "movabsq $0xFFFF000000000000, %%r15\n"
        /* Decrement count by one */
        "subq $1, %%rax\n"
        /* Jump to instrumented loop if we reached 0 */
        "je .instrloop\n"

        /* Non-instrumented loop */
        ".noninstrloop:\n"
        "addq $8, %[channel_ptr]\n"
        "movq (%[channel_ptr]), %%rbx\n"
        "test %%rbx, %%rbx\n"
        "jns .noninstr_flush\n"
        "orq %%r15, %%rbx\n"
        "movq (%%rbx), %%rbx\n"
        "jmp .noninstr_noflush\n"
        ".noninstr_flush:\n"
        "orq %%r15, %%rbx\n"
        "clflush (%%rbx)\n"
        ".noninstr_noflush:\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "subq $1, %%rax\n"
        "jne .noninstrloop\n"

        /* Instrumented loop */
        ".instrloop:\n"
        "addq $8, %[channel_ptr]\n"
        "movq (%[channel_ptr]), %%r11\n"
        "xor %%rax, %%rax\n"
        "cpuid\n"
        /* RDPMC 0 into R08 */
        "xor %%rcx, %%rcx\n"
        "rdpmc\n"
        "salq $32, %%rdx\n"
        "orq %%rax, %%rdx\n"
        "movq %%rdx, %%r8\n"
        /* RDPMC 1 into R09 */
        "addq $1, %%rcx\n"
        "rdpmc\n"
        "salq $32, %%rdx\n"
        "orq %%rax, %%rdx\n"
        "movq %%rdx, %%r9\n"
        /* RDPMC 3 into R10 */
        "addq $1, %%rcx\n"
        "rdpmc\n"
        "salq $32, %%rdx\n"
        "orq %%rax, %%rdx\n"
        "movq %%rdx, %%r10\n"
        /* Perform access */
        "test %%r11, %%r11\n"
        "jns .instr_flush\n"
        "orq %%r15, %%r11\n"
        "movq (%%r11), %%rbx\n"
        "jmp .instr_noflush\n"
        ".instr_flush:\n"
        "orq %%r15, %%r11\n"
        "clflush (%%r11)\n"
        ".instr_noflush:\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "mfence\n"
        "xor %%rax, %%rax\n"
        "cpuid\n"
        /* RDPMC 0 into R11 */
        "xor %%rcx, %%rcx\n"
        "rdpmc\n"
        "salq $32, %%rdx\n"
        "orq %%rax, %%rdx\n"
        "movq %%rdx, %%r11\n"
        /* RDPMC 1 into R15 */
        "addq $1, %%rcx\n"
        "rdpmc\n"
        "salq $32, %%rdx\n"
        "orq %%rax, %%rdx\n"
        "movq %%rdx, %%r15\n"
        /* RDPMC 3 into RDX */
        "addq $1, %%rcx\n"
        "rdpmc\n"
        "salq $32, %%rdx\n"
        "orq %%rax, %%rdx\n"

        /* Write back counters into channel (release semantics) */
        "movq %%rsi, %[channel_ptr]\n"
        "addq $8, %[channel_ptr]\n"
        "movq %%r8, (%[channel_ptr])\n"
        "addq $8, %[channel_ptr]\n"
        "movq %%r9, (%[channel_ptr])\n"
        "addq $8, %[channel_ptr]\n"
        "movq %%r10, (%[channel_ptr])\n"
        "addq $8, %[channel_ptr]\n"
        "movq %%r11, (%[channel_ptr])\n"
        "addq $8, %[channel_ptr]\n"
        "movq %%r15, (%[channel_ptr])\n"
        "addq $8, %[channel_ptr]\n"
        "movq %%rdx, (%[channel_ptr])\n"
        "movq %%rsi, %[channel_ptr]\n"
        "xor %%rax, %%rax\n"
        "movq %%rax, (%[channel_ptr])\n"
        "sfence\n"
        "jmp .retry\n"

        ".exit:\n"

        /* Nop slide to mark end to verify code in objdump -d */
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"

        /* Output registers*/
        :

        /* Input registers */
        : [channels_ptr] "r"(state->channels)
        , [channel_ptr] "r"(0ULL)
        , [active_channel] "r"(&(state->active_channel))

        /* Clobber registers */
        : "%rax", "%rbx", "%rcx", "%rdx", "%rsi"
        , "%r8", "%r9", "%r10", "%r11", "%r15"
        , "memory"
    );
#elif defined(__aarch64__) || defined(_M_ARM64)
    unsigned long long cisw_counter = 0
                     , isw_counter  = 0
                     , csw_counter  = 0;

    asm volatile (
        /* Nop slide to mark begin to verify code in objdump -d */
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "MOV %[cisw_counter], xzr\n"
        "MOV %[isw_counter], xzr\n"
        "MOV %[csw_counter], xzr\n"

        /* Calculate channel_ptr */
        "LDR x0, [%[active_channel]]\n"
        ".calculate_channel_ptr:\n"
        "UBFIZ x0, x0, #6, #32\n"
        "MOV %[channel_ptr], %[channels_ptr]\n"
        "ADD %[channel_ptr], %[channel_ptr], x0\n"

        /* Backup channel_ptr */
        "MOV x0, %[channel_ptr]\n"

        /* Loop that waits for channel access (acquire semantics) */
        ".retry:\n"
        "YIELD\n"
        "LDAR x1, [%[channel_ptr]]\n"
        "CBZ x1, .retry\n"

        /* Extract count from request */
        "LSR x1, x1, #1\n"

        /* Exit if count == 0 && switch_channel == 0 && clean_invalidate_cache == 0 */
        "CBZ x1, .exit\n"

        /* Check for clean_invalidate_cache */
        // ".clean_invalidate_cache:\n"
        // "UBFX x2, x1, #4, #2\n"
        // "CBZ x2, .handle_switch_channel\n"

        // "MRS x3, CLIDR_EL1\n"
        // "AND w4, w0, #0x07000000\n"
        // "LSR w4, w4, #23\n"
        // "STLR xzr, [%[channel_ptr]]\n"
        // "CBZ w4, .retry\n"
        // "mov w10, #0\n"
        // "mov w8, #1\n"
        // ""

        /* Check for switch_channel */
        ".handle_switch_channel:\n"
        "UBFX x2, x1, #3, #1\n"
        "CBZ x2, .handle_count\n"
        /* Put switched-out channel into idle */
        "STLR xzr, [%[channel_ptr]]\n"
        /* Read new active_channel from data[0] into x0 as used by .calculate_channel_ptr */
        "LDR x0, [%[channel_ptr], #8]\n"
        "STLR x0, [%[active_channel]]\n"
        "B .calculate_channel_ptr\n"

        /* This is where the action happens */
        ".handle_count:\n"
        /* Decrement count by one */
        "SUB x1, x1, #1\n"
        /* Jump to instrumented loop if we reached 0 */
        "CBZ x1, .instrloop\n"

        /* Non-instrumented loop */
        ".noninstrloop:\n"
        "ADD %[channel_ptr], %[channel_ptr], #8\n"
        "LDR x2, [%[channel_ptr]]\n"
        /* Check highest two bits for CISW */
        "UBFX x3, x2, #62, #2\n"
        "CMP x3, #2\n"
        "B.GT 3f\n"    /* >2 -> 3 -> no CISW */
        "B.EQ 2f\n"    /* 2 -> ISW */
        "CBZ x3, 0f\n" /* 0 -> CISW */

        "1:\n"
        "DC CSW, x2\n"
        "ADD %[csw_counter], %[csw_counter], #1\n"
        "B 4f\n"

        "2:\n"
        "DC ISW, x2\n"
        "ADD %[isw_counter], %[isw_counter], #1\n"
        "B 4f\n"

        "0:\n"
        "DC CISW, x2\n"
        "ADD %[cisw_counter], %[cisw_counter], #1\n"
        "B 4f\n"

        /* Normal access */
        "3:\n"
        "LDR x2, [x2]\n"
        "4:\n"
        "DSB SY\n"
        "SUB x1, x1, #1\n"
        "CBNZ x1, .noninstrloop\n"

        /* Instrumented loop */
        ".instrloop:\n"
        "ADD %[channel_ptr], %[channel_ptr], #8\n"
        "LDR x2, [%[channel_ptr]]\n"
        /* Check highest two bits for CISW */
        "UBFX x3, x2, #62, #2\n"
        "CMP x3, #2\n"
        "B.GT 3f\n"    /* >2 -> 3 -> no CISW */
        "B.EQ 2f\n"    /* 2 -> ISW */
        "CBZ x3, 0f\n" /* 0 -> CISW */

        "1:\n"
        "DC CSW, x2\n"
        "ADD %[csw_counter], %[csw_counter], #1\n"
        "B 4f\n"

        "2:\n"
        "DC ISW, x2\n"
        "ADD %[isw_counter], %[isw_counter], #1\n"
        "B 4f\n"

        "0:\n"
        "DC CISW, x2\n"
        "ADD %[cisw_counter], %[cisw_counter], #1\n"
        "B 4f\n"

        /* Instrumented access */
        "3:\n"
        "ISB SY\n"
        /* PMU counter 0 into x3 */
        "MRS x3, PMEVCNTR0_EL0\n"
        /* PMU counter 1 into x4 */
        "MRS x4, PMEVCNTR1_EL0\n"
        /* PMU counter 2 into x5 */
        "MRS x5, PMEVCNTR2_EL0\n"
        /* Perform access */
        "LDR x2, [x2]\n"
        "ISB SY\n"
        /* PMU counter 0 into x6 */
        "MRS x6, PMEVCNTR0_EL0\n"
        /* PMU counter 1 into x7 */
        "MRS x7, PMEVCNTR1_EL0\n"
        /* PMU counter 2 into x8 */
        "MRS x8, PMEVCNTR2_EL0\n"

        /* Write back counters into channel (release semantics) */
        "MOV %[channel_ptr], x0\n"
        "ADD %[channel_ptr], %[channel_ptr], #8\n"
        "STR x3, [%[channel_ptr]]\n"
        "ADD %[channel_ptr], %[channel_ptr], #8\n"
        "STR x4, [%[channel_ptr]]\n"
        "ADD %[channel_ptr], %[channel_ptr], #8\n"
        "STR x5, [%[channel_ptr]]\n"
        "ADD %[channel_ptr], %[channel_ptr], #8\n"
        "STR x6, [%[channel_ptr]]\n"
        "ADD %[channel_ptr], %[channel_ptr], #8\n"
        "STR x7, [%[channel_ptr]]\n"
        "ADD %[channel_ptr], %[channel_ptr], #8\n"
        "STR x8, [%[channel_ptr]]\n"

        "4:\n"
        "DSB SY\n"
        "MOV %[channel_ptr], x0\n"
        "STLR xzr, [%[channel_ptr]]\n"
        "B .retry\n"

        ".exit:\n"

        /* Nop slide to mark end to verify code in objdump -d */
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"

        /* Output registers */
        : [cisw_counter] "=&r"(cisw_counter)
        , [isw_counter] "=&r"(isw_counter)
        , [csw_counter] "=&r"(csw_counter)

        /* Input registers */
        : [channels_ptr] "r"(state->channels)
        , [channel_ptr] "r"(0ULL)
        , [active_channel] "r"(&(state->active_channel))

        /* Clobber registers */
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "memory"
    );

    pr_info("cisw_counter = %llu\n", cisw_counter);
    pr_info("csw_counter = %llu\n", csw_counter);
    pr_info("isw_counter = %llu\n", isw_counter);

    // unsigned long long id0;
    // asm volatile("MRS %[id0], PMCR_EL0" : [id0] "=r"(id0));
    // pr_info("PMCR = %llu\n", id0);
    // asm volatile("MRS %[id0], PMCNTENSET_EL0" : [id0] "=r"(id0));
    // pr_info("PMCNTENSET = %llu\n", id0);
    // asm volatile("MSR PMSELR_EL0, %[val]" :: [val] "r"(2));
    // asm volatile("MRS %[id0], PMSELR_EL0" : [id0] "=r"(id0));
    // pr_info("PMSELR = %llu\n", id0);
    // asm volatile("MRS %[id0], PMXEVTYPER_EL0" : [id0] "=r"(id0));
    // pr_info("PMXEVTYPER = %llu\n", id0);
    // asm volatile("MRS %[id0], PMXEVCNTR_EL0" : [id0] "=r"(id0));
    // pr_info("PMXEVCNTR = %llu\n", id0);
    // asm volatile("MRS %[id0], PMCCFILTR_EL0" : [id0] "=r"(id0));
    // pr_info("PMCCFILTR = %llu\n", id0);
    // asm volatile("MRS %[id0], PMCCNTR_EL0" : [id0] "=r"(id0));
    // pr_info("PMCCNTR = %llu\n", id0);
    // asm volatile ("MRS %[id0], CurrentEL" : [id0] "=r"(id0));
    // pr_info("EL = %llu\n", id0 >> 2);
    // // asm volatile("MSR s3_0_c15_c1_4, %[val]" :: [val] "r"(0x0000000961563000 | (1 << 15) | (1 << 5)));
    // asm volatile("MRS %[id0], S3_0_C15_C1_4" : [id0] "=r"(id0));
    // pr_info("CPUECTLR = %llu\n", id0);

    // asm volatile("MSR CSSELR_EL1, %[val]" :: [val] "r"(0b110));
    // asm volatile("MRS %[id0], CSSELR_EL1" : [id0] "=r"(id0));
    // pr_info("CSSELR = %llu\n", id0);
    // asm volatile("MRS %[id0], CCSIDR_EL1" : [id0] "=r"(id0));
    // pr_info("CCSIDR = %llu\n", id0);

#endif

    /* Restore previously saved PMC configuration */
    ch_local_pmc_restore(&local_pmc);

    if(state->isolation_level >= CH_ISOLATION_NO_PREEMPT) {
        if(state->isolation_level >= CH_ISOLATION_DISABLE_IRQ) {
            local_irq_restore(flags);
        }
        preempt_enable();
    }

    pr_info("Goodbye from " CH_AGENT_NAME " (PID %d) on CPU %d!\n", current->pid, cpu);
    put_cpu();
    ch_state_put(state);

    return 0;
}

static int device_open(struct inode* inode, struct file* file) {
    struct ch_state* state = alloc_ch_state();
    if(!state) return -ENOMEM;
    file->private_data = state;
    return 0;
}

static int device_release(struct inode* inode, struct file* file) {
    struct ch_state* state = file->private_data;
    if(atomic_read(&state->started)) {
        pr_info("Terminating channel %zu\n", state->active_channel);
        _ch_channel_terminate(state->channels + state->active_channel);
    }
    ch_state_put(state);
    pr_info("File released\n");
    return 0;
}

static int device_mmap(struct file* file, struct vm_area_struct* vma) {
    unsigned long pfn, len;
    int ret;

    struct ch_state* state = file->private_data;

    pfn = page_to_pfn(state->channels_page);

    len = vma->vm_end - vma->vm_start;
    ret = remap_pfn_range(vma, vma->vm_start, pfn, len, vma->vm_page_prot);
    if(ret < 0) {
        pr_err("Could not map channels page into user process\n");
        return -EIO;
    }

    return 0;
}

static long device_ioctl(struct file* file, unsigned int request, unsigned long argp) {
    struct task_struct* agent_task;
    int err;
    struct ch_state* state;
    unsigned level;
    int i;

    state = file->private_data;

    if(request == CH_IOC_ALLOC_MEMORY) {
        struct ch_ioc_alloc_config config;

        err = copy_from_user(&config, (struct ch_ioc_alloc_config*)argp, sizeof(config));
        if(err < 0) {
            return err;
        }

        err = ch_state_alloc_memory(state, config.order);
        if(err < 0) {
            return err;
        }

        config.physical_base = (page_to_pfn(state->regions_tail->page) << PAGE_SHIFT);
        config.virtual_base = (uintptr_t)state->regions_tail->base;

        pr_info("Allocated new memory region:\n");
        pr_info("Physical base address: 0x%px\n", (void*)(config.physical_base));
        pr_info("Virtual base address:  0x%px\n", (void*)(config.virtual_base));

        err = copy_to_user((struct ch_ioc_alloc_config*)argp, &config, sizeof(config));
        if(err < 0) {
            return err;
        }

        return 0;
    } else if(request == CH_IOC_CACHE_INFO) {
        struct ch_ioc_cache_info cache_info;
        cache_info.levels = ch_cache_levels();
        cache_info.offset_bits = ch_cache_offset_bits();
        for(level = 0; level < cache_info.levels; level++) {
            cache_info.sets[level] = ch_dcache_sets(level);
            cache_info.ways[level] = ch_dcache_ways(level);
        }

        err = copy_to_user((struct ch_ioc_cache_info*)argp, &cache_info, sizeof(cache_info));
        if(err < 0) {
            return err;
        }
        return 0;
    } else if(request == CH_IOC_START_AGENT) {
        struct ch_ioc_start_config config;

        if(!state->regions_head) {
            return -EINVAL;
        }

        err = copy_from_user(&config, (struct ch_ioc_start_config*)argp, sizeof(config));
        if(err < 0) {
            return err;
        }

        if(config.cpu >= NR_CPUS) {
            pr_alert("User requested CPU core %d that does not exist\n", config.cpu);
            return -EINVAL;
        }
        if(config.isolation_level >= NR_CH_ISOLATION) {
            pr_alert("Invalid isolation level %u\n", config.isolation_level);
            return -EINVAL;
        }

        pr_info("Requested agent to launch on CPU %d\n", config.cpu);

        if(!ch_state_try_start(state)) {
            pr_alert("Agent already started by a preceding call to ioctl()\n");
            return -EBUSY;
        }

        state->isolation_level = config.isolation_level;
        for(i = 0; i < 3; i++) {
            state->evts[i] = config.evts[i];
        }

        config.channels_base = (uintptr_t)state->channels;
        config.channel_count = PAGE_SIZE / sizeof(struct ch_channel);
        config.active_channel = state->active_channel;
        pr_info("Channels base address:  0x%px\n", (void*)(config.channels_base));
        pr_info("Active channel:         %zu (of %zu)\n", config.active_channel, config.channel_count);
        pr_info("Active channel address: 0x%px\n", (void*)(state->channels + state->active_channel));

        err = copy_to_user((struct ch_ioc_start_config*)argp, &config, sizeof(config));
        if(err < 0) {
            return err;
        }

        agent_task = kthread_create(agent, state, CH_AGENT_NAME);
        kthread_bind(agent_task, config.cpu);
        if(!agent_task) {
            pr_alert("Failed to create kthread on cpu\n");
            return -EIO;
        }

        ch_state_get(state);
        wake_up_process(agent_task);

        return 0;
    }

    return -EINVAL;
}

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .mmap = device_mmap,
    .unlocked_ioctl = device_ioctl
};

static int major;
static struct class *cls;

static int __init mod_init(void) {
    major = register_chrdev(0, CH_DEVICE_NAME, &fops);
    if(major < 0) {
        pr_alert("register_chrdev of " CH_DEVICE_NAME " failed with error %d\n", major);
        return major;
    }
    pr_info("Device successfully registered under major number %d\n", major);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0) 
    cls = class_create(CH_DEVICE_NAME); 
#else 
    cls = class_create(THIS_MODULE, CH_DEVICE_NAME); 
#endif 

    device_create(cls, NULL, MKDEV(major, 0), NULL, CH_DEVICE_NAME); 
    pr_info("Device created under /dev/%s\n", CH_DEVICE_NAME);

    pr_info("Cache levels: %d\n", ch_cache_levels());
    pr_info("Offset bits : %d\n", ch_cache_offset_bits());
    pr_info("Level 1 sets: %d\n", ch_dcache_sets(0));
    pr_info("Level 2 sets: %d\n", ch_dcache_sets(1));
    pr_info("Level 3 sets: %d\n", ch_dcache_sets(2));
    pr_info("Level 1 ways: %d\n", ch_dcache_ways(0));
    pr_info("Level 2 ways: %d\n", ch_dcache_ways(1));
    pr_info("Level 3 ways: %d\n", ch_dcache_ways(2));
    pr_info("Number of performance counters supported by this CPU: %d\n", ch_local_pmc_counters());

    return 0;
}

static void __exit mod_exit(void) {
    device_destroy(cls, MKDEV(major, 0)); 
    class_destroy(cls);
    unregister_chrdev(major, CH_DEVICE_NAME); 
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Simon Hilchenbach");
MODULE_DESCRIPTION("Kernel module accompanying the CacheHound tool");

module_init(mod_init);
module_exit(mod_exit);

