// Copyright (c) 2017, Franz Hollerer.
// SPDX-License-Identifier: MIT

/**
 * Digital I/O pins used throughout the project.
 */
#if !defined DIGIO_PINS_HPP
#define DIGIO_PINS_HPP

#include <hodea/device/hal/digio.hpp>

constexpr hodea::Digio_output run_led{GPIOA_BASE, 5};

class User_button : public hodea::Digio_input {
public:
    constexpr User_button(uintptr_t port, int pin)
        : Digio_input(port, pin) {}

    bool is_pressed() const
    {
        return !value();
    }
};

constexpr User_button user_button{GPIOC_BASE, 13};

#endif /*!DIGIO_PINS_HPP */
