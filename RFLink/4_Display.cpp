// ************************************* //
// * Arduino Project RFLink32        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "3_Serial.h"
#include "4_Display.h"

byte PKSequenceNumber = 0;       // 1 byte packet counter
char dbuffer[60];                // Buffer for message chunk data
char tempJsonbuffer[60];         // Buffer for message chunk data
char pbuffer[PRINT_BUFFER_SIZE]; // Buffer for complete message data
char topicName[PRINT_BUFFER_SIZE]; // Topic name for MQTT
char jsonBuffer[JSON_BUFFER_SIZE]; // Buffer for MQTT
StaticJsonDocument<JSON_BUFFER_SIZE> jsonDoc; // JSON document for MQTT

// ------------------- //
// Display shared func //
// ------------------- //

#if (defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__))
#error "For AVR plaforms, in all sprintf_P above, please replace %s with %S"
#endif

// Common Header
void display_Header(void)
{
  sprintf_P(dbuffer, PSTR("%s%02X"), PSTR("20;"), PKSequenceNumber++);
  strcat(pbuffer, dbuffer);
  // sprintf_P(jsonBuffer, PSTR("%s"), PSTR("{"));
  jsonDoc.clear();
}

// Plugin Name
void display_Name(const char *input)
{
  sprintf_P(dbuffer, PSTR(";%s"), input);
  strcat(pbuffer, dbuffer);
  sprintf_P(topicName, PSTR("%s"), input);
}

// Common Footer
void display_Footer(void)
{
  sprintf_P(dbuffer, PSTR("%s"), PSTR(";\r\n"));
  strcat(pbuffer, dbuffer);

  // sprintf_P(tempJsonbuffer, PSTR("%s"), PSTR("}"));
  // strcat(jsonBuffer, tempJsonbuffer);
}

// Start message
void display_Splash(void)
{
  sprintf_P(dbuffer, PSTR("%s%d.%d;BUILD=%s"), PSTR(";RFLink_ESP;VER="), BUILDNR, REVNR, PSTR(RFLINK_BUILDNAME));
  strcat(pbuffer, dbuffer);
}

// ID=9999 => device ID (often a rolling code and/or device channel number) (Hexadecimal)
void display_IDn(unsigned long input, byte n)
{
  switch (n)
  {
  case 2:
    addToBuffer("%02lx", "ID", input);
    sprintf_P(dbuffer, PSTR("%s%02lx"), PSTR("/"), input);
    break;
  case 4:
    addToBuffer("%04lx", "ID", input);
    sprintf_P(dbuffer, PSTR("%s%04lx"), PSTR("/"), input);
    break;
  case 6:
    addToBuffer("%06lx", "ID", input);
    sprintf_P(dbuffer, PSTR("%s%06lx"), PSTR("/"), input);
    break;
  case 8:
  default:
    addToBuffer("%08lx", "ID", input);
    sprintf_P(dbuffer, PSTR("%s%08lx"), PSTR("/"), input);
  }
  strcat(topicName, dbuffer);

}

// ID=9999 => device ID (often a rolling code) (Hexadecimal)
void display_CODE(unsigned long input, byte n)
{
  switch (n)
  {
  case 2:
    addToBuffer("%02lx", "CODE", input);
    break;
  case 4:
    addToBuffer("%04lx", "CODE", input);
    break;
  case 6:
    addToBuffer("%06lx", "CODE", input);
    break;
  case 8:
  default:
    addToBuffer("%08lx", "CODE", input);
  }
}

void display_IDc(const char *input)
{
  addToBuffer("'%s'", "ID", input);
}

// SWITCH=A16 => House/Unit code like A1, P2, B16 or a button number etc.
void display_SWITCH(byte input)
{
  addToBuffer("%02x", "SWITCH", input);
}

// SWITCH=A16 => House/Unit code like A1, P2, B16 or a button number etc.
void display_SWITCHc(const char *input)
{
  addToBuffer("%s", "SWITCH", input);
}

