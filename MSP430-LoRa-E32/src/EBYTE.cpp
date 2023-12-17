#include "EBYTE.h"
#include "UART.h"

#define INPUT_PIN 0
#define OUTPUT_PIN 1

#define LOW_PIN 0
#define HIGH_PIN 1

#define PIN_M0 BIT1
#define PIN_M1 BIT2
#define AUX_PIN BIT4  // Assumindo que o pino AUX está conectado ao P1.4

void delayEBYTE(const unsigned long int value) {
  for(int i = 0; i  < value; i++) {
    i++;
    __delay_cycles(1000);
  }
}

void pinMSPMode(volatile unsigned char &portDir, uint8_t pin, uint8_t mode) {
  if (mode == INPUT_PIN) {
    portDir &= ~(1 << pin);  // Configura o pino como entrada
  } else if (mode == OUTPUT_PIN) {
    portDir |= (1 << pin);  // Configura o pino como saída
  }
}

void pinMSPWrite(volatile unsigned char &portOut, uint8_t pin, uint8_t level) {
  if (level == LOW_PIN) {
    portOut &= ~(1 << pin);  // Nível baixo
  } else if (level == HIGH_PIN) {
    portOut |= (1 << pin);  // Nível alto
  }
}

int pinMSPRead(volatile unsigned char &port, uint8_t pin) {
  return (port & (1 << pin)) ? HIGH_PIN : LOW_PIN;
}

EBYTE Radio;
ebyte_t *ebyte;

bool initRadio() {
  bool ok = true;

  // ebyte_t *ebyte;
  ebyte->_M0 = PIN_M0;
  ebyte->_M1 = PIN_M1;
  ebyte->_AUX = AUX_PIN;
  
  UART_1_Start();

  pinMSPMode(P1DIR, ebyte->_AUX, INPUT_PIN);
  pinMSPMode(P2DIR, ebyte->_M0, OUTPUT_PIN);
  pinMSPMode(P2DIR, ebyte->_M1, OUTPUT_PIN);

  delayEBYTE(10);

  SetMode(MODE_NORMAL);

  // first get the module data

  ok = ReadModelData();

  if (!ok) {
    return false;
  }
  // now get parameters to put unit defaults into the class variables

  ok = ReadParameters();
  if (!ok) {
    return false;
  }
  return true;
}

bool available() {
  return UART_1_GetRxBufferSize();
}

// flush

void EBYTE_SendByte(uint8_t TheByte) {
  UART_1_PutChar(TheByte);
}

uint8_t EBYTE_GetByte() {
  return UART_1_GetByte();
}

//SendStruct

//GetStruct

void CompleteTask(unsigned long timeout) {
  unsigned long t = 0u;
  // ebyte_t *ebyte;
  pinMSPRead(P1OUT, ebyte->_AUX);

  if (pinMSPRead(P1OUT, ebyte->_AUX) == 0) {
    while (pinMSPRead(P1OUT, ebyte->_AUX) == 0) {
      delayEBYTE(1);
      t++;
      delayEBYTE(PIN_RECOVER);
      if (t > timeout) {
        break;
      }
    }
  } else {
    delayEBYTE(1000);
  }
  // delay(PIN_RECOVER);
}

void SetMode(uint8_t mode) {
  // data sheet claims module needs some extra time after mode setting (2ms)
  // most of my projects uses 10 ms, but 40ms is safer

  delayEBYTE(PIN_RECOVER);

  if (mode == MODE_NORMAL) {
    // Exemplo de uso com os pinos M0 e M1 representando P2.1 e P2.2
    if (mode == MODE_NORMAL) {
      pinMSPWrite(P2OUT, PIN_M0, LOW_PIN);
      pinMSPWrite(P2OUT, PIN_M1, LOW_PIN);

    } else if (mode == MODE_WAKEUP) {
      pinMSPWrite(P2OUT, PIN_M0, HIGH_PIN);
      pinMSPWrite(P2OUT, PIN_M1, LOW_PIN);

    } else if (mode == MODE_POWERDOWN) {
      pinMSPWrite(P2OUT, PIN_M0, LOW_PIN);
      pinMSPWrite(P2OUT, PIN_M1, HIGH_PIN);

    } else if (mode == MODE_PROGRAM) {
      pinMSPWrite(P2OUT, PIN_M0, HIGH_PIN);
      pinMSPWrite(P2OUT, PIN_M0, HIGH_PIN);
    }
  }

  // data sheet says 2ms later control is returned, let's give just a bit more
  // time these modules can take time to activate pins
  delayEBYTE(PIN_RECOVER);

  // clear out any junk
  // added rev 5
  // i've had some issues where after programming, the returned model is 0, and
  // all settings appear to be corrupt i imagine the issue is due to the
  // internal buffer full of junk, hence clearing
  ClearBuffer();

  // wait until aux pin goes back low
  CompleteTask(4000);
}

