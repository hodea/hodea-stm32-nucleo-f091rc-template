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
 * This function sets up the internal oscillator and PLL to provide
 * a 16 MHz system clock.
 *
 * \note
 * We come here due to a hardware or software reset and therefore
 * can be sure that HSI is active.
 */
extern "C" void SystemInit(void);
void SystemInit(void)
{
    /*
     * Set flash wait states
     * Reference Manual:
     * LATENCY[2:0]: Latency
     * These bits represent the ratio of the SYSCLK (system clock)
     * period to the Flash access time.
     *   000: Zero wait state, if 0 < SYSCLK <= 24 MHz
     *   001: One wait state, if 24 MHz < SYSCLK <= 48 MHz
     * Note:
     * The prefetch buffer has an impact on the performance only when the
     * wait state number is 1.
     */
     FLASH->ACR =
         _VAL2FLD(FLASH_ACR_PRFTBE, 0) |    // prefetch buffer disabled
         _VAL2FLD(FLASH_ACR_LATENCY, 0);    // zero wait states

    /*
     * Clock configuration.
     */
    RCC->CFGR =
        RCC_CFGR_MCO_NOCLOCK |      // no Microcontroller Clock Output
        RCC_CFGR_PLLMUL4 |          // PLL x4
        RCC_CFGR_PLLSRC_HSI_DIV2 |  // HSI clock divided by 2 as PLL entry
        RCC_CFGR_PPRE_DIV1 |        // APB1 prescaler: HCLK not divided
        RCC_CFGR_HPRE_DIV1 |        // AHB prescaler: SYSCLK not divided
        RCC_CFGR_SW_HSI;            // keep HSI till PLL is running

    /*
     * Turn on PLL and wait till it is ready.
     */
    set_bit(RCC->CR, RCC_CR_PLLON);
    while (!is_bit_set(RCC->CR, RCC_CR_PLLRDY)) ;

    /*
     * Select PLL output as system clock and wait till the switch
     * is finished.
     */
    set_bit(RCC->CFGR, RCC_CFGR_SW_PLL);
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) ;
}
