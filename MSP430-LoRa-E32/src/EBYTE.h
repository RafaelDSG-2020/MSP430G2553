/*
 The MIT License (MIT)
 Copyright (c) 2019 Kris Kasrpzak
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 On a personal note, if you develop an application or product using this library
 and make millions of dollars, I'm happy for you!
*/

/*
  Code by Kris Kasprzak kris.kasprzak@yahoo.com
  This library is intended to be used with EBYTE transcievers, small wireless units for MCU's such as
  Teensy and Arduino. This library let's users program the operating parameters and both send and recieve data.
  This company makes several modules with different capabilities, but most #defines here should be compatible with them
  All constants were extracted from several data sheets and listed in binary as that's how the data sheet represented each setting
  Hopefully, any changes or additions to constants can be a matter of copying the data sheet constants directly into these #defines
  Usage of this library consumes around 970 bytes
  Revision		Data		Author			Description
  1.0			3/6/2019	Kasprzak		Initial creation
  2.0			3/2/2020	Kasprzak		Added all functions to build the options bit (FEC, Pullup, and TransmissionMode
  3.0			3/27/2020	Kasprzak		Added more Get functions
  4.0			6/23/2020	Kasprzak		Added private method to clear the buffer to ensure read methods would not be filled with buffered data
  5.0			12/4/2020	Kasprzak		moved Reset to public, added Clear to SetMode to avoid buffer corruption during programming
  5.5			1/26/2022	Kasprzak		added attempt parameter to limit the retries

  Module connection
  Module	MCU						Description
  MO		Any digital pin*		pin to control working/program modes
  M1		Any digital pin*		pin to control working/program modes
  Rx		Any digital pin			pin to MCU TX pin (module transmits to MCU, hence MCU must recieve data from module
  Tx		Any digital pin			pin to MCU RX pin (module transmits to MCU, hence MCU must recieve data from module
  AUX		Any digital pin			pin to indicate when an operation is complete (low is busy, high is done)
  Vcc		+3v3 or 5V0
  Vcc		Ground					Ground must be common to module and MCU
  notes:
  * caution in connecting to Arduino pin 0 and 1 as those pins are for USB connection to PC
  you may need a 4K7 pullup to Rx and AUX pins (possibly Tx) if using and Arduino
  Module source
  http://www.ebyte.com/en/
  example module this library is intended to be used with
  http://www.ebyte.com/en/product-view-news.aspx?id=174
  Code usage
  1. Create a serial object
  2. Create EBYTE object that uses the serail object
  3. begin the serial object
  4. init the EBYTE object
  5. set parameters (optional but required if sender and reciever are different)
  6. send or listen to sent data

*/

#ifndef EBYTE_H
#define EBYTE_H

// #define EBYTE_H_LIB_VER 5.5

// #if ARDUINO >= 100
// #include "Arduino.h"
// #else
// #include "WProgram.h"
// #endif

#include <msp430.h>

#include <stdint.h>

#include <stdio.h>

// if you seem to get "corrupt settings add this line to your .ino
// #include <avr/io.h>

/*

if modules don't seem to save or read parameters, it's probably due to slow pin changing times
in the module. I see this happen rarely. You will have to adjust this value
when settin M0 an M1 there is gererally a short time for the transceiver modules
to react. The data sheet says 2 ms, but more time is generally needed. I'm using
50 ms below and maybe too long, but it seems to work in most cases. Increase this value
if your unit will not return parameter settings.
*/

#define PIN_RECOVER 50

#define USBUART_BUFFER_SIZE (64u)
#define USB_TIMEOUT (50u)

// modes NORMAL send and recieve for example
#define MODE_NORMAL 0 // can send and recieve
#define MODE_WAKEUP 1 // sends a preamble to waken receiver
#define MODE_POWERDOWN 2 // can't transmit but receive works only in wake up mode
#define MODE_PROGRAM 3 // for programming

void writeUsb64(uint8_t * buffer, uint16_t size);
void sendUsb(char * buffer, unsigned int size);

// options to save change permanently or temp (power down and restart will restore settings to last saved options
#define PERMANENT 0xC0
#define TEMPORARY 0xC2

bool initRadio();

// methods to set modules working parameters NOTHING WILL BE SAVED UNLESS SaveParameters() is called
void SetMode(uint8_t mode);
void SetAddress(uint16_t val);
void SetAddressH(uint8_t val);
void SetAddressL(uint8_t val);
void SetSpeed(uint8_t val);
void SetChannel(uint8_t val);

bool GetAux();

bool available();
// methods to get some operating parameters
uint16_t GetAddress();

// methods to get module data
uint8_t GetModel();
uint8_t GetVersion();
uint8_t GetFeatures();

uint8_t GetAddressH();
uint8_t GetAddressL();
uint8_t GetAirDataRate();
uint8_t GetUARTBaudRate();
uint8_t GetChannel();
uint8_t GetParityBit();
uint8_t GetTransmissionMode();
uint8_t GetPullupMode();
uint8_t GetWORTIming();
uint8_t GetFECMode();
uint8_t GetTransmitPower();

uint8_t GetOptions();
uint8_t GetSpeed();

// methods to get data from sending unit
uint8_t EBYTE_GetByte();

// method to send to data to receiving unit
void EBYTE_SendByte(uint8_t TheByte);

// mehod to print parameters
void PrintParameters();

// parameters are set above but NOT saved, here's how you save parameters
// notion here is you can set several but save once as opposed to saving on each parameter change
// you can save permanently (retained at start up, or temp which is ideal for dynamically changing the address or frequency
void SaveParameters(uint8_t val);

// MFG is not clear on what Reset does, but my testing indicates it clears buffer
// I use this when needing to restart the EBYTE after programming while data is still streaming in
// it does NOT return the ebyte back to factory defaults

// function to read modules parameters
bool ReadParameters();

// method to let method know of module is busy doing something (timeout provided to avoid lockups)
void CompleteTask(unsigned long timeout);

// utility funciton to build the "speed byte" which is a collection of a few different parameters
void BuildSpeedByte();

// utility funciton to build the "options byte" which is a collection of a few different parameters

void BuildOptionByte();
bool ReadModelData();
void ClearBuffer();

typedef struct ebyte_t {
  // pin variables
	int8_t _M0;
	int8_t _M1;
	int8_t _AUX;
  // variable for the 6 bytes that are sent to the module to program it
  // or bytes received to indicate modules programmed settings
  uint8_t _Params[6];

  // indicidual variables for each of the 6 bytes
  // _Params could be used as the main variable storage, but since some bytes
  // are a collection of several options, let's just make storage consistent
  // also Param[1] is different data depending on the _Save variable
  uint8_t _Save;
  uint8_t _AddressHigh;
  uint8_t _AddressLow;
  uint8_t _Speed;
  uint8_t _Channel;
  uint8_t _Options;
  uint8_t _Attempts;

  // individual variables for all the options
  uint8_t _ParityBit;
  uint8_t _UARTDataRate;
  uint8_t _AirDataRate;
  uint8_t _OptionTrans;
  uint8_t _OptionPullup;
  uint8_t _OptionWakeup;
  uint8_t _OptionFEC;
  uint8_t _OptionPower;
  uint16_t _Address;
  uint8_t _Model;
  uint8_t _Version;
  uint8_t _Features;
  uint8_t _buf;

}
EBYTE;

// extern unsigned long millis();

typedef struct data_t {
  // data sent and received
  uint8_t Count;
  uint8_t message1;
  uint8_t message2;
  uint8_t message3[6];
}
DATA;

extern EBYTE Radio;
#endif
/* [] END OF FILE */