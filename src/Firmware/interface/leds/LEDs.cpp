/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015, 2016 Igor Petrovic

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

#include "LEDs.h"
#include "../../board/Board.h"

LEDs::LEDs()
{
    //def const
}

void LEDs::init()
{
    setBlinkTime(database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterBlinkTime));
    setFadeTime(database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterFadeTime));

    //run LED animation on start-up
    startUpAnimation();
}

void LEDs::startUpAnimation()
{
    if (!database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterTotalLEDnumber) || !database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterStartUpSwitchTime))
        return;

    //turn off all LEDs before starting animation
    setAllOff();

    //switch (database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterStartUpRoutine))
    //{
        //case 1:
        //oneByOne(true, true, true);
        //oneByOne(false, false, true);
        //oneByOne(true, false, false);
        //oneByOne(false, true, true);
        //oneByOne(true, false, true);
        //oneByOne(false, false, false);
        //break;
//
        //case 2:
        //oneByOne(true, false, true);
        //oneByOne(false, false, false);
        //break;
//
        //case 3:
        //oneByOne(true, true, true);
        //oneByOne(false, true, true);
        //break;
//
        //case 4:
        //oneByOne(true, false, true);
        //oneByOne(true, false, false);
        //break;
//
        //case 5:
        //oneByOne(true, false, true);
        //break;
//
        //default:
        //break;
    //}

    setAllOff();
    wait(1000);
}

rgbValue_t LEDs::velocityToColor(uint8_t receivedVelocity, bool blinkEnabled)
{
    /*
    blinkEnabled:
    constant:
    0-7 off
    8-15 white
    16-23 cyan
    24-31 magenta
    32-39 red
    40-47 blue
    48-55 yellow
    56-63 green
    blink:
    64-71 off
    72-79 white
    80-87 cyan
    88-95 magenta
    96-103 red
    104-111 blue
    112-119 yellow
    120-127 green

    blinkDisabled:
    constant only:
    0-15 off
    16-31 white
    32-47 cyan
    48-63 magenta
    64-79 red
    80-95 blue
    96-111 yellow
    112-127 green
    */

    ledColor_t color;

    switch(blinkEnabled)
    {
        case false:
        color = (ledColor_t)(receivedVelocity/16);
        break;

        case true:
        if (receivedVelocity > 63)
            receivedVelocity -= 64;
        color = (ledColor_t)(receivedVelocity/8);
        break;

    }

    return colors[(uint8_t)color];
}

bool LEDs::velocityToblinkState(uint8_t receivedVelocity)
{
    return (receivedVelocity > 63);
}

void LEDs::noteToState(uint8_t receivedNote, uint8_t receivedVelocity, bool ledID, bool local)
{
    bool blinkEnabled_global = database.read(CONF_BLOCK_LED, ledHardwareParameterSection, ledHwParameterBlinkTime);
    bool blinkEnabled_led;
    if (!blinkEnabled_global)
        blinkEnabled_led = false;
    else
        blinkEnabled_led = velocityToblinkState(receivedVelocity);

    rgbValue_t color = velocityToColor(receivedVelocity, blinkEnabled_global);

    if (!ledID)
    {
        //match LED activation note with its index
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        {
            if (database.read(CONF_BLOCK_LED, ledActivationNoteSection, i) == receivedNote)
            {
                if (database.read(CONF_BLOCK_LED, ledRGBenabledSection, i))
                {
                    if (local)
                    {
                        //if local is set to true, check if local led control is enabled for this led before changing state
                        if (database.read(CONF_BLOCK_LED, ledLocalControlSection, i))
                            setRGBled(i, color, blinkEnabled_led);
                    }
                    else
                    {
                        setRGBled(i, color, blinkEnabled_led);
                    }
                }
                else
                {
                    bool state = color.r || color.g || color.b;
                    if (local)
                    {
                        if (database.read(CONF_BLOCK_LED, ledLocalControlSection, i))
                            setSingleLED(i, state, blinkEnabled_led);
                    }
                    else
                    {
                        setSingleLED(i, state, blinkEnabled_led);
                    }
                }
            }
        }
    }
    else
    {
        //treat received note as led ID
        //we can ignore local argument here
        if (database.read(CONF_BLOCK_LED, ledRGBenabledSection, receivedNote))
        {
            //rgb led
            setRGBled(receivedNote, color, blinkEnabled_led);
        }
        else
        {
            bool state = color.r || color.g || color.b;
            setSingleLED(receivedNote, state, blinkEnabled_led);
        }
    }
}

void LEDs::setAllOn()
{
    //turn on all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        setSingleLED(i, true, false);
}

