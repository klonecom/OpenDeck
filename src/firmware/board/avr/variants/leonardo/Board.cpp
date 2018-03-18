/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2018 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Board.h"

///
/// \brief Placeholder variable used only to reserve space in linker section.
///
const uint32_t appLength __attribute__ ((section (".applen"))) __attribute__((used)) = 0;

///
/// \brief Location at which size of application is written in flash.
///
#define APP_LENGTH_LOCATION         (uint32_t)0x000000AC

Board::Board()
{
    //default constructor
}

void Board::reboot(rebootType_t type)
{
    switch(type)
    {
        case rebootApp:
        eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, APP_REBOOT_VALUE);
        break;

        case rebootBtldr:
        eeprom_write_byte((uint8_t*)REBOOT_VALUE_EEPROM_LOCATION, BTLDR_REBOOT_VALUE);
        break;
    }

    mcuReset();
}

///
/// \brief Checks if firmware has been updated.
/// Firmware file has written CRC in last two flash addresses. Application stores last read CRC in EEPROM. If EEPROM and flash CRC differ, firmware has been updated.
///
bool Board::checkNewRevision()
{
    //current app crc is written in ".applen" linker section
    //previous crc is stored into eeprom
    //if two differ, app has changed

    uint32_t flash_size = pgm_read_dword(APP_LENGTH_LOCATION);
    uint16_t crc_eeprom = eeprom_read_word((uint16_t*)SW_CRC_LOCATION_EEPROM);
    uint16_t crc_flash = pgm_read_word(flash_size);

    if (crc_eeprom != crc_flash)
    {
        eeprom_update_word((uint16_t*)SW_CRC_LOCATION_EEPROM, crc_flash);
        return true;
    }

    return false;
}

void Board::ledFlashStartup(bool fwUpdated)
{
    for (int i=0; i<3; i++)
    {
        if (fwUpdated)
        {
            BTLDR_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
            BTLDR_LED_OFF(LED_IN_PORT, LED_IN_PIN);
            _delay_ms(200);
            BTLDR_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
            BTLDR_LED_ON(LED_IN_PORT, LED_IN_PIN);
            _delay_ms(200);
        }
        else
        {
            BTLDR_LED_ON(LED_OUT_PORT, LED_OUT_PIN);
            BTLDR_LED_ON(LED_IN_PORT, LED_IN_PIN);
            _delay_ms(200);
            BTLDR_LED_OFF(LED_OUT_PORT, LED_OUT_PIN);
            BTLDR_LED_OFF(LED_IN_PORT, LED_IN_PIN);
            _delay_ms(200);
        }
    }
}

Board board;
