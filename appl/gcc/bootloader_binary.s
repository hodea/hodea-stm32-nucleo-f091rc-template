# Copyright (c) 2017, Franz Hollerer.
# SPDX-License-Identifier: MIT

# Helper file to include bootloader image and the option bytes into
# the application binary.
# @author f.hollerer@hodea.org

.section .bootloader,"a",%progbits

.incbin "../../build/boot/bootloader.bin"

.section .option_bytes,"a",%progbits

.incbin "../../build/boot/option_bytes.bin"
