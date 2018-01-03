// Copyright (c) 2017, Franz Hollerer.
// SPDX-License-Identifier: MIT

/**
 * Application main code.
 * \author f.hollerer@gmx.net
 */
#include <cstdio>
#include <hodea/core/cstdint.hpp>
#include <hodea/core/bitmanip.hpp>
#include <hodea/device/hal/device_setup.hpp>
#include <hodea/device/hal/pin_config.hpp>
#include <hodea/device/hal/retarget_stdout_uart.hpp>
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

/**
 * Initialization.
 */
static void init()
{
    retarget_init(USART2, baud_to_brr(115200));
    rte_init();
}

/**
 * Shutdown.
 */
static void deinit()
{
    rte_deinit();
    retarget_deinit();
}

#if defined __ARMCC_VERSION && (__ARMCC_VERSION >= 6010050)
// Build works with -O3, but fails with -O0.
// This is the workaround proposed in support case 710226:w
__asm(".global __ARM_use_no_argv\n");
#endif

[[noreturn]] int main()
{
    init();

    printf("executing application\n");

    while (!user_button.is_pressed()) {
        kick_watchdog();
        run_led.toggle();
        Htsc::delay(Htsc::ms_to_ticks(200));
    }

    while (user_button.is_pressed()) ;   // wait till button is released
    Htsc::delay(Htsc::ms_to_ticks(100)); // care about bouncing 

    signal_update_request();

    deinit();
    enter_bootloader();
}
