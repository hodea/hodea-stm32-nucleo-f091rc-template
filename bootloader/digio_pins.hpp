// Copyright (c) 2017, Franz Hollerer. All rights reserved.
// This code is licensed under the MIT License (MIT).
// See LICENSE file for full details.

/**
 * Digital I/O pins used throughout the project.
 */
#if !defined _BOOT_DIGIO_PINS_HPP_
#define _BOOT_DIGIO_PINS_HPP_

#include <hodea/device/hal/digio.hpp>

constexpr hodea::Digio_output run_led{GPIOA_BASE, 5};

#endif /*!_BOOT_DIGIO_PINS_HPP_ */