//reset ()

void SetSpeed(uint8_t val) { Radio._Speed = val; }

//SetOptions

uint8_t GetSpeed() { return Radio._Speed; }

uint8_t GetOptions() { return Radio._Options; }

/*
method to set the high bit of the address
*/
void SetAddressH(uint8_t val) { Radio._AddressHigh = val; }

uint8_t GetAddressH() { return Radio._AddressHigh; }

/*
method to set the lo bit of the address
*/
void SetAddressL(uint8_t val) { Radio._AddressLow = val; }

uint8_t GetAddressL() { return Radio._AddressLow; }

/*
method to set the channel
*/
void SetChannel(uint8_t val) { Radio._Channel = val; }

uint8_t GetChannel() { return Radio._Channel; }

/*
method to set the air data rate
*/

//SetAirDataRate

uint8_t GetAirDataRate() { return Radio._AirDataRate; }

/*
method to set the parity bit
*/

//SetParityBit

uint8_t GetParityBit() { return Radio._ParityBit; }

/*
method to set the options bits
*/

//setTransmissionMode

uint8_t GetTransmissionMode() { return Radio._OptionTrans; }

//setPullupMode

uint8_t GetPullupMode() { return Radio._OptionPullup; }

//setWORTIming

uint8_t GetWORTIming() { return Radio._OptionWakeup; }

//setFECMode

uint8_t GetFECMode() { return Radio._OptionFEC; }

//setTransmitPower

uint8_t GetTransmitPower() { return Radio._OptionPower; }

/*
method to compute the address based on high and low bits
*/
void SetAddress(uint16_t Val) {
  Radio._AddressHigh = ((Val & 0xFFFF) >> 8);
  Radio._AddressLow = (Val & 0xFF);
}

/*
method to get the address which is a collection of hi and lo bytes
*/
uint16_t GetAddress() {
  return (Radio._AddressHigh << 8) | (Radio._AddressLow);
}

/*
set the UART baud rate
*/
//setUARTBaudRate

uint8_t GetUARTBaudRate() { return Radio._UARTDataRate; }

/*
method to build the byte for programming (notice it's a collection of a few
variables)
*/
void BuildSpeedByte() {
  Radio._Speed = 0;
  Radio._Speed = ((Radio._ParityBit & 0xFF) << 6) |
                 ((Radio._UARTDataRate & 0xFF) << 3) |
                 (Radio._AirDataRate & 0xFF);
}

/*
method to build the option byte for programming (notice it's a collection of a
few variables)
*/
void BuildOptionByte() {
  Radio._Options = 0;
  Radio._Options =
      ((Radio._OptionTrans & 0xFF) << 7) | ((Radio._OptionPullup & 0xFF) << 6) |
      ((Radio._OptionWakeup & 0xFF) << 3) | ((Radio._OptionFEC & 0xFF) << 2) |
      (Radio._OptionPower & 0b11);
}

bool GetAux() {
  // ebyte_t *ebyte;
  return pinMSPRead(P1OUT, ebyte->_AUX);
}

/*
method to save parameters to the module
*/
void SaveParameters(uint8_t val) {
  SetMode(MODE_PROGRAM);

  UART_1_PutChar(val);
  UART_1_PutChar(Radio._AddressHigh);
  UART_1_PutChar(Radio._AddressLow);
  UART_1_PutChar(Radio._Speed);
  UART_1_PutChar(Radio._Channel);
  UART_1_PutChar(Radio._Options);

  delayEBYTE(PIN_RECOVER);

  CompleteTask(4000);

  SetMode(MODE_NORMAL);
}

/*
method to print parameters, this can be called anytime after init(), because
init gets parameters and any method updates the variables
*/

void PrintParameters() {
  // char package[256];

  // Radio._ParityBit = (Radio._Speed & 0XC0) >> 6;
  // Radio._UARTDataRate = (Radio._Speed & 0X38) >> 3;
  // Radio._AirDataRate = Radio._Speed & 0X07;

  // Radio._OptionTrans = (Radio._Options & 0X80) >> 7;
  // Radio._OptionPullup = (Radio._Options & 0X40) >> 6;
  // Radio._OptionWakeup = (Radio._Options & 0X38) >> 3;
  // Radio._OptionFEC = (Radio._Options & 0X07) >> 2;
  // Radio._OptionPower = (Radio._Options & 0X03);

  // sprintf(package, "----------------------------------------");
  // sendUsb((void *)package, strlen(package));
  // sprintf(package,"Model no.: %x\n", Radio._Model);
  // sendUsb((void *)package, strlen(package));
  // sprintf(package,"Version  : %x\n", Radio._Version);
  // sendUsb((void *)package, strlen(package));
  // sprintf(package, "Features : %x\n",Radio._Features);
  // sendUsb((void *)package, strlen(package));
  // sprintf(package, "\"\n\"");
  // sendUsb((void *)package, strlen(package));
  // sprintf(package,"Mode (HEX): %x\n",Radio._Save);
  // sendUsb((void *)package, strlen(package));
  // sprintf(package, "Sped (HEX): %x\n", Radio._Speed);
  // sendUsb((void *)package, strlen(package));
  // sprintf(package, "Chan (HEX): %x\n", Radio._Channel);
  // sendUsb((void *)package, strlen(package));
  // sprintf(package, "Optn (HEX): %x\n", Radio._Options);
  // sendUsb((void *)package, strlen(package));
  // sprintf(package,"Addr (HEX): %x\n", Radio._Address);
  // sendUsb((void *)package, strlen(package));
  // sprintf(package,"----------------------------------------");
  // sendUsb((void *)package, strlen(package));
}

