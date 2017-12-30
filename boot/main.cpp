// Copyright (c) 2017, Franz Hollerer.
// SPDX-License-Identifier: MIT

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
#include <cstring>
#include <hodea/core/cstdint.hpp>
#include <hodea/core/bitmanip.hpp>
#include <hodea/device/hal/device_setup.hpp>
#include <hodea/device/hal/pin_config.hpp>
#include <hodea/device/hal/retarget_stdout_uart.hpp>
#include <hodea/device/hal/cpu.hpp>
#include <hodea/device/hal/bls.hpp>
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

constexpr Htsc_timer::Ticks no_activity_timeout =
    Htsc_timer::sec_to_ticks(10);

/**
 * Turn on clocks for peripherals used in the application.
 */
static void init_peripheral_clocks()
{
    set_bit(RCC->AHBENR, RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN);
    set_bit(RCC->APB2ENR, RCC_APB2ENR_SYSCFGCOMPEN);
    set_bit(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
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
 * 16   PA2/USART2_TX   O       AF1     unused
 * 17   PA3/USART2_RX   I       AF1     unused
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
static void init_pins()
{
    // Configure pin af register.
    Config_gpio_af{GPIOA}
        .pin(2, Gpio_pin_af::af1)
        .pin(2, Gpio_pin_af::af1)
        .write();

    // Configure pin mode register.
    Config_gpio_mode{GPIOA}
        .pin(2, Gpio_pin_mode::af)
        .pin(3, Gpio_pin_mode::af)
        .pin(5, Gpio_pin_mode::output)
        .pin(13, Gpio_pin_mode::af)
        .pin(14, Gpio_pin_mode::af)
        .write();
}

/**
 * Conditionally initialize boot_data.
 *
 * The \a boot_data is used to pass information from the application
 * to the bootloader in the case a firmware update is requested.
 *
 * Therefore, the \a boot_data is persistent. It is initialized with 0
 * under the following conditions:
 *
 * - The bootloader is entered due to hardware related resets, e.g.
 *   power-on reset, watchdog, etc.
 * - The bootloader is entered without firmware update request being set.
 *
 * \note
 * On ST devices a software reset causes the reset pin to be asserted in
 * order to reset the external circuit. Therefore, Reset_cause::software
 * and Reset_cause::reset_pin are set in this case when we query the
 * reset cause.
 */
static void init_boot_data(void)
{
    Reset_cause::Type rst;

    rst = get_reset_cause();
    clear_reset_causes();

    /*
     * Software reset flag and PIN reset flag always come together when
     * a reset is triggered by software.
     */
    Reset_cause::Type sw = Reset_cause::software | Reset_cause::reset_pin;
    if ((rst == sw) && is_update_requested())
        return;     // skip initialization
        
    std::memset(&boot_data, 0, sizeof(boot_data));
}

/**
 * Minimum required board initialization.
 *
 * This function sets up the minimum required board configuration,
 * regardless of whether we subsequently have to fall into bootloader mode
 * or jump directly into the application code.
 */
static void init_minimum()
{
    init_peripheral_clocks();
    init_pins();
    init_boot_data();
}

/**
 * Main initialization when falling into bootloader mode.
 */
static void init()
{
    retarget_init(USART2, baud_to_brr(115200));
    rte_init();
}

/**
 * De-initialization to bring board into a safe state.
 */
static void deinit()
{
    rte_deinit();
    retarget_deinit();
}

/**
 * Test if application code is valid.
 */
static bool is_appl_valid()
{
    if (!is_appl_info_sane())
        return false;

    uint32_t crc;

    crc = bls_progmem_crc(
                &appl_info.version,
                reinterpret_cast<uint32_t*>(appl_end_addr & ~3U)
                );

    boot_data.appl_crc = crc;

    if ((crc == appl_info.crc) ||
        (appl_info.ignore_crc == ignore_appl_crc_key))
        return true;

    return false;
}

#if defined __ARMCC_VERSION && (__ARMCC_VERSION >= 6010050)
// Build works with -O3, but fails with -O0.
// This is the workaround proposed in support case 710226
__asm(".global __ARM_use_no_argv\n");
#endif

[[noreturn]] int main()
{
    init_minimum();

    if (!is_update_requested() && is_appl_valid())
       enter_application();

    init();

    printf("bootloader mode entered\n");

    std::printf(
        "appl_info.crc = 0x%08lx, boot_data.crc = 0x%08lx\n",
        appl_info.crc, boot_data.appl_crc
        );

    Htsc::Ticks ts_led = 0;
    Htsc_timer exit_timer;

    exit_timer.start(no_activity_timeout);
    do {
        kick_watchdog();
        exit_timer.update();

        if (Htsc::is_elapsed_repetitive(ts_led, Htsc::ms_to_ticks(50)))
            run_led.toggle();

        /*
         * Additional code implementing the firmware update come here.
         * :
         */
    } while (!exit_timer.is_expired());

    reset_update_request();

    deinit();
    software_reset();
}
