/*
*	KeyReader.ino, Copyright Jonathan Mackey 2025
*
*	GNU license:
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*	Please maintain this license information along with authorship and copyright
*	notices in any redistribution of this code.
*/
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "KeyReaderSTM32.h"

KeyReaderSTM32	keyReader;

#if 0
#define xstr(s) str(s)
#define str(s) #s

#pragma message "The value of PIN_SPI_MOSI: " xstr(PIN_SPI_MOSI)
#pragma message "The value of PIN_SPI_MISO: " xstr(PIN_SPI_MISO)
#pragma message "The value of PIN_SPI_SCK: " xstr(PIN_SPI_SCK)
#pragma message "The value of PB3: " xstr(PB3)
#pragma message "The value of PB4: " xstr(PB4)
#pragma message "The value of PB5: " xstr(PB5)
#pragma message "The value of PA2: " xstr(PA2)
#pragma message "The value of PA3: " xstr(PA3)

#pragma message "The value of PIN_WIRE_SDA: " xstr(PIN_WIRE_SDA)
#pragma message "The value of PB6: " xstr(PB6)
#pragma message "The value of PB7: " xstr(PB7)
#pragma message "The value of PB8: " xstr(PB8)
#pragma message "The value of PB9: " xstr(PB9)
#pragma message "The value of PB10: " xstr(PB10)
#pragma message "The value of HSE_VALUE: " xstr(HSE_VALUE)
#pragma message "The value of PA2: " xstr(PA2)
#pragma message "The value of PIN_SERIAL_TX: " xstr(PIN_SERIAL_TX)
#pragma message "The value of PA_3: " xstr(PA_3)
#pragma message "The value of PIN_SERIAL_RX: " xstr(PIN_SERIAL_RX)
#endif

/*********************************** setup ************************************/
void setup(void)
{	
	Serial.begin(BAUD_RATE);
	Serial.printf(".Starting...\n");
	keyReader.begin();
}

/************************************ loop ************************************/
void loop(void)
{
	keyReader.Update();
}
