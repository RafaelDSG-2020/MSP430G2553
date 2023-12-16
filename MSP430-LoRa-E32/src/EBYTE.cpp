#include "EBYTE.h"
#include "UART_MSP.h"

#include <stdlib.h>

#include <stdio.h>

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

// extern unsigned long millis()
// {
//   for (int i = 0; i < 5000; i++)
//   {
//     i++;
//   }
// }

void delayEBYTE(unsigned int value)
{
  register unsigned int loops = ((F_CPU * value) >> 2);

  while (loops)
  {
    __delay_cycles(1);
    loops--;
  };
}

void pinMSPMode(volatile unsigned char &portDir, uint8_t pin, uint8_t mode)
{
  if (mode == INPUT)
  {
    portDir &= ~(1 << pin); // Configura o pino como entrada
  }
  else if (mode == OUTPUT)
  {
    portDir |= (1 << pin); // Configura o pino como saída
  }
}

void pinMSPWrite(volatile unsigned char &portOut, uint8_t pin, uint8_t level)
{
  if (level == LOW)
  {
    portOut &= ~(1 << pin); // Nível baixo
  }
  else if (level == HIGH)
  {
    portOut |= (1 << pin); // Nível alto
  }
}

int pinMSPRead(volatile unsigned char &port, uint8_t pin)
{
  return (port & (1 << pin)) ? HIGH : LOW;
}

unsigned long millis = 0;
unsigned long usbTimeoutTimer = 0u;



// void writeUsb64(uint8_t * buffer, uint16_t size) {
//   /*Host can send double SET_INTERFACE request. */
//   if (0u != USBUART_IsConfigurationChanged()) {
//     /*Initialize IN endpoints when device is configured. */
//     if (0u != USBUART_GetConfiguration()) {
//       /*Enumeration is done, enable OUT endpoint to receive data
//        *from host. */
//       USBUART_CDC_Init();
//     }
//   } else {
//     /*Service USB CDC when device is configured. */
//     if (0u != USBUART_GetConfiguration()) {
//       if (0u != size) {
//         usbTimeoutTimer = millis + USB_TIMEOUT;

//         /*Wait until component is ready to send data to host. */
//         while (0u == USBUART_CDCIsReady()) {
//           if (usbTimeoutTimer < millis) {
//             return;
//           }
//         }

//         /*Send data back to host. */
//         USBUART_PutData(buffer, size);
//         usbTimeoutTimer = millis + USB_TIMEOUT;

//         while (0u == USBUART_CDCIsReady()) {
//           if (usbTimeoutTimer < millis) {
//             return;
//           }
//         }

//         /*If the last sent packet is exactly the maximum packet
//          *size, it is followed by a zero-length packet to assure
//          *that the end of the segment is properly identified by
//          *the terminal.
//          */
//         if (USBUART_BUFFER_SIZE == size) {
//           usbTimeoutTimer = millis + USB_TIMEOUT;
//           /*Wait until component is ready to send data to PC. */
//           while (0u == USBUART_CDCIsReady()) {
//             if (usbTimeoutTimer < millis) {
//               return;
//             }
//           }

//           /*Send zero-length packet to PC. */
//           USBUART_PutData(NULL, 0u);
//         }
//       }
//     }
//   }
// }

// void sendUsb(char *buffer, unsigned int size)
// {
//   if (USBUART_bGetConfiguration() != 0)
//   {
//     if (size > 64u)
//     {
//       unsigned int bytesSent = 0, bytesToSend = 0;
//       while (bytesSent < size)
//       {
//         bytesToSend = size - bytesSent;
//         if (bytesToSend > 64u)
//         {
//           bytesToSend = 64u;
//         }

//         writeUsb64((void *)buffer + bytesSent, bytesToSend);
//         bytesSent = bytesSent + bytesToSend;
//       }
//     }
//     else
//     {
//       writeUsb64((void *)buffer, (uint16_t)size);
//     }
//   }
// }

EBYTE Radio;