void LEDs::setAllOff()
{
    //turn off all LEDs
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
        setSingleLED(i, true, false);
}

void LEDs::setState(uint8_t ledNumber, rgbValue_t color)
{
    setRGBled(ledNumber, color, false);
}

void LEDs::setState(uint8_t ledNumber, bool state)
{
    setSingleLED(ledNumber, state, false);
}

void LEDs::setSingleLED(uint8_t ledNumber, bool state, bool blinkMode)
{
    handleLED(ledNumber, state, blinkMode);

    if (blinkMode && state)
        startBlinking();
    else
        checkBlinkLEDs();
}

void LEDs::setRGBled(uint8_t ledNumber, rgbValue_t color, bool blinkMode)
{
    handleLED(ledNumber, color, blinkMode);

    if (blinkMode && color.r && color.g && color.b)
        startBlinking();
    else
        checkBlinkLEDs();
}

uint8_t LEDs::getState(uint8_t ledNumber)
{
    uint8_t returnValue;
    returnValue = ledState[ledNumber];
    return returnValue;
}

bool LEDs::allLEDsOn()
{
    //return true if all LEDs are on
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        if (getState(i))
            return false;
    }

    return true;
}

bool LEDs::allLEDsOff()
{
    //return true if all LEDs are off
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        if (!getState(i))
            return false;
    }

    return true;
}

void LEDs::setFadeTime(uint8_t transitionSpeed)
{
    //reset transition counter
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
            transitionCounter[i] = 0;

        pwmSteps = transitionSpeed;
    }
}

void LEDs::setBlinkTime(uint16_t blinkTime)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        ledBlinkTime = blinkTime*100;
        blinkTimerCounter = 0;
    }
}

void LEDs::handleLED(uint8_t ledNumber, bool state, bool blinkMode)
{
    /*

    LED state is stored into one byte (ledState). The bits have following meaning (7 being the MSB bit):

    7: x
    6: x
    5: x
    4: x
    3: Blink bit (timer changes this bit)
    2: LED is active (either it blinks or it's constantly on), this bit is OR function between bit 0 and 1
    1: LED blinks
    0: LED is constantly turned on

    */

    uint8_t currentState = getState(ledNumber);

    switch(state) {

        case false:
        //turn off the led
        currentState = 0;
        break;

        case true:
        //turn on the led
        //if led was already active, clear the on bits before setting new state
        if (bitRead(currentState, LED_ACTIVE_BIT))
            currentState = 0;

        bitSet(currentState, LED_ACTIVE_BIT);
        if (blinkMode)
        {
            bitSet(currentState, LED_BLINK_ON_BIT);
            //this will turn the led immediately no matter how little time it's
            //going to blink first time
            bitSet(currentState, LED_BLINK_STATE_BIT);
        }
        else
        {
            bitSet(currentState, LED_CONSTANT_ON_BIT);
        }
        break;
    }

    ledState[ledNumber] = currentState;
}

void LEDs::handleLED(uint8_t ledNumber, rgbValue_t color, bool blinkMode)
{
    uint8_t led1 = board.getRGBaddress(ledNumber, rgb_R);
    uint8_t led2 = board.getRGBaddress(ledNumber, rgb_G);
    uint8_t led3 = board.getRGBaddress(ledNumber, rgb_B);

    handleLED(led1, color.r, blinkMode);
    handleLED(led2, color.g, blinkMode);
    handleLED(led3, color.b, blinkMode);
}

void LEDs::startBlinking()
{
    if (!blinkEnabled)
    {
        blinkEnabled = true;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            blinkState = true;
            blinkTimerCounter = 0;
        }
    }
}

void LEDs::stopBlinking()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        blinkState = true;
        blinkTimerCounter = 0;
    }
    blinkEnabled = false;
}

bool LEDs::blinkingActive()
{
    bool state;
    state = blinkEnabled;
    return state;
}

void LEDs::checkBlinkLEDs()
{
    //this function will disable blinking
    //if none of the LEDs is in blinking state

    //else it will enable it

    bool _blinkEnabled = false;
    uint8_t ledState;

    //if any LED is blinking, set timerState to true and exit the loop
    for (int i=0; i<MAX_NUMBER_OF_LEDS; i++)
    {
        ledState = getState(i);
        if (bitRead(ledState, LED_BLINK_ON_BIT))
        {
            _blinkEnabled = true;
            break;
        }
    }

    if (_blinkEnabled)
    {
        startBlinking();
    }
    else if (!_blinkEnabled && blinkingActive())
    {
        //don't bother reseting variables if blinking is already disabled
        //reset blinkState to default value
        stopBlinking();
    }
}

LEDs leds;