// CMD=ON => Command (ON/OFF/ALLON/ALLOFF) Additional for Milight: DISCO+/DISCO-/MODE0 - MODE8
void display_CMD(boolean all, byte on)
{
  switch (on)
  {
  case CMD_On:
    if (all == CMD_All)
    {
      addToBuffer("'%s'", "CMD", "ALLON");
    }
    else
    {
      addToBuffer("'%s'", "CMD", "ON");
    }
    break;
  case CMD_Off:
    if (all == CMD_All)
    {
      addToBuffer("'%s'", "CMD", "ALLOFF");
    }
    else
    {
      addToBuffer("'%s'", "CMD", "OFF");
    }
    break;
  case CMD_Bright:
    addToBuffer("'%s'", "CMD", "BRIGHT");
    break;
  case CMD_Dim:
    addToBuffer("'%s'", "CMD", "DIM");
    break;
  case CMD_Up:
    addToBuffer("'%s'", "CMD", "UP");
    break;
  case CMD_Down:
    addToBuffer("'%s'", "CMD", "DOWN");
    break;
  case CMD_Stop:
    addToBuffer("'%s'", "CMD", "STOP");
    break;
  case CMD_Pair:
    addToBuffer("'%s'", "CMD", "PAIR");
    break;
  case CMD_Unknown:
  default:
    addToBuffer("'%s'", "CMD", "UNKNOWN");
  }
}

void display_SIGNAL(uint8_t* frame, int RTS_ExpectedByteCount)
{
  sprintf_P(tempJsonbuffer, PSTR("signal: '"));
  strcat(jsonBuffer, tempJsonbuffer);

  for (int i = 0; i < RTS_ExpectedByteCount; i++)
  {
    sprintf_P(tempJsonbuffer, PSTR("%02x "), frame[i]);
    strcat(jsonBuffer, tempJsonbuffer);
  }
  sprintf_P(tempJsonbuffer, PSTR("'"));
  strcat(jsonBuffer, tempJsonbuffer);
}

// SET_LEVEL=15 => Direct dimming level setting value (decimal value: 0-15)
void display_SET_LEVEL(byte input)
{
  addToBuffer("%02d", "SET_LEVEL", input);
}

// TEMP=9999 => Temperature celcius (hexadecimal), high bit contains negative sign, needs division by 10
void display_TEMP(unsigned int input)
{
  addToBuffer("%04x", "TEMP", input);
}

// HUM=99 => Humidity (decimal value: 0-100 to indicate relative humidity in %)
void display_HUM(byte input)
{
  addToBuffer("%02d", "HUM", input);
}

// BARO=9999 => Barometric pressure (hexadecimal)
void display_BARO(unsigned int input)
{
  addToBuffer("%04x", "BARO", input);
}

// HSTATUS=99 => 0=Normal, 1=Comfortable, 2=Dry, 3=Wet
void display_HSTATUS(byte input)
{
  addToBuffer("%02x", "HSTATUS", input);
}

// BFORECAST=99 => 0=No Info/Unknown, 1=Sunny, 2=Partly Cloudy, 3=Cloudy, 4=Rain
void display_BFORECAST(byte input)
{
  addToBuffer("%02x", "BFORECAST", input);
}

// UV=9999 => UV intensity (hexadecimal)
void display_UV(unsigned int input)
{
  addToBuffer("%04x", "UV", input);
}

// LUX=9999 => Light intensity (hexadecimal)
void display_LUX(unsigned int input)
{
  addToBuffer("%04x", "LUX", input);
}

// BAT=OK => Battery status indicator (OK/LOW)
void display_BAT(boolean input)
{
  if (input == true)
    addToBuffer("'%s'", "BAT", "OK");
  else
    addToBuffer("'%s'", "BAT", "LOW");
}

// RAIN=1234 => Total rain in mm. (hexadecimal) 0x8d = 141 decimal = 14.1 mm (needs division by 10)
void display_RAIN(unsigned int input)
{
  addToBuffer("%04x", "RAIN", input);
}

// RAINRATE=1234 => Rain rate in mm. (hexadecimal) 0x8d = 141 decimal = 14.1 mm (needs division by 10)
void display_RAINRATE(unsigned int input)
{
  addToBuffer("%04x", "RAINRATE", input);
}

// WINSP=9999 => Wind speed in km. p/h (hexadecimal) needs division by 10
void display_WINSP(unsigned int input)
{
  addToBuffer("%04x", "WINSP", input);
}

// AWINSP=9999 => Average Wind speed in km. p/h (hexadecimal) needs division by 10
void display_AWINSP(unsigned int input)
{
  addToBuffer("%04x", "AWINSP", input);
}

// WINGS=9999 => Wind Gust in km. p/h (hexadecimal)
void display_WINGS(unsigned int input)
{
  addToBuffer("%04x", "WINGS", input);
}

