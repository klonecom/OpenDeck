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

#include "Analog.h"
#include "board/Board.h"
#include "core/src/general/BitManipulation.h"

using namespace Interface::analog;

void Analog::update()
{
    if (!Board::interface::analog::isDataAvailable())
        return;

    //check values
    for (int i=0; i<MAX_NUMBER_OF_ANALOG; i++)
    {
        //don't process component if it's not enabled
        if (!database.read(DB_BLOCK_ANALOG, dbSection_analog_enable, i))
            continue;

        int16_t analogData = Board::interface::analog::readValue(i);
        type_t type = static_cast<type_t>(database.read(DB_BLOCK_ANALOG, dbSection_analog_type, i));

        if (type != type_t::button)
        {
            switch(type)
            {
                case type_t::potentiometerControlChange:
                case type_t::potentiometerNote:
                case type_t::nrpn7b:
                case type_t::nrpn14b:
                case type_t::pitchBend:
                checkPotentiometerValue(type, i, analogData);
                break;

                case type_t::fsr:
                checkFSRvalue(i, analogData);
                break;

                default:
                break;
            }
        }
        else
        {
            if (buttonHandler != nullptr)
                (*buttonHandler)(i, analogData);
        }
    }

    Board::interface::analog::continueReadout();
}

void Analog::debounceReset(uint16_t index)
{
    lastDirection[index] = potDirection_t::initial;
    lastAnalogueValue[index] = 0;
    fsrPressed[index] = false;
}

///
/// \param fptr [in]    Pointer to function.
///
void Analog::setButtonHandler(void(*fptr)(uint8_t adcIndex, uint16_t adcValue))
{
    buttonHandler = fptr;
}