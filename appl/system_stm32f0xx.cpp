// Copyright (c) 2017, Franz Hollerer.
// SPDX-License-Identifier: MIT

/**
 * System and clock configuration.
 *
 * This file implements the minimum required system and clock configuration
 * functions as specified by CMSIS for the bootloader mode.
 *
 * As a minimum CMSIS requires:
 *
 * - A device-specific system configuration function, \a SystemInit()
 * - A global variable that contains the system frequency,
 *   \a SystemCoreClock.
 *
 * \a SystemInit() is called from the startup code before main() is
 * entered.
 *
 * \sa http://www.keil.com/pack/doc/cmsis/Core/html/group__system__init__gr.html
 *
 * \author f.hollerer@gmx.net
 */

#include <hodea/core/cstdint.hpp>
#include <hodea/core/bitmanip.hpp>
#include <hodea/device/hal/device_setup.hpp>

using namespace hodea;

extern "C" uint32_t SystemCoreClock;
uint32_t SystemCoreClock __attribute__((used)) = config_sysclk_hz;

/**
 * Device specific system configuration called before main is entered.
 *
 * \note
 * The bootloader already performed the low-level configuration, therefore
 * there is nothing to do here.
 */
extern "C" void SystemInit(void);
void SystemInit(void)
{
}
