// Copyright (c) 2017, Franz Hollerer.
// SPDX-License-Identifier: MIT

/**
 * Option bytes.
 * \author f.hollerer@gmx.net
 */
#include <hodea/core/cstdint.hpp>
#include <hodea/core/bitmanip.hpp>

using namespace hodea;

constexpr uint16_t add_complement(unsigned v)
{
    return ((~v & 0xff) << 8) | v;
}

struct Option_bytes {
    uint16_t rdp;
    uint16_t user;
    uint16_t data0;
    uint16_t data1;
    uint16_t wrp0;
    uint16_t wrp1;
    uint16_t wrp2;
    uint16_t wrp3;
};

 const Option_bytes option_bytes 
     __attribute__((section(".option_bytes"), used)) =
{
    .rdp = add_complement(0xaa),
    .user = add_complement(0xff), // bit 0: 0 = hardware, 1 = software watchdog
    .data0 = add_complement(0xff),
    .data1 = add_complement(0xff),
    .wrp0 = add_complement(0xff),
    .wrp1 = add_complement(0xff),
    .wrp2 = add_complement(0xff),
    .wrp3 = add_complement(0xff)
};