/*
method to read parameters,
*/

bool ReadParameters() {
  uint8_t count;

  Radio._Params[0] = 0;
  Radio._Params[1] = 0;
  Radio._Params[2] = 0;
  Radio._Params[3] = 0;
  Radio._Params[4] = 0;
  Radio._Params[5] = 0;

  SetMode(MODE_PROGRAM);

  UART_1_PutChar(0xC1);

  UART_1_PutChar(0xC1);

  UART_1_PutChar(0xC1);

  for (count = 0; count < 6; count++) {
    Radio._Params[count] = UART_1_GetByte();
  }

  Radio._Save = Radio._Params[0];
  Radio._AddressHigh = Radio._Params[1];
  Radio._AddressLow = Radio._Params[2];
  Radio._Speed = Radio._Params[3];
  Radio._Channel = Radio._Params[4];
  Radio._Options = Radio._Params[5];

  Radio._Address = (Radio._AddressHigh << 8) | (Radio._AddressLow);
  Radio._ParityBit = (Radio._Speed & 0XC0) >> 6;
  Radio._UARTDataRate = (Radio._Speed & 0X38) >> 3;
  Radio._AirDataRate = Radio._Speed & 0X07;

  Radio._OptionTrans = (Radio._Options & 0X80) >> 7;
  Radio._OptionPullup = (Radio._Options & 0X40) >> 6;
  Radio._OptionWakeup = (Radio._Options & 0X38) >> 3;
  Radio._OptionFEC = (Radio._Options & 0X07) >> 2;
  Radio._OptionPower = (Radio._Options & 0X03);

  SetMode(MODE_NORMAL);

  if (0xC0 != Radio._Params[0]) {
    return false;
  }
  return true;
}

bool ReadModelData() {
  Radio._Params[0] = 0;
  Radio._Params[1] = 0;
  Radio._Params[2] = 0;
  Radio._Params[3] = 0;
  Radio._Params[4] = 0;
  Radio._Params[5] = 0;

  bool found = false;
  int count, i = 0;

  SetMode(MODE_PROGRAM);

  UART_1_PutChar(0xC3);
  UART_1_PutChar(0xC3);
  UART_1_PutChar(0xC3);

  for (count = 0; count < 6; count++) {
    Radio._Params[count] = UART_1_GetByte();
  }

  Radio._Save = Radio._Params[0];
  Radio._Model = Radio._Params[1];
  Radio._Version = Radio._Params[2];
  Radio._Features = Radio._Params[3];

  if (0xC3 != Radio._Params[0]) {
    // i'm not terribly sure this is the best way to retry
    // may need to set the mode back to normal first....
    for (i = 0; i < 5; i++) {
      Radio._Params[0] = 0;
      Radio._Params[1] = 0;
      Radio._Params[2] = 0;
      Radio._Params[3] = 0;
      Radio._Params[4] = 0;
      Radio._Params[5] = 0;

      UART_1_PutChar(0xC3);
      UART_1_PutChar(0xC3);
      UART_1_PutChar(0xC3);

      for (count = 0; count < 6; count++) {
        Radio._Params[count] = UART_1_GetByte();
      }

      if (0xC3 == Radio._Params[0]) {
        found = true;
        break;
      }

      delayEBYTE(100);
    }
  } else {
    found = true;
  }

  SetMode(MODE_NORMAL);

  return found;
}
/*
method to get module model and E50-TTL-100 will return 50
*/

uint8_t GetModel() {
	return Radio._Model;
}

/*
method to get module version (undocumented as to the value)
*/

uint8_t GetVersion() {
	return Radio._Version;
}

/*
method to get module version (undocumented as to the value)
*/
uint8_t GetFeatures() {
	return Radio._Features;
}

/*
method to clear the serial buffer

without clearing the buffer, i find getting the parameters very unreliable after programming.
i suspect stuff in the buffer affects rogramming 
hence, let's clean it out
this is called as part of the setmode

*/
void ClearBuffer(){
    UART_1_ClearRxBuffer();
}
