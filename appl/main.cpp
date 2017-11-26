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

using namespace hodea;

/**
 * Turn on clocks for peripherals used in the application.
 */
static void init_peripheral_clocks(void)
{
    // enable clocks for all GPIO ports and DMA1
    set_bit(
        RCC->AHBENR,
        RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN |
        RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOFEN |
#if defined RCC_AHBENR_DMA1EN
        RCC_AHBENR_DMA1EN
#else
        RCC_AHBENR_DMAEN
#endif
        );

    // enable clock for USART1 and SPI1
    set_bit(RCC->APB2ENR, RCC_APB2ENR_USART1EN | RCC_APB2ENR_SPI1EN);

    // enable clock for USART2 and USART3
    set_bit(RCC->APB1ENR, RCC_APB1ENR_USART2EN | RCC_APB1ENR_USART3EN);
}

/**
 * General Purpose I/O pin configuration.
 *
 * Pin  Name            Dir     AF      Function
 * 1    VBAT
 * 2    PC13            I               B_USER, blue user push button
 * 3    PC14            I               unused, reserved for RCC_OSC32_IN
 * 4    PC15            I               unused, reserved for RCC_OSC32_OUT
 * 5    PF0             I               unused, reserved for RCC_OSC_IN
 * 6    PF1             I               unused, reserved for RCC_OUS_OUT
 * 7    NRST            I/O
 * 8    PC0             O               DIGIO_OC_OUT1, open collector
 * 9    PC1             O               DIGIO_OC_OUT2, open collector
 * 10   PC2             I               DIGIO_IN1
 * 11   PC3             I               unused
 * 12   VSSA
 * 13   VDDA
 * 14   PA0/SYS_WKUP1   I               wakeup pin WKUP1
 * 15   PA1             I               unused
 *
 * 16   PA2/USART2_TX   O               Usart2 TX output
 *      + nucleo-f0915c         AF1
 *      + nucleo-f334r8         AF7
 * 17   PA3/USART2_RX   I               Usart2 RX input
 *      + nucleo-f0915c         AF1
 *      + nucleo-f334r8         AF7
 * 18   VSS
 * 19   VDD
 * 20   PA4/SPI1_NSS    O               SPI1 slave select
 *      + nucleo-f0915c         AF0
 *      + nucleo-f334r8         AF5
 * 21   PA5             O               RUN_LED
 * 22   PA6/SPI1_MISO   I               SPI master in slave out
 *      + nucleo-f0915c         AF0
 *      + nucleo-f334r8         AF5
 * 23   PA7/SPI1_MOSI   O               SPI master out slave in
 *      + nucleo-f0915c         AF0
 *      + nucleo-f334r8         AF5
 * 24   PC4             I               unused
 * 25   PC5             I               unused
 * 26   PB0             I               unused
 * 27   PB1             I               unused
 * 28   PB2             I               unused
 * 29   PB10/USART3_TX  O               Usart3 TX output
 *      + nucleo-f0915c         AF4
 *      + nucleo-f334r8         AF7
 * 30   PB11/USART3_RX  I               Usart3 RX input
 *      + nucleo-f0915c         AF4
 *      + nucleo-f334r8         AF7
 * 31   VSS
 * 32   VDD
 * 33   PB12            I               unused
 * 34   PB13            I               unused
 * 35   PB14/USART3_DE  O               Usart3 driver enable output
 *      + nucleo-f0915c         AF4
 *      + nucleo-f334r8         AF7
 * 36   PB15            I               unused
 * 37   PC6             I               unused
 * 38   PC7             I               unused
 * 39   PC8             O               USER_LED
 * 40   PC9             O               FINISHED_LED
 * 41   PA8             I               ERROR_LED
 * 42   PA9/USART1_TX   O               Usart1 TX output
 *      + nucleo-f0915c         AF1
 *      + nucleo-f334r8         AF7
 * 43   PA10/USART1_RX  I               Usart1 RX input
 *      + nucleo-f0915c         AF1
 *      + nucleo-f334r8         AF7
 * 44   PA11/USART1_CTS I               Usart1 CTS input
 *      + nucleo-f0915c         AF1
 *      + nucleo-f334r8         AF7
 * 45   PA12/USART1_RTS O               Usart1 RTS output 
 *      + nucleo-f0915c         AF1
 *      + nucleo-f334r8         AF7
 * 46   PA13/SYS_SWDIO  O       AF0     Serial wire debug, data I/O
 * 47   VSS
 * 48   VDD
 * 49   PA14/SYS_SWCLK  I       AF0     Serial wire debug, clock input
 * 50   PA15            I               unused
 * 51   PC10            O               Debug pin
 * 52   PC11            I               unused
 * 53   PC12            I               unused
 * 54   PD2             I               unused
 * 55   PB3/SPI1_SCK    O               SPI serial clock
 *      + nucleo-f0915c         AF0
 *      + nucleo-f334r8         AF5
 * 56   PB4             I               unused
 * 57   PB5/I2C1_SMBA   O               SMB alert
 *      + nucleo-f0915c         AF3
 *      + nucleo-f334r8         AF4
 * 58   PB6/ISC1_SCL    I               I2C clock
 *      + nucleo-f0915c         AF1
 *      + nucleo-f334r8         AF4
 * 59   PB7/I2C1_SDA    I/O             I2C serial data
 *      + nucleo-f0915c         AF1
 *      + nucleo-f334r8         AF4
 * 60   BOOT0                           Boot memory selection
 * 61   PB8             I               unused
 * 62   PB9/TIM17_CH1   O               Tim17 PWM output
 *      + nucleo-f0915c         AF2
 *      + nucleo-f334r8         AF1
 * 63   VSS
 * 64   VDD
 */
