/*

Copyright 2015-2019 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "board/Board.h"
#include "board/Internal.h"
#include "board/common/io/Helpers.h"
#include "Pins.h"

namespace Board
{
    namespace detail
    {
        namespace io
        {
            void indicateBtldr()
            {
#if defined(LED_INDICATORS)
                CORE_IO_CONFIG(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN, core::io::pinMode_t::output);
                INT_LED_ON(LED_MIDI_IN_DIN_PORT, LED_MIDI_IN_DIN_PIN);
                INT_LED_ON(LED_MIDI_OUT_DIN_PORT, LED_MIDI_OUT_DIN_PIN);
#endif

#if defined(NUMBER_OF_OUT_SR) && !defined(NUMBER_OF_LED_COLUMNS)
                //turn on all available LEDs
                CORE_IO_CONFIG(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN, core::io::pinMode_t::output);
                CORE_IO_CONFIG(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN, core::io::pinMode_t::output);

                CORE_IO_SET_LOW(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);

                for (int i = 0; i < NUMBER_OF_OUT_SR; i++)
                {
                    for (int j = 0; j < 8; j++)
                    {
                        EXT_LED_ON(SR_OUT_DATA_PORT, SR_OUT_DATA_PIN);
                        CORE_IO_SET_LOW(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                        _NOP();
                        _NOP();
                        CORE_IO_SET_HIGH(SR_OUT_CLK_PORT, SR_OUT_CLK_PIN);
                    }
                }

                CORE_IO_SET_HIGH(SR_OUT_LATCH_PORT, SR_OUT_LATCH_PIN);
//make sure internal led is turned off for mega/uno
#elif defined(OD_BOARD_MEGA)
                CORE_IO_CONFIG(PORTB, 7, core::io::pinMode_t::output);
                CORE_IO_SET_LOW(PORTB, 7);
#elif defined(OD_BOARD_UNO)
                CORE_IO_CONFIG(PORTB, 5, core::io::pinMode_t::output);
                CORE_IO_SET_LOW(PORTB, 5);
#elif defined(OD_BOARD_TEENSY2PP)
                //only one led
                CORE_IO_CONFIG(LED_IN_PORT, LED_IN_PIN, core::io::pinMode_t::output);
                INT_LED_ON(LED_IN_PORT, LED_IN_PIN);
#endif
            }

            bool isBtldrButtonActive()
            {
#if defined(BTLDR_BUTTON_INDEX)
                CORE_IO_SET_LOW(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
                CORE_IO_SET_LOW(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);
                _NOP();

                CORE_IO_SET_HIGH(SR_DIN_LATCH_PORT, SR_DIN_LATCH_PIN);

                for (int j = 0; j < NUMBER_OF_IN_SR; j++)
                {
                    for (int i = 0; i < NUMBER_OF_IN_SR_INPUTS; i++)
                    {
                        uint8_t index = (7 - i) + j * NUMBER_OF_OUT_SR_INPUTS;

                        if (index == BTLDR_BUTTON_INDEX)
                        {
                            if (!CORE_IO_READ(SR_DIN_DATA_PORT, SR_DIN_DATA_PIN))
                                return true;
                        }

                        CORE_IO_SET_LOW(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
                        _NOP();
                        CORE_IO_SET_HIGH(SR_DIN_CLK_PORT, SR_DIN_CLK_PIN);
                    }
                }

                return false;
#elif defined(BTLDR_BUTTON_PORT)
                return !CORE_IO_READ(BTLDR_BUTTON_PORT, BTLDR_BUTTON_PIN);
#else
                //no hardware entry possible in this case
                return false;
#endif
            }
        }    // namespace io
    }        // namespace detail
}    // namespace Board