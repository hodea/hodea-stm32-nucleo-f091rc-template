// Copyright (c) 2017, Franz Hollerer.
// SPDX-License-Identifier: MIT

/**
 * Option bytes.
 * \author f.hollerer@gmx.net
 */
#include <hodea/core/cstdint.hpp>
#include <hodea/core/bitmanip.hpp>

using namespace hodea;

typedef unsigned Bit;

constexpr unsigned compose_byte(
    Bit b7, Bit b6, Bit b5, Bit b4, Bit b3, Bit b2, Bit b1, Bit b0
    )
{
    return b7 << 7 | b6 << 6 | b5 << 5 | b4 << 4 |
           b3 << 3 | b2 << 2 | b1 << 1 | b0 << 0;
}

constexpr unsigned compose_option_byte(
    Bit b7, Bit b6, Bit b5, Bit b4, Bit b3, Bit b2, Bit b1, Bit b0
    )
{
    return (~compose_byte(b7, b6, b5, b4, b3, b2, b1, b0) & 0xffU) << 8 |
             compose_byte(b7, b6, b5, b4, b3, b2, b1, b0);
}

constexpr unsigned compose_option_byte(unsigned v)
{
    return ((~v & 0xffU) << 8) | v;
}

constexpr unsigned rdp_level0 = 0xaa; // unprotected (default)
constexpr unsigned rdp_level1 = 0x55; // read protection
constexpr unsigned rdp_level2 = 0xcc; // no debug; irreversible!!

struct Option_bytes {
    uint16_t rdp;       // read protection
    uint16_t user;      // user options, e.g. watchdog
    uint16_t data0;     // user data
    uint16_t data1;
    uint16_t wrp0;      // write protection
    uint16_t wrp1;
    uint16_t wrp2;
    uint16_t wrp3;
};

const Option_bytes option_bytes 
     __attribute__((section(".option_bytes"), used)) =
{
    .rdp = compose_option_byte(rdp_level0),
    .user = compose_option_byte(1, 1, 1, 1, 1, 1, 1, 1),
    .data0 = compose_option_byte(1, 1, 1, 1, 1, 1, 1, 1),
    .data1 = compose_option_byte(1, 1, 1, 1, 1, 1, 1, 1),
    .wrp0 = compose_option_byte(1, 1, 1, 1, 1, 1, 1, 1),
    .wrp1 = compose_option_byte(1, 1, 1, 1, 1, 1, 1, 1),
    .wrp2 = compose_option_byte(1, 1, 1, 1, 1, 1, 1, 1),
    .wrp3 = compose_option_byte(1, 1, 1, 1, 1, 1, 1, 1)
};

