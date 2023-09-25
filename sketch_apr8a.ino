/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// Lesen von PGNs vom NMEA2000-Bus
// Version 0.1, 08.01.2021, AK-Homberger

#include <cstdarg>
#// Demo: NMEA2000 library. Bus listener. Sends all bus data to serial in Actisense format.
// Note! If you use Arduino Mega, it can not handle lot of messages without using interrupt.
// enable interrupt by defining pin you have used for MCP2515 interrupt pin. See below
// definition #define N2k_CAN_INT_PIN 21
//
// Now demo also blinks led for each message and starts slow blink, if no messages has been received.
// Do not confuse with led blinking code


#define USE_N2K_CAN 1
#define N2k_SPI_CS_PIN 3
#define N2k_CAN_INT_PIN 7
#define USE_MCP_CAN_CLOCK_SET 16


#include <Arduino.h>
#include <NMEA2000_CAN.h>

#include <N2kMessages.h>

// Forward declarations for led blinking
void LedOn(unsigned long OnTime);
void UpdateLedState();
void HandleNMEA2000Msg(const tN2kMsg &N2kMsg);

//*****************************************************************************
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  NMEA2000.SetN2kCANMsgBufSize(8);
  NMEA2000.SetN2kCANReceiveFrameBufSize(100);
  NMEA2000.SetForwardStream(&Serial);  // PC output on due native port
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text
  // NMEA2000.EnableForward(false);                 // Disable all msg forwarding to USB (=Serial)
  NMEA2000.SetMsgHandler(HandleNMEA2000Msg);
  NMEA2000.Open();
}

//*****************************************************************************
void loop() {
  NMEA2000.ParseMessages();
  // Dummy to empty input buffer to avoid board to stuck with e.g. NMEA Reader
  if ( Serial.available() ) { Serial.read(); } 
  UpdateLedState();
}

// Code below is just for handling led blinking.

#define LedOnTime 2
#define LedBlinkTime 1000
unsigned long TurnLedOffTime=0;
unsigned long TurnLedOnTime=millis()+LedBlinkTime;

//*****************************************************************************
void LedOn(unsigned long OnTime) {
  digitalWrite(LED_BUILTIN, HIGH);
  TurnLedOffTime=millis()+OnTime;
  TurnLedOnTime=0;
}

//*****************************************************************************
void UpdateLedState() {
  if ( TurnLedOffTime>0 && TurnLedOffTime<millis() ) {
    digitalWrite(LED_BUILTIN, LOW);
    TurnLedOffTime=0;
    TurnLedOnTime=millis()+LedBlinkTime;
  }
  
  if ( TurnLedOnTime>0 && TurnLedOnTime<millis() ) LedOn(LedBlinkTime);
}

//*****************************************************************************
//NMEA 2000 message handler
void HandleNMEA2000Msg(const tN2kMsg &/*N2kMsg*/) {
  LedOn(LedOnTime);
}





void serial_printf(Serial_& serial, const char* fmt, ...) { 
    va_list argv;
    va_start(argv, fmt);

    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            // Look for specification of number of decimal places
            int places = 2;
            if (fmt[i+1] == '.') i++;  // alw1746: Allows %.4f precision like in stdio printf (%4f will still work).
            if (fmt[i+1] >= '0' && fmt[i+1] <= '9') {
                places = fmt[i+1] - '0';
                i++;
            }
            
            switch (fmt[++i]) {
                case 'B':
                    serial.print("0b"); // Fall through intended
                case 'b':
                    serial.print(va_arg(argv, int), BIN);
                    break;
                case 'c': 
                    serial.print((char) va_arg(argv, int));
                    break;
                case 'd': 
                case 'i':
                    serial.print(va_arg(argv, int), DEC);
                    break;
                case 'f': 
                    serial.print(va_arg(argv, double), places);
                    break;
                case 'l': 
                    serial.print(va_arg(argv, long), DEC);
                    break;
                case 'o':
                    serial.print(va_arg(argv, int) == 0 ? "off" : "on");
                    break;
                case 's': 
                    serial.print(va_arg(argv, const char*));
                    break;
                case 'X':
                    serial.print("0x"); // Fall through intended
                case 'x':
                    serial.print(va_arg(argv, int), HEX);
                    break;
                case '%': 
                    serial.print(fmt[i]);
                    break;
                default:
                    serial.print("?");
                    break;
            }
        } else {
            serial.print(fmt[i]);
        }
    }
    va_end(argv);
}