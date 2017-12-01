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

const Boot_info boot_info_rom 
    __attribute__((section(".boot_info"), used)) =
{
    boot_magic,                 // magic
    1,                          // version
    "project_template boot"     // id_string
};

/**
 * Turn on clocks for peripherals used in the application.
 */
static void init_peripheral_clocks(void)
{
    set_bit(RCC->AHBENR, RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN);
    set_bit(RCC->APB2ENR, RCC_APB2ENR_SYSCFGCOMPEN);
}

/**
 * General Purpose I/O pin configuration.
 *
 * \verbatim
 * Pin  Name            Dir     AF      Function
 * 1    VBAT
 * 2    PC13            I               B_USER, blue user button
 * 3    PC14            I               unused
 * 4    PC15            I               unused
 * 5    PF0             I               unused
 * 6    PF1             I               unused
 * 7    NRST            I/O
 * 8    PC0             I               unused
 * 9    PC1             I               unused
 * 10   PC2             I               unused
 * 11   PC3             I               unused
 * 12   VSSA
 * 13   VDDA
 * 14   PA0             I               unused
 * 15   PA1             I               unused
 * 16   PA2             I               unused
 * 17   PA3             I               unused
 * 18   VSS
 * 19   VDD
 * 20   PA4             I               unused
 * 21   PA5             O               LD1, green run LED
 * 22   PA6             I               unused
 * 23   PA7             I               unused
 * 24   PC4             I               unused
 * 25   PC5             I               unused
 * 26   PB0             I               unused
 * 27   PB1             I               unused
 * 28   PB2             I               unused
 * 29   PB10            I               unused
 * 30   PB11            I               unused
 * 31   VSS
 * 32   VDD
 * 33   PB12            I               unused
 * 34   PB13            I               unused
 * 35   PB14            I               unused
 * 36   PB15            I               unused
 * 37   PC6             I               unused
 * 38   PC7             I               unused
 * 39   PC8             I               unused
 * 40   PC9             I               unused
 * 41   PA8             I               unused
 * 42   PA9             I               unused
 * 43   PA10            I               unused
 * 44   PA11            I               unused
 * 45   PA12            I               unused
 * 46   PA13/SYS_SWDIO  O       AF0     Serial wire debug, data I/O 
 * 47   VSS
 * 48   VDD
 * 49   PA14/SYS_SWCLK  I       AF0     Serial wire debug, clock input
 * 50   PA15            I               unused
 * 51   PC10            I               unused
 * 52   PC11            I               unused
 * 53   PC12            I               unused
 * 54   PD2             I               unused
 * 55   PB3             I               unused
 * 56   PB4             I               unused
 * 57   PB5             I               unused
 * 58   PB6             I               unused
 * 59   PB7             I               unused
 * 60   BOOT0                           unused
 * 61   PB8             I               unused
 * 62   PB9             I               unused
 * 63   VSS
 * 64   VDD
 * \endverbatim
 *
 * \note 
 * On reset all pins except PA13/SYS_SWDIO and PA14/SYS_SWCLK are
 * configured as digital input.  PA13 and PA14 are in AF0 mode.
 *
 * - GPIOx_MODER
 *      all pins are digital input, except PA13 and PA14
 * - GPIOx_OTYPER
 *      all outputs are in push-pull output mode
 * - GPIOx_OSPEEDR
 *      all pins set to low speed, except PA14/SYS_SWCLK for which
 *      high speed is enabled
 *      low speed: up to 2 MHz
 * - GPIOx_PUBDR
 *      no pull-up / pull-down, except PA13 and PA14 where pull-up is
 *      enabled
 * - GPIOx_ODR
 *      all bits cleared
 * - GPIOx_AFRL, GPIOx_AFRH
 *      all pins set to AF0 (active if alternate function mode selected)
 */
static void init_pins(void)
{
    // Configure pin mode register.
    Config_gpio_mode{GPIOA}
        .pin(5, Gpio_pin_mode::output)
        .pin(13, Gpio_pin_mode::af)
        .pin(14, Gpio_pin_mode::af)
        .write();
}

#if defined __ARMCC_VERSION && (__ARMCC_VERSION >= 6010050)
// Build works with -O3, but fails with -O0.
// This is the workaround proposed in support case 710226
__asm(".global __ARM_use_no_argv\n");
#endif

[[noreturn]] int main()
{
    init_peripheral_clocks();
    init_pins();
    rte_init();

    while (!user_button.is_pressed()) {
        run_led.toggle();
        htsc::delay(htsc::ms_to_ticks(100));
    }

    enter_application();
}
