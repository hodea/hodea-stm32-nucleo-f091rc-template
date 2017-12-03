; Copyright (c) 2017, Franz Hollerer.
; SPDX-License-Identifier: MIT

; Helper file to include bootloader image and the option bytes into
; the application binary.
; @author f.hollerer@gmx.net

	AREA bootloader_sec, CODE, READONLY
        EXPORT __bootloader_image

__bootloader_image
|__tagsym$$used|
        ; Note: The bootloader project must be built first so that
        ; the bootloader binary exists for including.
	INCBIN ..\build\bootloader\bootloader_image.bin\ER_BOOT

	AREA option_bytes_sec, READONLY
        EXPORT __option_bytes

__option_bytes
||__tagsym$$used||
        ; Note: The bootloader project must be built first so that
        ; the bootloader binary exists for including.
	INCBIN ..\build\bootloader\bootloader_image.bin\OPTION_BYTES

        KEEP |__tagsym$$used|
        KEEP ||__tagsym$$used||

	END
