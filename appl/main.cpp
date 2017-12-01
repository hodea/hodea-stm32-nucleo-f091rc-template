// Copyright (c) 2017, Franz Hollerer. All rights reserved.
// This code is licensed under the MIT License (MIT).
// See LICENSE file for full details.

/**
 * Bootloader main code.
 *
 * This file provides the framework for a bootloader. It implements the
 * following features:
 *
 * - Provides a minimum board configuration.
 * - Enters bootloader mode in case a firmware update is requested or
 *   the application is corrupted or not present.
 * - Otherwise, if the CRC is correct it starts the application.
 *
 * Especially the I/Os, which are set to input after reset, are
 * initialized according the board layout. With that we make sure that
 * the board is always in a safe state. This also means that the bootloader
 * must be aware of the hardware, and that hardware changes require to
 * adapt the bootloader.
 *
 * The application takes over the clock and pin configuration from
 * the bootloader and builds the application specific part on top of it.
 *
 * What is not part of this framework is the code is the firmware upate
 * itself.
 *
 * \author f.hollerer@gmx.net
 */
#include <cstdio>
#include <hodea/core/cstdint.hpp>
#include <hodea/core/bitmanip.hpp>
#include <hodea/device/hal/device_setup.hpp>
#include <hodea/device/hal/pin_config.hpp>
#include <hodea/rte/setup.hpp>
#include <hodea/rte/htsc.hpp>
#include "../share/digio_pins.hpp"
#include "../share/boot_appl_if.hpp"

using namespace hodea;

const Appl_info appl_info_rom 
    __attribute__((section(".appl_info"), used)) =
{
    appl_magic,                 // magic
    ignore_appl_crc_key,        // ignore_crc
    0,                          // crc
    1,                          // version
    "project_template appl"     // id_string
};


#if defined __ARMCC_VERSION && (__ARMCC_VERSION >= 6010050)
// Build works with -O3, but fails with -O0.
// This is the workaround proposed in support case 710226:w
__asm(".global __ARM_use_no_argv\n");
#endif

[[noreturn]] int main()
{
    rte_init();

    while (!user_button.is_pressed()) {
        run_led.toggle();
        htsc::delay(htsc::ms_to_ticks(200));
    }

    enter_bootloader();
}
