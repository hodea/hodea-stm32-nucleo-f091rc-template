// Copyright (c) 2017, Franz Hollerer. All rights reserved.
// This code is licensed under the MIT License (MIT).
// See LICENSE file for full details.

/**
 * Interface between bootloader and application code.
 */
#if !defined _BOOT_APPL_IF_HPP_
#define _BOOT_APPL_IF_HPP_

#include <hodea/core/cstdint.hpp>
#include <hodea/device/hal/device_setup.hpp>

/**
 * Number of vector table entries including initial stack pointer.
 *
 * \note
 * This must match with the vector table defined in system_stm32f*.s,
 * otherwise the application may crash.
 */
constexpr int nvic_vector_table_entries = 47;

constexpr uintptr_t appl_vector_table_rom_addr = 0x08001840U;

/**
 * Activate bootloader.
 *
 * This function branches to the bootloader. It does not return.
 */
[[noreturn]] static inline void activate_bootloader()
{
    NVIC_SystemReset();
    for (;;) ;              // avoid warning about [[noreturn]]
}

/**
 * Activate application.
 *
 * This function branches to the application. It does not return.
 */
[[noreturn]] void activate_application();

#endif /*!_BOOT_APPL_IF_HPP_ */
