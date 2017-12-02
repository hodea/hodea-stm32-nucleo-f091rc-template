// Copyright (c) 2017, Franz Hollerer.
// SPDX-License-Identifier: MIT

/**
 * Interface between bootloader and application code.
 */
#include <cstring>
#include "boot_appl_if.hpp"

Boot_data boot_data __attribute__((section(".boot_data"), used));

/**
 * Copy of the application interrupt vector table in SRAM.
 */
uint32_t __attribute__((section(".appl_vector_ram"), used))
    appl_vector_table_ram[nvic_vector_table_entries];

#if defined __GNUC__

[[noreturn]] void jump_to_appl()
{
    asm volatile(
        "ldr r0, =appl_vector_table_ram\n\t"
        "ldr r1, [r0]\n\t"      // load stack pointer initial value
        "msr msp, r1\n\t"       // set stack pointer
        "isb\n\t"
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
    isb
    adds r0, #4                     ; reset vector
    ldr r1, [r0]
    bx r1                           ; branch to reset vector
}

#endif

/**
 * Enter application.
 *
 * This function branches to the application. It does not return.
 *
 * \note
 * On Cortex-M0 devices we have to copy the vector table into SRAM and
 * remap it to address 0 before we can jump to the application code.
 * Cortex-M3/4 introduced the SCB->VTOR register to relocate the
 * vector table.
 *
 * If the MCU supports relocating the vector table the function can be
 * implemented as follows:
 *
 * \code
 * void activate_application()
 * {
 *     SCB->VTOR = appl_vector_table_ram;
 *     __DSB();
 *     jump_to_appl();
 * }
 * \endcode
 */
void enter_application()
{
    /*
     * Copy application interrupt vector table from FLASH to SRAM.
     */
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