bool initRadio()
{

  UART_1_Start();
  //UART_1_ENABLE_Write(1);
  //USBUART_Start(0, USBUART_3V_OPERATION);

  bool ok = true;

  delayEBYTE(10);

  SetMode(MODE_NORMAL);

  // first get the module data

  ok = ReadModelData();

  if (!ok)
  {
    return false;
  }
  // now get parameters to put unit defaults into the class variables

  ok = ReadParameters();
  if (!ok)
  {
    return false;
  }

  char usbPackege[256];

  // sprintf(usbPackege, "AirDataRate: %d\n", GetAirDataRate());
  // sendUsb((void *)usbPackege, strlen(usbPackege));
  // sprintf(usbPackege, "Channel: %d\n", GetChannel());
  // sendUsb((void *)usbPackege, strlen(usbPackege));

  SetChannel(5);
  SetSpeed(0x1A);
  SaveParameters(PERMANENT);
  PrintParameters();

  return true;
}

void SetMode(uint8_t mode)
{

  // data sheet claims module needs some extra time after mode setting (2ms)
  // most of my projects uses 10 ms, but 40ms is safer

  delayEBYTE(PIN_RECOVER);

  if (mode == MODE_NORMAL)
  {
    // Exemplo de uso com os pinos M0 e M1 representando P2.1 e P2.2
    if (mode == MODE_NORMAL)
    {
      pinMSPWrite(P2OUT, 1, LOW);
      pinMSPWrite(P2OUT, 2, LOW);
    }
    else if (mode == MODE_WAKEUP)
    {
      pinMSPWrite(P2OUT, 1, HIGH);
      pinMSPWrite(P2OUT, 2, LOW);
    }
    else if (mode == MODE_POWERDOWN)
    {
      pinMSPWrite(P2OUT, 1, LOW);
      pinMSPWrite(P2OUT, 2, HIGH);
    }
    else if (mode == MODE_PROGRAM)
    {
      pinMSPWrite(P2OUT, 1, HIGH);
      pinMSPWrite(P2OUT, 2, HIGH);
    }
  }

  // data sheet says 2ms later control is returned, let's give just a bit more time
  // these modules can take time to activate pins
  delayEBYTE(PIN_RECOVER);

  // clear out any junk
  // added rev 5
  // i've had some issues where after programming, the returned model is 0, and all settings appear to be corrupt
  // i imagine the issue is due to the internal buffer full of junk, hence clearing
  ClearBuffer();

  // wait until aux pin goes back low
  CompleteTask(4000);
}

void SetAddress(uint16_t Val)
{
  Radio._AddressHigh = ((Val & 0xFFFF) >> 8);
  Radio._AddressLow = (Val & 0xFF);
}

void SetAddressH(uint8_t val)
{
  Radio._AddressHigh = val;
}

void SetAddressL(uint8_t val)
{
  Radio._AddressLow = val;
}

void SetSpeed(uint8_t val)
{
  Radio._Speed = val;
}

void SetChannel(uint8_t val)
{
  Radio._Channel = val;
}

bool GetAux()
{
  ebyte_t *ebyte;
  return pinMSPRead(P1OUT, ebyte->_AUX);
}

bool available()
{
  return UART_1_GetRxBufferSize();
  
}

uint16_t GetAddress()
{
  return (Radio._AddressHigh << 8) | (Radio._AddressLow);
}

uint8_t GetModel()
{

  return Radio._Model;
}

uint8_t GetVersion()
{

  return Radio._Version;
}

uint8_t GetFeatures()
{

  return Radio._Features;
}

uint8_t GetAddressH()
{
  return Radio._AddressHigh;
}

uint8_t GetAddressL()
{
  return Radio._AddressLow;
}

uint8_t GetAirDataRate()
{
  return Radio._AirDataRate;
}

uint8_t GetUARTBaudRate()
{
  return Radio._UARTDataRate;
}

uint8_t GetChannel()
{
  return Radio._Channel;
}

uint8_t GetParityBit()
{
  return Radio._ParityBit;
}

uint8_t GetTransmissionMode()
{
  return Radio._OptionTrans;
}

uint8_t GetPullupMode()
{
  return Radio._OptionPullup;
}

