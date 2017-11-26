// Copyright (c) 2017, Franz Hollerer. All rights reserved.
// This code is licensed under the MIT License (MIT).
// See LICENSE file for full details.

/**
 * Interface between bootloader and application code.
 */
#include <cstring>
#include "boot_appl_if.hpp"

/**
 * Copy of the application interrupt vector table in SRAM.
 */
uint32_t appl_vector_table_ram[nvic_vector_table_entries];

#if defined __GNUC__

[[noreturn]] void jump_to_appl()
{
    asm volatile(
        "ldr r0, =appl_vector_table_ram\n\t"
        "ldr r1, [r0]\n\t"      // load stack pointer initial value
        "msr msp, r1\n\t"       // set stack pointer
        "add r0, #4\n\t"        // load reset vector
        "ldr r1, [r0]\n\t"      
        "bx  r1\n\t"            // branch to reset vector
        :::"memory"
        );
    for (;;) ;                  // avoid warning about [[noreturn]]
}

#elif defined __ARMCC_VERSION && (__ARMCC_VERSION >= 6010050)

[[noreturn]] __asm static void jump_to_appl(void)
{
    EXTERN  appl_vector_table_ram

    ldr r0, =appl_vector_table_ram  ; stack pointer initial value
    ldr r1, [r0]
    msr msp, r1
    nop                             ; padding (otherwise we get a warning)
    adds r0, #4                     ; reset vector
    ldr r1, [r0]
    bx r1                           ; branch to reset vector
}

#endif

/**
 * Activate application.
 *
 * This function branches to the application. It does not return.
 */
void activate_application()
{
    // copy application interrupt vector table from flash to sram
    std::memcpy(
            appl_vector_table_ram, 
            reinterpret_cast<void*>(appl_vector_table_rom_addr),
            sizeof(appl_vector_table_ram)
            );

    /*
     * Map SRAM to address 0 thus the copy of the application vector table
     * is used.
     * Note: The SRAM is still accessible on its original address.
     */
    SYSCFG->CFGR1 |= (SYSCFG_CFGR1_MEM_MODE_0 | SYSCFG_CFGR1_MEM_MODE_1);
    __DSB();

    jump_to_appl();
}