// WINDIR=123 => Wind direction (integer value from 0-15) reflecting 0-360 degrees in 22.5 degree steps
void display_WINDIR(unsigned int input)
{
  addToBuffer("%03d", "WINDIR", input);
}

// WINCHL => wind chill (hexadecimal, see TEMP)
void display_WINCHL(unsigned int input)
{
  addToBuffer("%04x", "WINCHL", input);
}

// WINTMP=1234 => Wind meter temperature reading (hexadecimal, see TEMP)
void display_WINTMP(unsigned int input)
{
  addToBuffer("%04x", "WINTMP", input);
}

// CHIME=123 => Chime/Doorbell melody number
void display_CHIME(unsigned int input)
{
  addToBuffer("%03d", "CHIME", input);
}

// SMOKEALERT=ON => ON/OFF
void display_SMOKEALERT(boolean input)
{
  if (input == SMOKE_On)
    addToBuffer("'%s'", "SMOKEALERT", "ON");
  else
    addToBuffer("'%s'", "SMOKEALERT", "OFF");
}

// PIR=ON => ON/OFF
void display_PIR(boolean input)
{
  if (input == PIR_On)
    addToBuffer("'%s'", "PIR", "ON");
  else
    addToBuffer("'%s'", "PIR", "OFF");
}

// CO2=1234 => CO2 air quality
void display_CO2(unsigned int input)
{
  addToBuffer("%04d", "CO2", input);
}

// SOUND=1234 => Noise level
void display_SOUND(unsigned int input)
{
  addToBuffer("%04d", "SOUND", input);
}

// KWATT=9999 => KWatt (hexadecimal)
void display_KWATT(unsigned int input)
{
  addToBuffer("%04x", "KWATT", input);
}

// WATT=9999 => Watt (hexadecimal)
void display_WATT(unsigned int input)
{
  addToBuffer("%04x", "WATT", input);
}

// CURRENT=1234 => Current phase 1
void display_CURRENT(unsigned int input)
{
  addToBuffer("%04d", "CURRENT", input);
}

// DIST=1234 => Distance
void display_DIST(unsigned int input)
{
  addToBuffer("%04d", "DIST", input);
}

// METER=1234 => Meter values (water/electricity etc.)
void display_METER(unsigned int input)
{
  addToBuffer("%04d", "METER", input);
}

// VOLT=1234 => Voltage
void display_VOLT(unsigned int input)
{
  addToBuffer("%04d", "VOLT", input);
}

// RGBW=9999 => Milight: provides 1 byte color and 1 byte brightness value
void display_RGBW(unsigned int input)
{
  addToBuffer("%04x", "RGBW", input);
}

// Channel
void display_CHAN(byte channel)
{
  addToBuffer("%04x", "CHN", channel);
}

void addToBuffer(const char* format, const char* fieldName, const void* value )  {
  sprintf_P(dbuffer, PSTR(";%s=%s"), format);

  sprintf_P(dbuffer, dbuffer, fieldName, value);
  strcat(pbuffer, dbuffer);

  sprintf_P(dbuffer, PSTR("'%s': %s,"), format);

  //const char* lowername = tolower(fieldName);

  sprintf_P(dbuffer, dbuffer, fieldName, value);
  strcat(jsonBuffer, dbuffer);

  sprintf_P(dbuffer, format, value);
  jsonDoc[fieldName] = dbuffer;
}


// --------------------- //
// get label shared func //
// --------------------- //

char retrieveBuffer[INPUT_COMMAND_SIZE];
char *ptr;
const char c_delim[2] = ";";
char c_label[12];

void retrieve_Init()
{
  memcpy(retrieveBuffer, InputBuffer_Serial, INPUT_COMMAND_SIZE);
  ptr = strtok(retrieveBuffer, c_delim);
}

boolean retrieve_Name(const char *c_Name)
{
  if (ptr != NULL)
  {
    if (strncasecmp(ptr, c_Name, strlen(c_Name)) != 0)
      return false;
    ptr = strtok(NULL, c_delim);
    return true;
  }
  else
    return false;
}

boolean retrieve_hasPrefix(const char* prefix)
{
  if (ptr != NULL && prefix != NULL)
  {
    int prefixLength = strlen(prefix);
    if (strncasecmp(ptr, prefix, prefixLength) != 0)
      return false;
    ptr += prefixLength;
    return true;
  }
  else
    return false;
}