uint8_t GetWORTIming()
{
  return Radio._OptionWakeup;
}

uint8_t GetFECMode()
{
  return Radio._OptionFEC;
}

uint8_t GetTransmitPower()
{
  return Radio._OptionPower;
}

uint8_t GetSpeed()
{
  return Radio._Speed;
}

uint8_t GetOptions()
{
  return Radio._Options;
}

uint8_t EBYTE_GetByte()
{

  return UART_1_GetByte();
}

void EBYTE_SendByte(uint8_t TheByte)
{

  UART_1_PutChar(TheByte);
}

void PrintParameters()
{

  char package[256];

  // sprintf(package, "----------------------------------------");
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "Model no.: %x\n", Radio._Model);
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "Version  : %x\n", Radio._Version);
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "Features : %x\n", Radio._Features);
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "\"\n\"");
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "Mode (HEX): %x\n", Radio._Save);
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "Sped (HEX): %x\n", Radio._Speed);
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "Chan (HEX): %x\n", Radio._Channel);
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "Optn (HEX): %x\n", Radio._Options);
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "Addr (HEX): %x\n", Radio._Address);
  // sendUsb((void * ) package, strlen(package));
  // sprintf(package, "----------------------------------------");
  // sendUsb((void * ) package, strlen(package));
}

void SaveParameters(uint8_t val)
{

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

bool ReadParameters()
{

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

  for (count = 0; count < 6; count++)
  {
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

  if (0xC0 != Radio._Params[0])
  {

    return false;
  }

  return true;
}

void CompleteTask(unsigned long timeout)
{

  unsigned long t = 0u;
  ebyte_t *ebyte;

  if (pinMSPRead(P1DIR, ebyte->_AUX) == 0)
  {
    while (pinMSPRead(P1DIR, ebyte->_AUX) == 0)
    {
      delayEBYTE(1);
      t++;
      delayEBYTE(PIN_RECOVER);
      if (t > timeout)
      {
        break;
      }
    }
  }
  else
  {
    delayEBYTE(1000);
  }

  // delay(PIN_RECOVER);
}

void BuildSpeedByte()
{
  Radio._Speed = 0;
  Radio._Speed = ((Radio._ParityBit & 0xFF) << 6) | ((Radio._UARTDataRate & 0xFF) << 3) | (Radio._AirDataRate & 0xFF);
}

void BuildOptionByte()
{
  Radio._Options = 0;
  Radio._Options = ((Radio._OptionTrans & 0xFF) << 7) | ((Radio._OptionPullup & 0xFF) << 6) | ((Radio._OptionWakeup & 0xFF) << 3) | ((Radio._OptionFEC & 0xFF) << 2) | (Radio._OptionPower & 0b11);
}

bool ReadModelData()
{

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

  for (count = 0; count < 6; count++)
  {
    Radio._Params[count] = UART_1_GetByte();
  }

  Radio._Save = Radio._Params[0];
  Radio._Model = Radio._Params[1];
  Radio._Version = Radio._Params[2];
  Radio._Features = Radio._Params[3];

  if (0xC3 != Radio._Params[0])
  {

    // i'm not terribly sure this is the best way to retry
    // may need to set the mode back to normal first....
    for (i = 0; i < 5; i++)
    {
      Radio._Params[0] = 0;
      Radio._Params[1] = 0;
      Radio._Params[2] = 0;
      Radio._Params[3] = 0;
      Radio._Params[4] = 0;
      Radio._Params[5] = 0;

      UART_1_PutChar(0xC3);
      UART_1_PutChar(0xC3);
      UART_1_PutChar(0xC3);

      for (count = 0; count < 6; count++)
      {
        Radio._Params[count] = UART_1_GetByte();
      }

      if (0xC3 == Radio._Params[0])
      {
        found = true;
        break;
      }

      delayEBYTE(100);
    }
  }
  else
  {
    found = true;
  }

  SetMode(MODE_NORMAL);

  return found;
}

void ClearBuffer()
{
  UART_1_ClearRxBuffer();
}