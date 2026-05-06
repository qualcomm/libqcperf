/*
        Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
        Redistribution and use in source and binary forms, with or without
        modification, are permitted (subject to the limitations in the
        disclaimer below) provided that the following conditions are met:
                * Redistributions of source code must retain the above copyright
                  notice, this list of conditions and the following disclaimer.
                * Redistributions in binary form must reproduce the above
                  copyright notice, this list of conditions and the following
                  disclaimer in the documentation and/or other materials provided
                  with the distribution.
                * Neither the name of Qualcomm Technologies, Inc. nor the names of its
                  contributors may be used to endorse or promote products derived
                  from this software without specific prior written permission.
        NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
        GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
        HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
        WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
        MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
        IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
        ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
        IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
        OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
        IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file qsleep.c
 * @brief Implementation of cross-platform sleep utility functions
 * @author Skand Gupta (skangupt@qti.qualcomm.com)
 *
 * This file implements the sleep utility functions defined in qsleep.h,
 * providing a cross-platform way to suspend thread execution for specified
 * durations. The implementation automatically selects the appropriate
 * platform-specific sleep function based on the target operating system.
 *
 * For Windows platforms, it uses the Sleep() function from the Windows API.
 * For Linux/POSIX platforms, it uses sleep() for second-level precision and
 * usleep() for millisecond-level precision.
 */

#include "qsleep.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void sleep_ms(uint32_t duration_in_ms) {
#ifdef _WIN32
    Sleep(duration_in_ms);
#else
    if (duration_in_ms >= 1000) {
        sleep(duration_in_ms / 1000);
    }
    usleep((duration_in_ms % 1000) * 1000);
#endif
}

void sleep_sec(uint16_t duration_in_sec) {
#ifdef _WIN32
    Sleep(duration_in_sec * 1000);
#else
    sleep(duration_in_sec);
#endif
}
