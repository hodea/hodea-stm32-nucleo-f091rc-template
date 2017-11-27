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
 * Persistent data in SRAM shared between bootloader and application.
 */
typedef struct {
    /**
     * Flag set by application before switching to bootloader to start
     * a firmware update.
     */
    bool is_update_requested;

    // additional data which needs to be persistent comes here...
} Boot_data;

extern Boot_data boot_data;

/**
 * Information about the bootloader.
 */
typedef struct {
    uint32_t magic;     //!< Magic number used to check integrity.
    uint32_t version;   //!< Bootloader version information.
    char id_string[30]; //!< Textual information about the bootloader image.
} Boot_info;

static const Boot_info& boot_info = *reinterpret_cast<Boot_info*>(0x2000);

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
