// Copyright (c) 2017, Franz Hollerer. All rights reserved.
// This code is licensed under the MIT License (MIT).
// See LICENSE file for full details.

/**
 * User specific configuration for the HODEA source code library.
 *
 * \author f.hollerer@gmx.net
 */
#if !defined _BOOT_HODEA_USER_CONFIG_HPP_
#define _BOOT_HODEA_USER_CONFIG_HPP_

#define HODEA_CONFIG_HTSC_TIME_BASE_INCLUDE \
    <hodea/device/arm_cortex_m/htsc_systick_time_base.hpp>

namespace hodea {

 //! System core clock in [Hz].
constexpr unsigned config_sysclk_hz = 16000000;

//! Cortex system timer clock in [Hz].
constexpr unsigned config_systick_hz = config_sysclk_hz / 8;

//! APB1 peripheral clocks in [Hz].
constexpr unsigned config_apb1_pclk_hz = config_sysclk_hz;

//! APB1 timer clocks in [Hz].
constexpr unsigned config_apb1_tclk_hz = config_sysclk_hz;

} // namespace hodea

#endif /*!_BOOT_HODEA_USER_CONFIG_HPP_ */