static void init_pins(void)
{
    // Configure pin alternate function register.
    Config_gpio_af{GPIOA}
        .pin(2, Gpio_pin_af::af1)
        .pin(3, Gpio_pin_af::af1)
        .pin(4, Gpio_pin_af::af0)
        .pin(6, Gpio_pin_af::af0)
        .pin(7, Gpio_pin_af::af0)
        .pin(9, Gpio_pin_af::af1)
        .pin(10, Gpio_pin_af::af0)
        .pin(11, Gpio_pin_af::af1)
        .pin(12, Gpio_pin_af::af1)
        .pin(13, Gpio_pin_af::af0)
        .pin(14, Gpio_pin_af::af0)
        .write();
    Config_gpio_af{GPIOB}
        .pin(3, Gpio_pin_af::af0)
        .pin(5, Gpio_pin_af::af3)
        .pin(6, Gpio_pin_af::af1)
        .pin(7, Gpio_pin_af::af1)
        .pin(9, Gpio_pin_af::af2)
        .pin(10, Gpio_pin_af::af4)
        .pin(11, Gpio_pin_af::af4)
        .pin(14, Gpio_pin_af::af4)
        .write();
    Config_gpio_af{GPIOC}.write();
    Config_gpio_af{GPIOD}.write();
    Config_gpio_af{GPIOF}.write();

    /*
     * Configure output type register.
     * PC0 and PC1 must be configured as open drain. All others output
     * are kept in push/pull mode.
     */
    Config_gpio_otype{GPIOC}
        .pin(0, Gpio_pin_otype::open_drain)
        .pin(1, Gpio_pin_otype::open_drain)
        .write();

    // Configure pin mode register.
    Config_gpio_mode{GPIOA}
        .pin(0, Gpio_pin_mode::input)
        .pin(1, Gpio_pin_mode::input)
        .pin(2, Gpio_pin_mode::af)
        .pin(3, Gpio_pin_mode::af)
        .pin(4, Gpio_pin_mode::af)
        .pin(5, Gpio_pin_mode::output)
        .pin(6, Gpio_pin_mode::af)
        .pin(7, Gpio_pin_mode::af)
        .pin(8, Gpio_pin_mode::output)
        .pin(9, Gpio_pin_mode::af)
        .pin(10, Gpio_pin_mode::af)
        .pin(11, Gpio_pin_mode::af)
        .pin(12, Gpio_pin_mode::af)
        .pin(13, Gpio_pin_mode::af)
        .pin(14, Gpio_pin_mode::af)
        .pin(15, Gpio_pin_mode::input)
        .write();

    Config_gpio_mode{GPIOB}
        .pin(0, Gpio_pin_mode::input)
        .pin(1, Gpio_pin_mode::input)
        .pin(2, Gpio_pin_mode::input)
        .pin(3, Gpio_pin_mode::af)
        .pin(4, Gpio_pin_mode::input)
        .pin(5, Gpio_pin_mode::af)
        .pin(6, Gpio_pin_mode::af)
        .pin(7, Gpio_pin_mode::af)
        .pin(8, Gpio_pin_mode::input)
        .pin(9, Gpio_pin_mode::af)
        .pin(10, Gpio_pin_mode::af)
        .pin(11, Gpio_pin_mode::af)
        .pin(12, Gpio_pin_mode::input)
        .pin(13, Gpio_pin_mode::input)
        .pin(14, Gpio_pin_mode::af)
        .pin(15, Gpio_pin_mode::input)
        .write();

    Config_gpio_mode{GPIOC}
        .pin(0, Gpio_pin_mode::output)
        .pin(1, Gpio_pin_mode::output)
        .pin(2, Gpio_pin_mode::input)
        .pin(3, Gpio_pin_mode::input)
        .pin(4, Gpio_pin_mode::input)
        .pin(5, Gpio_pin_mode::input)
        .pin(6, Gpio_pin_mode::input)
        .pin(7, Gpio_pin_mode::input)
        .pin(8, Gpio_pin_mode::output)
        .pin(9, Gpio_pin_mode::output)
        .pin(10, Gpio_pin_mode::output)
        .pin(11, Gpio_pin_mode::input)
        .pin(12, Gpio_pin_mode::input)
        .pin(13, Gpio_pin_mode::input)
        .pin(14, Gpio_pin_mode::input)
        .pin(15, Gpio_pin_mode::input)
        .write();

    Config_gpio_mode{GPIOD}.write();    // all input
    Config_gpio_mode{GPIOF}.write();    // all input
}

#if defined __ARMCC_VERSION && (__ARMCC_VERSION >= 6010050)
// Build works with -O3, but fails with -O0.
// This is the workaround proposed in support case 710226:w
__asm(".global __ARM_use_no_argv\n");
#endif

int main()
{
    init_peripheral_clocks();
    init_pins();
    rte_init();

    return 0;
}
