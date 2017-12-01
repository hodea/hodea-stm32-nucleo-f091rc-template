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

constexpr uintptr_t boot_info_addr = 0x080000bcU;
constexpr uintptr_t appl_info_addr = 0x08002000U;
constexpr uintptr_t appl_vector_table_rom_addr = 0x08002040U;
constexpr uintptr_t appl_end_addr = 0x08003fffU;

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
    uint16_t magic;     //!< Magic number used to check integrity.
    uint32_t version;   //!< Bootloader version information.
    char id_string[30]; //!< Textual information about the bootloader image.
} Boot_info;

static const Boot_info& boot_info =
    *reinterpret_cast<Boot_info*>(boot_info_addr);

constexpr uint16_t boot_magic = (0xa400 | sizeof(Boot_info));

/**
 * Information about the application.
 */
typedef struct {
    uint16_t magic;      //!< Magic number used to check integrity.
    uint16_t ignore_crc; //!< Ignores CRC if set to ignore_appl_crc_key

    /**
     * CRC-32 over application code.
     * The CRC is calculated from the version member of this structure
     * till appl_end_addr.
     *
     * (Ethernet) polynomial: 0x4C11DB7
     * CRC initial value: 0xffffffff
     */
    uint32_t crc;
    uint32_t version;   //!< Application version information.
    char id_string[30]; //!< Textual information about the bootloader image.
} Appl_info;

static const Appl_info& appl_info =
    *reinterpret_cast<Appl_info*>(appl_info_addr);

constexpr uint16_t ignore_appl_crc_key = 0xb0c1;
constexpr uint16_t appl_magic = (0x6100 | sizeof(Appl_info));

/**
 * Enter bootloader.
 *
 * This function branches to the bootloader. It does not return.
 */
[[noreturn]] static inline void enter_bootloader()
{
    NVIC_SystemReset();
    for (;;) ;              // avoid warning about [[noreturn]]
}

/**
 * Enter application.
 *
 * This function branches to the application. It does not return.
 */
[[noreturn]] void enter_application();

#endif /*!_BOOT_APPL_IF_HPP_ */