boolean retrieve_decimalNumber(unsigned long &value, byte maxDigits, const char* prefix)
{
  if (ptr != NULL)
  {
    if ((prefix != NULL) && (strncasecmp(ptr, prefix, strlen(prefix)) == 0))
      ptr += strlen(prefix);

    if (strlen(ptr) > maxDigits)
      return false;

    for (byte i = 0; i < strlen(ptr); i++)
      if (!isdigit(ptr[i]))
        return false;

    value = strtoul(ptr, NULL, DEC);

    ptr = strtok(NULL, c_delim);
    return true;
  }

  return false;
}

boolean retrieve_hexNumber(unsigned long &value, byte maxNibbles, const char* prefix)
{
  if (ptr != NULL)
  {
    if ((prefix != NULL) && (strncasecmp(ptr, prefix, strlen(prefix)) == 0))
      ptr += strlen(prefix);

    if (strlen(ptr) > maxNibbles)
      return false;

    for (byte i = 0; i < strlen(ptr); i++)
      if (!isxdigit(ptr[i]))
        return false;

    value = strtoul(ptr, NULL, HEX);

    ptr = strtok(NULL, c_delim);
    return true;
  }

  return false;
}

boolean retrieve_Command(byte &value, const char* prefix)
{
  if (ptr != NULL)
  {
    if ((prefix != NULL) && (strncasecmp(ptr, prefix, strlen(prefix)) == 0))
      ptr += strlen(prefix);

    if (strlen(ptr) > 7)
      return false;

    for (byte i = 0; i < strlen(ptr); i++)
      if (!isalnum(ptr[i]))
        return false;

    value = str2cmd(ptr); // Get ON/OFF etc. command
    if (value != false)
        ptr = strtok(NULL, c_delim);

    return (value != false);
  }

  return false;
}

boolean retrieve_long(unsigned long &value, const char* prefix)
{
    return retrieve_hexNumber(value, 8, prefix);
}

boolean retrieve_word(uint16_t &value, const char* prefix)
{
    unsigned long retrievedValue;
    bool result = retrieve_hexNumber(retrievedValue, 4, prefix);
    if (result)
        value = (uint16_t)retrievedValue;
    return result;
}

boolean retrieve_byte(byte &value, const char* prefix)
{
    unsigned long retrievedValue;
    bool result = retrieve_hexNumber(retrievedValue, 2, prefix);
    if (result)
        value = (byte)retrievedValue;
    return result;
}

boolean retrieve_nibble(byte &value, const char* prefix)
{
    unsigned long retrievedValue;
    bool result = retrieve_hexNumber(retrievedValue, 1, prefix);
    if (result)
        value = (byte)retrievedValue;
    return result;
}

boolean retrieve_ID(unsigned long &ul_ID)
{
    boolean result = retrieve_long(ul_ID, "ID=");
    if (result)
        ul_ID &= 0x03FFFFFF;
    return result;
}

boolean retrieve_Switch(byte &b_Switch)
{
    return retrieve_nibble(b_Switch, "SWITCH=");
}

boolean retrieve_Command(byte &b_Cmd)
{
    return retrieve_Command(b_Cmd, "CMD=");
}

boolean retrieve_Command(byte &b_Cmd, byte &b_Cmd2)
{
  // Command
  char c_Cmd[10];

  if (ptr != NULL)
  {
    strcpy(c_label, "SET_LEVEL=");
    if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
      ptr += strlen(c_label);

    strcpy(c_label, "CMD=");
    if (strncasecmp(ptr, c_label, strlen(c_label)) == 0)
      ptr += strlen(c_label);

    if (strlen(ptr) > 7)
      return false;

    for (byte i = 0; i < strlen(ptr); i++)
      if (!isalnum(ptr[i]))
        return false;

    strcpy(c_Cmd, ptr);

    b_Cmd2 = str2cmd(c_Cmd); // Get ON/OFF etc. command
    if (b_Cmd2 == false)     // Not a valid command received? ON/OFF/ALLON/ALLOFF
      b_Cmd2 = (byte)strtoul(c_Cmd, NULL, HEX);
    // ON
    switch (b_Cmd2)
    {
    case VALUE_ON:
    case VALUE_ALLON:
      b_Cmd |= B01;
      break;
    }
    // Group
    switch (b_Cmd2)
    {
    case VALUE_ALLON:
    case VALUE_ALLOFF:
      b_Cmd |= B10;
      break;
    }
    // Dimmer
    switch (b_Cmd2)
    {
    case VALUE_ON:
    case VALUE_OFF:
    case VALUE_ALLON:
    case VALUE_ALLOFF:
      b_Cmd2 = 0xFF;
      break;
    }

    ptr = strtok(NULL, c_delim);
    return true;
  }
  else
    return false;
}

