//#define Xconfiguration
#ifndef configuration_hhh
#define configuration_hhh


int baudRateSerial = 115200;

char mqtt_server[40] = "192.168.0.55";
char mqtt_port[6] = "1883";

char idSystem[40] ="1";
//const char* idESP8266;
String idESP8266;


//flag for saving data
bool shouldSaveConfig = false;

/**
 * Flags Pins
 *
const bool flagRELAY1 = true;
const bool flagRELAY2 = true;
const bool flagRELAY3 = true;
const bool flagRELAY4 = true;

const bool flagDHT = true;
const bool flagHUM = true;
*/
/*
 * 
 */
//String channel = String(idSystem) + "/" + String(idESP8266) + "/%s/" + i + "/%s";
String channelFormat;
char channel_char[100]; 
//char *channel;
String channel;
/**
 * Configurações de Pins
 *
const int pinRELAY1 = 15;
const int pinRELAY2 = 14;
const int pinRELAY3 = 12;
const int pinRELAY4 = 13;

const int pinHum = A0;
*/
const int DHTPIN = 2;
const int DHTTYPE = 22;

// Pin configuratie
const int MCP3008_CLOCK_PIN=2;
const int MCP3008_MISO_PIN=4;
const int MCP3008_MOSI_PIN=5;
const int MCP3008_CS_PIN=15;


//Turbidity Voltage Reference
const float VClearWater = 2.70;

//{DHT, Relay1, Relay2, Relay3, Relay4, Analog, HUM, Turbidity,...};
const bool flagArray[] = {true, true, true, true, true, true, true, true};

//{DHT, Relay1, Relay2, Relay3, Relay4, Analog, HUM, Turbidity,...};
const int pinArray[] = {0, 16, 14, 12, 13, A0, 0, 1};

//{DHT, Relay1, Relay2, Relay3, Relay4, Analog, HUM, Turbidity,...};
const int updateTimeArray[] = {10000, 10000, 10000, 10000, 10000, 10000, 10000};

//{DHT, Relay1, Relay2, Relay3, Relay4, Analog, HUM, Turbidity,...};
long lastUpdateTimeArray[] = {0, 0, 0, 0, 0, 0, 0, 0};
#endif
