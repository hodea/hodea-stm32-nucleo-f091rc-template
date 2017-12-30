// Copyright (c) 2017, Franz Hollerer.
// SPDX-License-Identifier: MIT

/**
 * Interface between bootloader and application code.
 */
#if !defined BOOT_APPL_IF_HPP
#define BOOT_APPL_IF_HPP

#include <hodea/core/cstdint.hpp>
#include <hodea/device/hal/device_setup.hpp>
#include <hodea/device/hal/cpu.hpp>

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
     * Set by the application to \a update_requested_key to instruct the
     * bootloader to start the firmware update.
     */
    uint16_t update_requested;

    /**
     * CRC over application code as calculated by the bootloader.
     *
     * This is provided for convenience. It may be read out via a debugger
     * at the point appl_info needs to be prepared for a new release.
     */
    uint32_t appl_crc;

    // additional data which needs to be persistent comes here...
} Boot_data;

extern Boot_data boot_data;

constexpr uint16_t update_requested_key = 0xd989;

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
    uint16_t ignore_crc; //!< Ignores CRC if set to \a ignore_appl_crc_key.

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
 * Test if the bootloader info structure is correct.
 */
static inline bool is_boot_info_sane()
{
    return boot_info.magic == boot_magic;
}

/**
 * Test if the application info structure is correct.
 */
static inline bool is_appl_info_sane()
{
    return appl_info.magic == appl_magic;
}

/**
 * Test if a firmware update is requested.
 */
static inline bool is_update_requested()
{
    return boot_data.update_requested == update_requested_key;
}

/**
 * Signal firmware update request.
 */
static inline void signal_update_request()
{
    boot_data.update_requested = update_requested_key;
}

/**
 * Reset firmware update request.
 */
static inline void reset_update_request()
{
    boot_data.update_requested = 0;
}

/**
 * Enter bootloader.
 *
 * This function branches to the bootloader. It does not return.
 */
[[noreturn]] static inline void enter_bootloader()
{
    hodea::software_reset();
}

/**
 * Enter application.
 *
 * This function branches to the application. It does not return.
 */
[[noreturn]] void enter_application();


#endif /*!BOOT_APPL_IF_HPP */