boolean retrieve_End()
{
  // End
  if (ptr != NULL)
    return false;
  return true;
}

/*********************************************************************************************\
   Convert string to command code
\*********************************************************************************************/
int str2cmd(const char *command)
{
  if (strcasecmp(command, "ON") == 0)
    return VALUE_ON;
  if (strcasecmp(command, "OFF") == 0)
    return VALUE_OFF;
  if (strcasecmp(command, "ALLON") == 0)
    return VALUE_ALLON;
  if (strcasecmp(command, "ALLOFF") == 0)
    return VALUE_ALLOFF;
  if (strcasecmp(command, "PAIR") == 0)
    return VALUE_PAIR;
  if (strcasecmp(command, "DIM") == 0)
    return VALUE_DIM;
  if (strcasecmp(command, "BRIGHT") == 0)
    return VALUE_BRIGHT;
  if (strcasecmp(command, "UP") == 0)
    return VALUE_UP;
  if (strcasecmp(command, "DOWN") == 0)
    return VALUE_DOWN;
  if (strcasecmp(command, "STOP") == 0)
    return VALUE_STOP;
  if (strcasecmp(command, "CONFIRM") == 0)
    return VALUE_CONFIRM;
  if (strcasecmp(command, "LIMIT") == 0)
    return VALUE_LIMIT;
  return false;
}

void replacechar(char *str, char orig, char rep)
{
  char *ix = str;
  int n = 0;
  while ((ix = strchr(ix, orig)) != NULL)
  {
    *ix++ = rep;
    n++;
  }
}

#if defined(ESP8266) && !defined(TARGET_BOARD_ESP8285)
uint8_t String2GPIO(String sGPIO)
{
  byte num_part;
  char cGPIO[4];

  sGPIO.toCharArray(cGPIO, 4);

  if (strlen(cGPIO) != 2)
    return NOT_A_PIN;
  if (cGPIO[0] != 'D')
    return NOT_A_PIN;
  if (isdigit(cGPIO[1]))
    num_part = (cGPIO[1] - '0');
  else
    return NOT_A_PIN;

  switch (num_part)
  {
  case 0:
    return D0;
    break;
  case 1:
    return D1;
    break;
  case 2:
    return D2;
    break;
  case 3:
    return D3;
    break;
  case 4:
    return D4;
    break;
  case 5:
    return D5;
    break;
  case 6:
    return D6;
    break;
  case 7:
    return D7;
    break;
  case 8:
    return D8;
    break;
  default:
    return NOT_A_PIN;
  }
}

String GPIO2String(uint8_t uGPIO)
{
  switch (uGPIO)
  {
  case D0:
    return "D0";
    break;
  case D1:
    return "D1";
    break;
  case D2:
    return "D2";
    break;
  case D3:
    return "D3";
    break;
  case D4:
    return "D4";
    break;
  case D5:
    return "D5";
    break;
  case D6:
    return "D6";
    break;
  case D7:
    return "D7";
    break;
  case D8:
    return "D8";
    break;
  default:
    return "NOT_A_PIN";
  }
}
#else // ESP32 or 8285
uint8_t String2GPIO(String sGPIO)
{
  char cGPIO[4];

  sGPIO.trim();
  sGPIO.toCharArray(cGPIO, 4);

  switch (strlen(cGPIO))
  {
  case 1:
    if (isdigit(cGPIO[0]))
      return (cGPIO[0] - '0');
  case 2:
    if ((isdigit(cGPIO[0])) && (isdigit(cGPIO[1])))
      return ((cGPIO[0] - '0') * 10 + (cGPIO[1] - '0'));
  default:
    return NOT_A_PIN;
  }
}

String GPIO2String(uint8_t uGPIO)
{
  if (uGPIO < 40)
    return String(uGPIO);
  else
    return "NOT_A_PIN";
}
#endif // ESP32
