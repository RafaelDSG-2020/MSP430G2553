#include "EBYTE.h"
#include <stdlib.h>

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

extern unsigned long millis() {
    for(int i = 0; i <5000; i++){
        i++;
    }
}

void delayEBYTE(unsigned int value)
{
    register unsigned int loops =  ((F_CPU * value) >> 2) ;

    while(loops)
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

void EBYTE_Init(EBYTE *ebyte, uint8_t PIN_M0, uint8_t PIN_M1, uint8_t PIN_AUX)
{
    ebyte->_M0 = PIN_M0;
    ebyte->_M1 = PIN_M1;
    ebyte->_AUX = PIN_AUX;
}

/*
Initialize the unit--basicall this reads the modules parameters and stores the parameters
for potential future module programming
*/
bool init(EBYTE *ebyte, uint8_t attempts)
{
    bool ok = true;

    pinMSPMode(P1DIR, ebyte->_AUX, INPUT);
    pinMSPMode(P2DIR, ebyte->_M0, OUTPUT);
    pinMSPMode(P2DIR, ebyte->_M1, OUTPUT);

    delayEBYTE(10);

    if (attempts < 1)
    {
        attempts = 1;
    }
    if (attempts > 10)
    {
        attempts = 10;
    }

    SetMode(ebyte, MODE_NORMAL);

    // first get the module data (must be called first for some odd reason

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

    return true;
}

/*
Method to indicate availability
*/
bool Available(const EBYTE *ebyte)
{
    return ebyte->_s->available();
}

/*
Method to indicate availability
*/
void Flush(EBYTE *ebyte)
{
    ebyte->_s->flush();
}

/*
Method to write a single byte...not sure how useful this really is. If you need to send
more that one byte, put the data into a data structure and send it in a big chunk
*/
void SendByte(EBYTE *ebyte, uint8_t TheByte)
{
    ebyte->_s->write(TheByte);
}

/*
Method to get a single byte...not sure how useful this really is. If you need to get
more that one byte, put the data into a data structure and send/receive it in a big chunk
*/
uint8_t GetByte(EBYTE *ebyte)
{
    return ebyte->_s->read();
}

/*
Method to send a chunk of data provided data is in a struct--my personal favorite as you
need not parse or worry about sprintf() inability to handle floats
TTP: put your structure definition into a .h file and include in both the sender and reciever
sketches
NOTE: of your sender and receiver MCU's are different (Teensy and Arduino) caution on the data
types each handle ints floats differently
*/
bool SendStruct(EBYTE *ebyte, const void *TheStructure, uint16_t size_)
{
    ebyte->_buf = ebyte->_s->write((uint8_t *)TheStructure, size_);

    CompleteTask(ebyte, 1000);

    return (ebyte->_buf == size_);
}

/*
Method to get a chunk of data provided data is in a struct--my personal favorite as you
need not parse or worry about sprintf() inability to handle floats
TTP: put your structure definition into a .h file and include in both the sender and reciever
sketches
NOTE: of your sender and receiver MCU's are different (Teensy and Arduino) caution on the data
types each handle ints floats differently
*/
bool GetStruct(EBYTE *ebyte, const void *TheStructure, uint16_t size_)
{
    ebyte->_buf = ebyte->_s->readBytes((uint8_t *)TheStructure, size_);

    CompleteTask(ebyte, 1000);

    return (ebyte->_buf == size_);
}
/*
Utility method to wait until module is doen tranmitting
a timeout is provided to avoid an infinite loop
*/

void CompleteTask(EBYTE *ebyte, unsigned long timeout)
{

    unsigned long t = millis();

    // make darn sure millis() is not about to reach max data type limit and start over
    if (((unsigned long) (t + timeout)) == 0){
    	t = 0;
    }

    // if AUX pin was supplied and look for HIGH state
    // note you can omit using AUX if no pins are available, but you will have to use delay() to let module finish

    // per data sheet control after aux goes high is 2ms so delay for at least that long
    // some MCU are slow so give 50 ms

     if (ebyte->_AUX != -1) {

        while(pinMSPRead(P1DIR, ebyte->_AUX) == LOW){
            delayEBYTE(PIN_RECOVER);
           if ((millis() - t) > timeout){
    			break;
    		}
        }
    }
    else {
    	// if you can't use aux pin, use 4K7 pullup with Arduino
    	// you may need to adjust this value if transmissions fail
    	delayEBYTE(1000);

    }

    // delay(PIN_RECOVER);
}
void setMode(EBYTE *ebyte, uint8_t mode)
{
    // Data sheet afirma que o módulo precisa de algum tempo extra após a configuração do modo (2ms)
    // A maioria dos meus projetos usa 10 ms, mas 40 ms é mais seguro

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
    delayEBYTE(PIN_RECOVER);

    ClearBuffer();

    // wait until aux pin goes back low
    CompleteTask(ebyte, 4000);
}

void Reset(EBYTE *ebyte)
{

    SetMode(ebyte, MODE_PROGRAM);

    ebyte->_s->write(0xC4);
    ebyte->_s->write(0xC4);
    ebyte->_s->write(0xC4);

    CompleteTask(ebyte, 4000);

    SetMode(ebyte, MODE_PROGRAM);
}

void SetSpeed(EBYTE *ebyte, uint8_t val)
{
    ebyte->_Speed = val;
}

void SetOptions(EBYTE *ebyte, uint8_t val)
{
    ebyte->_Options = val;
}

uint8_t GetSpeed(const EBYTE *ebyte)
{
    return ebyte->_Speed;
}

uint8_t GetOptions(const EBYTE *ebyte)
{
    return ebyte->_Options;
}

/*
method to set the high bit of the address
*/
void SetAddressH(EBYTE *ebyte, uint8_t val)
{
    ebyte->_AddressHigh = val;
}

uint8_t GetAddressH(const EBYTE *ebyte)
{
    return ebyte->_AddressHigh;
}

/*
method to set the lo bit of the address
*/
void SetAddressL(EBYTE *ebyte, uint8_t val)
{
    ebyte->_AddressLow = val;
}

uint8_t GetAddressL(const EBYTE *ebyte)
{
    return ebyte->_AddressLow;
}

/*
method to set the channel
*/
void SetChannel(EBYTE *ebyte, uint8_t val)
{
    ebyte->_Channel = val;
}

uint8_t GetChannel(const EBYTE *ebyte)
{
    return ebyte->_Channel;
}

/*
method to set the air data rate
*/
void SetAirDataRate(EBYTE *ebyte, uint8_t val)
{
    ebyte->_AirDataRate = val;
    // Assuming that BuildSpeedByte() is a function specific to EBYTE class
    BuildSpeedByte();
}

uint8_t GetAirDataRate(const EBYTE *ebyte)
{
    return ebyte->_AirDataRate;
}

/*
method to set the parity bit
*/
void SetParityBit(EBYTE *ebyte, uint8_t val)
{
    ebyte->_ParityBit = val;
    // Assuming that BuildSpeedByte() is a function specific to EBYTE class
    BuildSpeedByte();
}

uint8_t GetParityBit(const EBYTE *ebyte)
{
    return ebyte->_ParityBit;
}

/*
method to set the options bits
*/
void SetTransmissionMode(EBYTE *ebyte, uint8_t val)
{
    ebyte->_OptionTrans = val;
    // Assuming that BuildOptionByte() is a function specific to EBYTE class
    BuildOptionByte();
}

uint8_t GetTransmissionMode(const EBYTE *ebyte)
{
    return ebyte->_OptionTrans;
}

void SetPullupMode(EBYTE *ebyte, uint8_t val)
{
    ebyte->_OptionPullup = val;
    // Assuming that BuildOptionByte() is a function specific to EBYTE class
    BuildOptionByte();
}

uint8_t GetPullupMode(const EBYTE *ebyte)
{
    return ebyte->_OptionPullup;
}

void SetWORTIming(EBYTE *ebyte, uint8_t val)
{
    ebyte->_OptionWakeup = val;
    // Assuming that BuildOptionByte() is a function specific to EBYTE class
    BuildOptionByte();
}

uint8_t GetWORTIming(const EBYTE *ebyte)
{
    return ebyte->_OptionWakeup;
}

void SetFECMode(EBYTE *ebyte, uint8_t val)
{
    ebyte->_OptionFEC = val;
    BuildOptionByte();
}

uint8_t GetFECMode(const EBYTE *ebyte)
{
    return ebyte->_OptionFEC;
}

/*
method to set transmit power
*/
void SetTransmitPower(EBYTE *ebyte, uint8_t val)
{
    ebyte->_OptionPower = val;
    // Assuming that BuildOptionByte() is a function specific to EBYTE class
    BuildOptionByte();
}

uint8_t GetTransmitPower(const EBYTE *ebyte)
{
    return ebyte->_OptionPower;
}

/*
method to compute the address based on high and low bits
*/
void SetAddress(EBYTE *ebyte, uint16_t val)
{
    ebyte->_AddressHigh = ((val & 0xFFFF) >> 8);
    ebyte->_AddressLow = (val & 0xFF);
}

/*
method to get the address which is a collection of hi and lo bytes
*/
uint16_t GetAddress(const EBYTE *ebyte)
{
    return (uint16_t)((ebyte->_AddressHigh << 8) | (ebyte->_AddressLow));
}

/*
set the UART baud rate
*/
void SetUARTBaudRate(EBYTE *ebyte, uint8_t val)
{
    ebyte->_UARTDataRate = val;
    BuildSpeedByte();
}

uint8_t GetUARTBaudRate(const EBYTE *ebyte)
{
    return ebyte->_UARTDataRate;
}

/*
method to build the byte for programming (notice it's a collection of a few variables)
*/
void BuildSpeedByte(EBYTE *ebyte)
{
    ebyte->_Speed = 0;
    ebyte->_Speed = ((ebyte->_ParityBit & 0xFF) << 6) | ((ebyte->_UARTDataRate & 0xFF) << 3) | (ebyte->_AirDataRate & 0xFF);
}

/*
method to build the option byte for programming (notice it's a collection of a few variables)
*/
void BuildOptionByte(EBYTE *ebyte)
{
    ebyte->_Options = 0;
    ebyte->_Options = ((ebyte->_OptionTrans & 0xFF) << 7) | ((ebyte->_OptionPullup & 0xFF) << 6) | ((ebyte->_OptionWakeup & 0xFF) << 3) | ((ebyte->_OptionFEC & 0xFF) << 2) | (ebyte->_OptionPower & 0b11);
}

bool GetAux(EBYTE *ebyte)
{
    return pinMSPRead(P1OUT, ebyte->_AUX);
}

/*
method to save parameters to the module
*/

void SaveParameters(EBYTE *ebyte, uint8_t val)
{
    SetMode(ebyte, MODE_PROGRAM);

    // ClearBuffer();

    /*
    Serial.print("val: ");
    Serial.println(val);

    Serial.print("_AddressHigh: ");
    Serial.println(_AddressHigh);

    Serial.print("_AddressLow: ");
    Serial.println(_AddressLow);

    Serial.print("_Speed: ");
    Serial.println(_Speed);

    Serial.print("_Channel: ");
    Serial.println(_Channel);

    Serial.print("_Options: ");
    Serial.println(_Options);
    */

    ebyte->_s->write(val);
    ebyte->_s->write(ebyte->_AddressHigh);
    ebyte->_s->write(ebyte->_AddressLow);
    ebyte->_s->write(ebyte->_Speed);
    ebyte->_s->write(ebyte->_Channel);
    ebyte->_s->write(ebyte->_Options);

    delayEBYTE(PIN_RECOVER);

    CompleteTask(ebyte, 4000); // Função CompleteTask não está definida no código fornecido

    SetMode(ebyte, MODE_NORMAL);
}

/*
method to read parameters,
*/

bool ReadParameters(EBYTE *ebyte)
{
    ebyte->_Params[0] = 0;
    ebyte->_Params[1] = 0;
    ebyte->_Params[2] = 0;
    ebyte->_Params[3] = 0;
    ebyte->_Params[4] = 0;
    ebyte->_Params[5] = 0;

    SetMode(ebyte, MODE_PROGRAM);

    ebyte->_s->write(0xC1);
    ebyte->_s->write(0xC1);
    ebyte->_s->write(0xC1);
    ebyte->_s->readBytes((uint8_t *)&ebyte->_Params, (uint8_t)sizeof(ebyte->_Params));

    ebyte->_Save = ebyte->_Params[0];
    ebyte->_AddressHigh = ebyte->_Params[1];
    ebyte->_AddressLow = ebyte->_Params[2];
    ebyte->_Speed = ebyte->_Params[3];
    ebyte->_Channel = ebyte->_Params[4];
    ebyte->_Options = ebyte->_Params[5];

    ebyte->_Address = (uint16_t)((ebyte->_AddressHigh << 8) | (ebyte->_AddressLow));
    ebyte->_ParityBit = (ebyte->_Speed & 0xC0) >> 6;
    ebyte->_UARTDataRate = (ebyte->_Speed & 0x38) >> 3;
    ebyte->_AirDataRate = ebyte->_Speed & 0x07;

    ebyte->_OptionTrans = (ebyte->_Options & 0x80) >> 7;
    ebyte->_OptionPullup = (ebyte->_Options & 0x40) >> 6;
    ebyte->_OptionWakeup = (ebyte->_Options & 0x38) >> 3;
    ebyte->_OptionFEC = (ebyte->_Options & 0x07) >> 2;
    ebyte->_OptionPower = ebyte->_Options & 0x03;

    SetMode(ebyte, MODE_NORMAL);

    if (0xC0 != ebyte->_Params[0])
    {
        return false;
    }

    return true;
}

bool ReadModelData(EBYTE *ebyte)
{
    ebyte->_Params[0] = 0;
    ebyte->_Params[1] = 0;
    ebyte->_Params[2] = 0;
    ebyte->_Params[3] = 0;
    ebyte->_Params[4] = 0;
    ebyte->_Params[5] = 0;

    bool found = false;
    int i = 0;

    SetMode(ebyte, MODE_PROGRAM);

    ebyte->_s->write(0xC3);
    ebyte->_s->write(0xC3);
    ebyte->_s->write(0xC3);
    ebyte->_s->readBytes((uint8_t *)&ebyte->_Params, (uint8_t)sizeof(ebyte->_Params));

    ebyte->_Save = ebyte->_Params[0];
    ebyte->_Model = ebyte->_Params[1];
    ebyte->_Version = ebyte->_Params[2];
    ebyte->_Features = ebyte->_Params[3];

    if (0xC3 != ebyte->_Params[0])
    {
        for (i = 0; i < 5; i++)
        {
            ebyte->_Params[0] = 0;
            ebyte->_Params[1] = 0;
            ebyte->_Params[2] = 0;
            ebyte->_Params[3] = 0;
            ebyte->_Params[4] = 0;
            ebyte->_Params[5] = 0;

            ebyte->_s->write(0xC3);
            ebyte->_s->write(0xC3);
            ebyte->_s->write(0xC3);

            ebyte->_s->readBytes((uint8_t *)&ebyte->_Params, (uint8_t)sizeof(ebyte->_Params));
            /*
            Serial.print("_Attempts ");Serial.println(_Attempts);
            Serial.print("_Params[0] ");Serial.println(_Params[0]);
            Serial.print("_Params[1] ");Serial.println(_Params[1]);
            Serial.print("_Params[2] ");Serial.println(_Params[2]);
            Serial.print("_Params[3] ");Serial.println(_Params[3]);
            Serial.print("_Params[4] ");Serial.println(_Params[4]);
            Serial.print("_Params[5] ");Serial.println(_Params[5]);
            */
            if (0xC3 == ebyte->_Params[0])
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

    SetMode(ebyte, MODE_NORMAL);

    return found;
}

/*
method to get module model and E50-TTL-100 will return 50
*/
uint8_t GetModel(const EBYTE *ebyte)
{
    return ebyte->_Model;
}

/*
method to get module version (undocumented as to the value)
*/
uint8_t GetVersion(const EBYTE *ebyte)
{
    return ebyte->_Version;
}

/*
method to get module features (undocumented as to the value)
*/
uint8_t GetFeatures(const EBYTE *ebyte)
{
    return ebyte->_Features;
}

/*
method to clear the serial buffer

without clearing the buffer, i find getting the parameters very unreliable after programming.
i suspect stuff in the buffer affects rogramming
hence, let's clean it out
this is called as part of the setmode

*/
void ClearBuffer(EBYTE* ebyte) {
    unsigned long amt = millis();

    while (Available(ebyte)) {
        Flush(ebyte);
        if ((millis() - amt) > 5000) {
            // Substitua o Serial.println por uma função adequada para o seu ambiente
            // já que Serial.println pode não estar disponível em todos os ambientes C.
            //printf("runaway\n");
            break;
        }
    }
}