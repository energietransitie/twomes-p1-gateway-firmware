#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <utils.h>
#include <espnow_settings.h>
#include <ArduinoJson.h> //this is used for parsing json
#include <esp_now.h>
//#include <BLE.h>    //not enough flash on Adafruit Huzzah ESP32

//debug Messages switching
#define WiFiconfig_debug_messages 1
bool printAllData = true;
bool printDecodedData = true;

#define MAXTELEGRAMLENGTH 1500    // Length of chars to read for decoding
#define MAXSENDMESSAGELENGTH 1500 //review this, maybe this could be dynamic allocated

//Pin setup
HardwareSerial P1Poort(2);     // Use UART2
const int dataReceivePin = 16; // Data receive pin
const int dataReqPin = 26;     // Request pin on 26
const int redLed = 18;
const int greenLed = 23; // Tijdelijke GPIO 5, moet GPIO 23 zijn!
const int button = 19;

char telegram[MAXTELEGRAMLENGTH]; // Variable for telegram data

char sendMessage[MAXSENDMESSAGELENGTH]; //REVIEW, this memory should be allocated in sender function

//defines for max amount of memory allocated for measurements
#define maxMeasurements_smartMeter 10
#define maxMeasurements_roomTemp 10
#define maxMeasurements_boilerTemp 10

#define macArrayLength 6 //number of position in a mac address

//defines for selecting send_mode
#define numberOfModes 3
#define smartMeter_send_mode 0
#define roomTemp_send_mode 1
#define boilerTemp_send_mode 2

//define dataTypes
#define smartMeter_dataType 0
#define roomTemp_dataType 1
#define boilerTemp_dataType 2

//define returns from json parser
#define return_parse_data_into_json_error 0
#define return_parse_data_into_json_smartMeterData 1
#define return_parse_data_into_json_roomTempData 2
#define return_parse_data_into_json_boilerTempData 3

struct serverCredentials //for saving server connection settings
{
  const char *serverAddress = defaultserverAddress;
  const uint16_t serverPort = defaultServerPort;
  const char *serverURI[numberOfModes] = {smartMeterUri, roomTempUri, boilerTempUri};
} serverCredentials1; //allocate memory for this

typedef struct global_data_save_format
{
  //for specifying data and locating it
  uint8_t dataType;     //see defines above
  uint8_t dataPosition; //position in data struct

  //for building JSON
  byte macID[macArrayLength];   //id: MAC from device which took the sample(s)
  uint64_t lastTime;            //lastTime: time at latest sample
  uint8_t interval;             //interval: time between samples
  uint8_t numberOfMeasurements; //total: number of Measurements in this message
} global_data_save_format;

global_data_save_format global_sample_data[maxMeasurements_smartMeter + maxMeasurements_roomTemp + maxMeasurements_boilerTemp]; //this place should be able to hold data from all sources

uint8_t amount_filled_global_data_positions = 0, amount_filled_smartMeter_positions = 0, amount_filled_roomTemp_positions = 0, amount_filled_boilerTemp_positions = 0; //counts saved measurements by type

struct smartMeterData
{
  byte dsmrVersion;            // DSMR versie zonder punt
  long elecUsedT1;             // Elektriciteit verbruikt tarief 1 in Watt
  long elecUsedT2;             // Elektriciteit verbruikt tarief 2 in Watt
  long elecDeliveredT1;        // Elektriciteit geleverd tarief 1 in Watt
  long elecDeliveredT2;        // Elektriciteit geleverd tarief 1 in Watt
  byte currentTarrif;          // Huidig tafief
  long elecCurrentUsage;       // Huidig elektriciteitsverbruik In Watt
  long elecCurrentDeliver;     // Huidig elektriciteit levering in Watt
  long gasUsage;               // Gasverbruik in dm3
  char timeGasMeasurement[12]; // Tijdstip waarop gas voor het laats is gemeten YY:MM:DD:HH:MM:SS
  //char timeRead[12];           // Tijdstip waarop meter voor het laats is uitgelezen YY:MM:DD:HH:MM:SS
};
struct smartMeterData smartMeter_measurement[maxMeasurements_smartMeter];

struct roomTempData
{
  float roomTemps[maximum_samples_espnow]; //add variables for receiving data from roomTemp monitor device
};
struct roomTempData roomTemp_measurement[maxMeasurements_roomTemp];

struct boilerTempData
{
  float pipeTemps1[maximum_samples_espnow];
  float pipeTemps2[maximum_samples_espnow]; //add variables for receiving data from boiler monitor device
};
struct boilerTempData boilerTemp_measurement[maxMeasurements_boilerTemp];

typedef struct ESP_message
{
  uint8_t numberofMeasurements;
  uint8_t intervalTime;
  float pipeTemps1[maximum_samples_espnow];
  float pipeTemps2[maximum_samples_espnow];
} ESP_message;

byte myMac[macArrayLength]; //to save my Mac address

// CRC
unsigned int currentCRC = 0;

// Function declaration
boolean WiFiconfig(boolean);
boolean ESPnowconfig(boolean);
void getData(int);
void printData(int, int);
int FindCharInArrayRev(char[], char, int, int);
void printValue(int, int);
bool checkValues(int, bool);
void decodeTelegram(int, int);
bool procesData(int);
bool crcCheck(int, int);
unsigned int CRC16(unsigned int, unsigned char *, int);
boolean sender();
String convertToString(char[]);
void makePostRequest();
boolean makePostRequest2(uint8_t);
uint8_t parse_data_into_json(uint8_t *selectedPosition);
void interruptButton();

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  //char macStr[18];
  //Serial.print("Packet received from: ");
  //snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
  // mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Serial.println(macStr);

  ESP_message received_ESPNOW_message;

  memcpy(&received_ESPNOW_message, incomingData, sizeof(received_ESPNOW_message));

  // if (amount_filled_global_data_positions == (maxMeasurements_smartMeter + maxMeasurements_roomTemp + maxMeasurements_boilerTemp)) //since memory allocation is static this is not needed
  // {
  //   printf("global_data_positions are full\n"); //REVIEW the old data should be renewed with the old data?
  //   return;
  // }

  //determine what type sensor this is
  switch (boilerTemp_dataType)
  {
  case roomTemp_dataType:
  {
    Serial.printf("This is not supported yet");
  }
  break;
  case boilerTemp_dataType:
  {
    if (amount_filled_boilerTemp_positions >= maxMeasurements_boilerTemp)
    {
      printf("boilerTemp_positions are full\n"); //REVIEW the old data should be renewed with the old data?
      return;
    }

    global_sample_data[amount_filled_global_data_positions].dataType = boilerTemp_dataType; //is now hardcoded boilerTemp, roomTemp not supported yet

    global_sample_data[amount_filled_global_data_positions].lastTime = (1611233085 + uint8_t(amount_filled_global_data_positions * 60)); //this should be the local time!

    for (uint8_t counter1 = 0; counter1 < macArrayLength; counter1++) //copy mac address
    {
      global_sample_data[amount_filled_global_data_positions].macID[counter1] = mac_addr[counter1];
    }

    global_sample_data[amount_filled_global_data_positions].numberOfMeasurements = received_ESPNOW_message.numberofMeasurements;
    global_sample_data[amount_filled_global_data_positions].interval = received_ESPNOW_message.intervalTime;
    global_sample_data[amount_filled_global_data_positions].dataPosition = amount_filled_boilerTemp_positions;

    for (uint8_t counter1 = 0; counter1 < (received_ESPNOW_message.numberofMeasurements); counter1++)
    {
      boilerTemp_measurement[amount_filled_boilerTemp_positions].pipeTemps1[counter1] = received_ESPNOW_message.pipeTemps1[counter1];
      boilerTemp_measurement[amount_filled_boilerTemp_positions].pipeTemps2[counter1] = received_ESPNOW_message.pipeTemps2[counter1];
    }
    amount_filled_boilerTemp_positions++;
  }
  break;
  }
  amount_filled_global_data_positions++;
}

void setup()
{
  esp_read_mac(myMac, ESP_MAC_WIFI_STA);
  // Start terminal communication
  Serial.begin(115200);
  Serial.println("Starting...");
  WiFi.mode(WIFI_MODE_STA);
  Serial.print("MAC address of this gateway: ");
  Serial.println(WiFi.macAddress());
  WiFi.mode(WIFI_OFF);

  // // Pin setup
  pinMode(dataReqPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(dataReqPin, HIGH);
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, HIGH);

  // Attatch interrupt to button
  // attachInterrupt(button, interruptButton, FALLING);

  // Configure P1Poort serial connection
  P1Poort.begin(115200, SERIAL_8N1, dataReceivePin, -1); // Start HardwareSerial. RX, TX
  //WiFiconfig(true);

  //start_BLE_server();     //not enough flash on Adafruit Huzzah ESP32
  printf("ESPnowconfig enabled return: %d\n", ESPnowconfig(true));
}

uint8_t tempSave_amount_filled_global_data_positions = 0;
uint64_t lastSendTime = 0, last_smartMeter_contact = 0; //to save last time

void loop()
{
  if (amount_filled_global_data_positions != tempSave_amount_filled_global_data_positions)
  {
    tempSave_amount_filled_global_data_positions = amount_filled_global_data_positions;
    Serial.printf("amount_filled_global_data_positions: %u\n", amount_filled_global_data_positions);
    Serial.printf("amount_filled_smartMeter_positions: %u\n", amount_filled_smartMeter_positions);
    Serial.printf("amount_filled_boilerTemp_positions: %u\n", amount_filled_boilerTemp_positions);
    Serial.printf("amount_filled_roomTemp_positions: %u\n", amount_filled_roomTemp_positions);
  }
#define start_send_from_global_data_positions 5 //filled global data positions should be this value before send action will start
#define start_send_with_interval_time 10        //in seconds,
#define smartMeter_get_interval_time 5          //in seconds
#define enableSmartmeter 1

  if ((millis() - last_smartMeter_contact) > (smartMeter_get_interval_time * 1000) && enableSmartmeter)
  {
    // if (amount_filled_global_data_positions == (maxMeasurements_smartMeter + maxMeasurements_roomTemp + maxMeasurements_boilerTemp)) //since memory allocation is static this is not needed
    // {
    //   printf("global_data_positions are full\n"); //REVIEW the old data should be renewed with the old data?
    //   return;
    // }
    if (amount_filled_smartMeter_positions >= maxMeasurements_smartMeter)
    {
      printf("smartMeter_positions are full\n"); //REVIEW the old data should be renewed with the old data?
    }
    else
    {
      printf("Doing a smartMeter measurment now\n");
      global_sample_data[amount_filled_global_data_positions].dataType = smartMeter_dataType; //is now hardcoded boilerTemp, roomTemp not supported yet

      global_sample_data[amount_filled_global_data_positions].lastTime = (1611233085 + uint8_t(amount_filled_global_data_positions * 60)); //this should be the local time!

      for (uint8_t counter1 = 0; counter1 < macArrayLength; counter1++) //copy mac address from this gateway
      {
        global_sample_data[amount_filled_global_data_positions].macID[counter1] = myMac[counter1];
      }

      global_sample_data[amount_filled_global_data_positions].numberOfMeasurements = 1;
      global_sample_data[amount_filled_global_data_positions].interval = 0;
      global_sample_data[amount_filled_global_data_positions].dataPosition = amount_filled_smartMeter_positions;

      memset(telegram, 0, sizeof(telegram)); // Empty telegram
      int maxRead = 0;
      getData(maxRead);

      //config Serial port
      //get data en put in data storage
      last_smartMeter_contact = millis();
    }
  }
  else if ((amount_filled_global_data_positions >= start_send_from_global_data_positions) && ((millis() - lastSendTime) > (start_send_with_interval_time * 1000))) //there is data to send, so do this
  {
    ESPnowconfig(false);
    if (WiFiconfig(true))
    {
      while (amount_filled_global_data_positions > 0 && sender())
      {
        Serial.printf("Sending thing now, amount_filled_global_data_positions: %u\n", amount_filled_global_data_positions);
      }
      WiFiconfig(false);
      ESPnowconfig(true);
    }
    else
    {
      Serial.printf("WiFiconfig fail\n");
      delay(1000);

      WiFiconfig(false);
      ESPnowconfig(true);
    }
    lastSendTime = millis();
  }
}

boolean WiFiconfig(boolean requestedState)
{
  if (requestedState)
  {
#if WiFiconfig_debug_messages
    Serial.println("Connecting to WiFi");
#endif
    if (WiFi.mode(WIFI_STA) == false)
    {
      Serial.printf("Wifi mode error\n");
      return false;
    }
    WiFi.begin(SSID, PASS);
    // if (WiFi.begin(SSID, PASS) != WL_CONNECT_FAILED)
    // {
    //   Serial.printf("Wifi credentials error\n");
    //   return false;
    // }
    while (WiFi.status() != WL_CONNECTED) //REVIEW, here is an timeout needed
    {
#if WiFiconfig_debug_messages
      Serial.print(".");
#endif
      delay(50);
    }
#if WiFiconfig_debug_messages
    Serial.println("\nConnected to WiFi");
    return true;
#endif
  }
  else
  {
    WiFi.mode(WIFI_OFF);
    printf("WiFi.getMode: %d\n", WiFi.getMode());
    if (WiFi.getMode() == WIFI_MODE_NULL)
    {
      return true;
    }
  }
  Serial.printf("WiFiconfig fault\n");
  return false;
}

//input:  requested state of ESP-NOW communication
//output: true if succeed, otherwise false
//effect: sets ESP-NOW communication in requested state
boolean ESPnowconfig(boolean requestedState)
{
  if (requestedState)
  {
    //digitalWrite(measureTimePin1, LOW);
    //WiFi.softAP("bullshit", "bulllshit", espnow_channel); //REVIEW: use this to change wifi radio channel, but this should be an other function
    WiFi.mode(WIFI_STA); //REVIEW: check if this is necessary
    if (esp_now_init() == ESP_OK)
    {
      if (esp_now_register_recv_cb(OnDataRecv) == ESP_OK) // Once ESPNow is successfully Init, we will register for recv CB to, get recv packer info
      {
        return true;
      }
    }
  }
  else
  {
    if (esp_now_unregister_recv_cb() == ESP_OK)
    {
      if (esp_now_deinit() == ESP_OK)
      {
        WiFi.mode(WIFI_OFF);
        return true;
      }
    }
  }
  Serial.println("ESPNOW config fault");
  return false;
}

// This function gets called when button is pressed
void interruptButton()
{
}

// Reads data and calls function procesData(int) to proces the data
void getData(int maxRead)
{
  while (P1Poort.available())
  { // Buffer leegmaken
    P1Poort.read();
  }
  digitalWrite(dataReqPin, LOW); // Request data

  int counter = 0;
  int timeOutCounter = 0;
  bool startFound = false;
  bool endFound = false;
  bool crcFound = false;

  while (!crcFound && timeOutCounter < 15000)
  { // Reads data untill one telegram is fully readed and copied, after not receiving data for 15 seconds the loop exits
    if (P1Poort.available())
    {
      char c = P1Poort.read();
      char inChar = (char)c;

      if (!startFound)
      { // If start is not found yes, wait for start symbol
        if (inChar == '/')
        {
          startFound = true;
          telegram[counter] = inChar;
          counter++;
        }
      }
      else if (!endFound)
      { // If start is found, copy incoming char into telegram until end symbol is found
        telegram[counter] = inChar;
        counter++;
        if (inChar == '!')
        {
          endFound = true;
        }
      }
      else
      { // If end symbol is found, copy incoming char into telegram until \n is found
        telegram[counter] = inChar;
        counter++;
        if (inChar == '\n')
        {
          crcFound = true;
        }
      }
    }
    else
    { // Wait 1 ms if no data available
      timeOutCounter++;
      delay(1);
    }
  }
  maxRead++;
  digitalWrite(dataReqPin, HIGH); // Stop requesting data
  procesData(maxRead);
}

// Checks if data a fully telegram is readed including CRC. If not, data is reread by calling function getData(int) for a maximum of 10 times per loop.
// Also calls function decodeTelegram to attach the readed parameters to variables
bool procesData(int maxRead)
{
  int startChar = FindCharInArrayRev(telegram, '/', sizeof(telegram), 0);       // Plaats van eerste char uit één diagram
  int endChar = FindCharInArrayRev(telegram, '!', sizeof(telegram), startChar); // Plaats van laatste char uit één diagram

  if (startChar >= 0 && endChar >= 0 && (endChar + 5) < MAXTELEGRAMLENGTH)
  {

    if (printAllData) // For debugging!
      printData(startChar, endChar);

    if (printDecodedData)
    { // For debugging!
      decodeTelegram(startChar, endChar);
      if (crcCheck(startChar, endChar))
      {
        amount_filled_smartMeter_positions++;
        amount_filled_global_data_positions++;
        return true; // Return true if CRC check is valid
      }
      else
      {
        if (maxRead < 10)
        {
          Serial.println("CRC check not valid, reading data again");
          getData(maxRead);
        }
        else
        {
          Serial.println("CRC check not valid, maxRead has reached it's limit");
          return false; // Return false if CRC check is invalid
        }
      }
    }
  }
  else
  {
    if (maxRead < 10)
    {
      Serial.println("Not all data found, reading data again");
      //client.println("Not all data found, reading data again");
      //Data was not read correctly, try reading again
      getData(maxRead);
    }
    else
    {
      Serial.println("Not all data found, maxRead has reached it's limit");
      //client.println("Not all data found, maxRead has reached it's limit");
      return false;
    }
  }
  return false;
}

// Seperates each line and calls the function checkValues(int, bool) to attach the readed parameters to a variable
void decodeTelegram(int startChar, int endChar)
{
  int placeOfNewLine = startChar;
  bool foundStart = false;
  while (placeOfNewLine < endChar && placeOfNewLine >= 0)
  {
    yield();

    if (!foundStart)
    {
      foundStart = checkValues(placeOfNewLine, foundStart);
    }
    else
    {
      checkValues(placeOfNewLine, foundStart);
    }

    placeOfNewLine = FindCharInArrayRev(telegram, '\n', endChar - startChar, placeOfNewLine) + 1;
  }
}

// Checks to what variable the readed parameter belongs and assigns the parameter to the variable by calling the function updateValue(int, int)
// Also does CRC for each line
bool checkValues(int placeOfNewLine, bool foundStart)
{
  int endChar = FindCharInArrayRev(telegram, '\n', placeOfNewLine + 300, placeOfNewLine);
  if ((strncmp(telegram + placeOfNewLine, "/", strlen("/")) == 0) && !foundStart)
  {
    // start found. Reset CRC calculation
    currentCRC = CRC16(0x0000, (unsigned char *)telegram + placeOfNewLine, endChar + 1 - placeOfNewLine);
    return true;
  }
  else
  {
    currentCRC = CRC16(currentCRC, (unsigned char *)telegram + placeOfNewLine, endChar + 1 - placeOfNewLine);
  }

  long round;
  long decimal;
  if (strncmp(telegram + placeOfNewLine, "1-3:0.2.8", strlen("1-3:0.2.8")) == 0)
  { // DSMR-versie
    Serial.print("DSMR-versie: ");
    sscanf(telegram + placeOfNewLine, "1-3:0.2.8(%ld", &round);
    smartMeter_measurement[amount_filled_smartMeter_positions].dsmrVersion = round;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].dsmrVersion);
  }
  else if (strncmp(telegram + placeOfNewLine, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0)
  { // Elektriciteit verbruikt T1
    Serial.print("Elektriciteit verbruikt T1: ");
    sscanf(telegram + placeOfNewLine, "1-0:1.8.1(%ld.%ld", &round, &decimal);
    smartMeter_measurement[amount_filled_smartMeter_positions].elecUsedT1 = round * 1000 + decimal;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].elecUsedT1);
  }
  else if (strncmp(telegram + placeOfNewLine, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0)
  { // Elektriciteit verbruikt T2
    Serial.print("Elektriciteit verbruikt T2: ");
    sscanf(telegram + placeOfNewLine, "1-0:1.8.2(%ld.%ld", &round, &decimal);
    smartMeter_measurement[amount_filled_smartMeter_positions].elecUsedT2 = round * 1000 + decimal;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].elecUsedT2);
  }
  else if (strncmp(telegram + placeOfNewLine, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0)
  { // Elektriciteit geleverd T1
    Serial.print("Elektriciteit geleverd T1: ");
    sscanf(telegram + placeOfNewLine, "1-0:2.8.1(%ld.%ld", &round, &decimal);
    smartMeter_measurement[amount_filled_smartMeter_positions].elecDeliveredT1 = round * 1000 + decimal;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].elecDeliveredT1);
  }
  else if (strncmp(telegram + placeOfNewLine, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0)
  { // Elektriciteit geleverd T2
    Serial.print("Elektriciteit geleverd T2: ");
    sscanf(telegram + placeOfNewLine, "1-0:2.8.2(%ld.%ld", &round, &decimal);
    smartMeter_measurement[amount_filled_smartMeter_positions].elecDeliveredT2 = round * 1000 + decimal;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].elecDeliveredT2);
  }
  else if (strncmp(telegram + placeOfNewLine, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0)
  { // Huidig tarief
    Serial.print("Hudig tarief: ");
    sscanf(telegram + placeOfNewLine, "0-0:96.14.0(%ld", &round);
    smartMeter_measurement[amount_filled_smartMeter_positions].currentTarrif = round;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].currentTarrif);
  }
  else if (strncmp(telegram + placeOfNewLine, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0)
  { // Actueel energie verbruik
    Serial.print("Actueel elektriciteits verbruik: ");
    sscanf(telegram + placeOfNewLine, "1-0:1.7.0(%ld.%ld", &round, &decimal);
    smartMeter_measurement[amount_filled_smartMeter_positions].elecCurrentUsage = round * 1000 + decimal;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].elecCurrentUsage);
  }
  else if (strncmp(telegram + placeOfNewLine, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0)
  { // Actueel energie levering
    Serial.print("Actueel elektriciteit teruglevering: ");
    sscanf(telegram + placeOfNewLine, "1-0:2.7.0(%ld.%ld", &round, &decimal);
    smartMeter_measurement[amount_filled_smartMeter_positions].elecCurrentDeliver = round * 1000 + decimal;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].elecCurrentDeliver);
  }
  else if (strncmp(telegram + placeOfNewLine, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0)
  { // Gasverbruik + tijdstip gemeten
    Serial.print("Gasverbruik: ");
    sscanf(telegram + placeOfNewLine + strlen("0-1:24.2.1") + 15, "(%ld.%ld", &round, &decimal);
    smartMeter_measurement[amount_filled_smartMeter_positions].gasUsage = round * 1000 + decimal;
    Serial.println(smartMeter_measurement[amount_filled_smartMeter_positions].gasUsage);
    Serial.print("Tijdstip gas gemeten: ");
    int j = 0;
    int countUntil = placeOfNewLine + strlen("0-1:24.2.1") + sizeof(smartMeter_measurement[amount_filled_smartMeter_positions].timeGasMeasurement) + 1;
    for (int i = placeOfNewLine + strlen("0-1:24.2.1") + 1; i < countUntil; i++)
    {
      smartMeter_measurement[amount_filled_smartMeter_positions].timeGasMeasurement[j] = telegram[i];
      Serial.print(smartMeter_measurement[amount_filled_smartMeter_positions].timeGasMeasurement[j]);
      j++;
    }
    Serial.print("\n");
  }
  /*
  else if (strncmp(telegram + placeOfNewLine, "0-0:1.0.0", strlen("0-0:1.0.0")) == 0)
  { // Tijd data uitgelezen
    Serial.print("Tijd meter uitgelezen: ");
    int j = 0;
    int countUntil = placeOfNewLine + strlen("0-0:1.0.0") + sizeof(smartMeter_measurement[amount_filled_smartMeter_positions].timeRead) + 1;
    for (int i = placeOfNewLine + strlen("0-0:1.0.0") + 1; i < countUntil; i++)
    {
      smartMeter_measurement[amount_filled_smartMeter_positions].timeRead[j] = telegram[i];
      Serial.print(smartMeter_measurement[amount_filled_smartMeter_positions].timeRead[j]);
      j++;
    }
    Serial.print("\n");
  }*/

  return false;
}

// Assign readed parameters to it's corresponding variable TEMP FUNCTION
void printValue(int placeOfNewLine, int valuePlace)
{
  for (int i = placeOfNewLine + valuePlace; i < placeOfNewLine + 200; i++)
  {
    Serial.print(telegram[i]);
    if (telegram[i] == '\n')
    {
      break;
    }
  }
}

// Returns the place of a certain char in a char array
int FindCharInArrayRev(char array[], char c, int len, int startCountingFrom)
{
  for (int i = startCountingFrom; i < len; i++)
  {
    if (array[i] == c)
    {
      return i;
    }
  }
  return -1;
}

// Converts Char to String and returns it TEMP FUNCTION
String convertToString(char c[11])
{
  String s = "";
  for (int i = 0; i < 12; i++)
  {
    s += c[i];
  }
  return s;
}

// Prints all the data of one telegram TEMP FUNCTION
void printData(int startChar, int endChar)
{
  Serial.println("Data is:");
  for (int i = startChar; i < endChar + 5; i++)
  {
    Serial.print(telegram[i]);
  }
  Serial.println("");
}

// Performs the final CRC
bool crcCheck(int startChar, int endChar)
{
  currentCRC = CRC16(currentCRC, (unsigned char *)telegram + endChar, 1);
  char messageCRC[5];
  strncpy(messageCRC, telegram + endChar + 1, 4);
  messageCRC[4] = 0;
  bool validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);
  currentCRC = 0;

  if (validCRCFound)
  {
    Serial.println("\nVALID CRC FOUND!");
    return true;
  }
  else
  {
    Serial.println("\n===INVALID CRC FOUND!===");
    return false;
  }
  return false;
}

//Returns CRC
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
  for (int pos = 0; pos < len; pos++)
  {
    crc ^= (unsigned int)buf[pos]; // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--)
    { // Loop over each bit
      if ((crc & 0x0001) != 0)
      {            // If the LSB is set
        crc >>= 1; // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else         // Else LSB is not set
        crc >>= 1; // Just shift right
    }
  }

  return crc;
}

boolean sender()
{
  uint8_t position_to_send = (amount_filled_global_data_positions - 1); //select last measurement made
  uint8_t answerFromThis = parse_data_into_json(&position_to_send);
  switch (answerFromThis)
  {
  case return_parse_data_into_json_error:

    printf("error in parser\n");
    return false;
    break;

  case return_parse_data_into_json_smartMeterData:

    printf("Send smartMeterData\n");
    if (makePostRequest2(smartMeter_send_mode))
    {
      amount_filled_global_data_positions--;
      amount_filled_smartMeter_positions--;
      return true;
    }
    else
    {
      printf("Fault smartMeterData\n");
    }
    break;

  case return_parse_data_into_json_roomTempData:
    printf("Send roomTempData\n");
    if (makePostRequest2(roomTemp_send_mode))
    {
      amount_filled_global_data_positions--;
      amount_filled_roomTemp_positions--;
      return true;
    }
    else
    {
      printf("Fault roomTempData\n");
    }
    break;

  case return_parse_data_into_json_boilerTempData:
    printf("Send boilerTempData\n");
    if (makePostRequest2(boilerTemp_send_mode))
    {
      amount_filled_global_data_positions--;
      amount_filled_boilerTemp_positions--;
      return true;
    }
    else
    {
      printf("Fault boilerTempData\n");
    }
    break;
  }
  return false;
}

//output: 0: error, 1: parsed smartMeterData, 2: parsed roomTempData, 3: parsed boilerTempData
uint8_t parse_data_into_json(uint8_t *selectedPosition)
{
  StaticJsonDocument<MAXSENDMESSAGELENGTH> parsedJsonDoc; //maybe we could make this dynamic

  char tempMac[macArrayLength * 3];
  snprintf(tempMac, sizeof(tempMac), "%02x:%02x:%02x:%02x:%02x:%02x",
           global_sample_data[*selectedPosition].macID[0], global_sample_data[*selectedPosition].macID[1], global_sample_data[*selectedPosition].macID[2], global_sample_data[*selectedPosition].macID[3], global_sample_data[*selectedPosition].macID[4], global_sample_data[*selectedPosition].macID[5]);
  parsedJsonDoc["id"] = tempMac; //connect to correct data

  JsonObject dataSpec = parsedJsonDoc.createNestedObject("dataSpec");
  dataSpec["lastTime"] = 0;                                              //global_sample_data[*selectedPosition].lastTime; //connect to correct data
  dataSpec["interval"] = global_sample_data[*selectedPosition].interval; //correct to current data
  dataSpec["total"] = global_sample_data[*selectedPosition].numberOfMeasurements;

  JsonObject data = parsedJsonDoc.createNestedObject("data");

  switch (global_sample_data[*selectedPosition].dataType)
  {

  case smartMeter_dataType:
  {
    JsonArray dsmr = data.createNestedArray("dsmr");
    JsonArray evt1 = data.createNestedArray("evt1");
    JsonArray evt2 = data.createNestedArray("evt2");
    JsonArray egt1 = data.createNestedArray("egt1");
    JsonArray egt2 = data.createNestedArray("egt2");
    JsonArray ht = data.createNestedArray("ht");
    JsonArray ehv = data.createNestedArray("ehv");
    JsonArray ehl = data.createNestedArray("ehl");
    JsonArray gas = data.createNestedArray("gas");
    JsonArray tgas = data.createNestedArray("tgas");
    for (byte index1 = 0; index1 < global_sample_data[*selectedPosition].numberOfMeasurements; index1++)
    {
      dsmr.add(smartMeter_measurement[index1].dsmrVersion);
      evt1.add(smartMeter_measurement[index1].elecUsedT1);
      evt2.add(smartMeter_measurement[index1].elecUsedT2);
      egt1.add(smartMeter_measurement[index1].elecDeliveredT1);
      egt2.add(smartMeter_measurement[index1].elecDeliveredT2);
      ht.add(smartMeter_measurement[index1].currentTarrif);
      ehv.add(smartMeter_measurement[index1].elecCurrentUsage);
      ehl.add(smartMeter_measurement[index1].elecCurrentDeliver);
      gas.add(smartMeter_measurement[index1].gasUsage);
      tgas.add(smartMeter_measurement[index1].timeGasMeasurement);
    }
    serializeJson(parsedJsonDoc, sendMessage);
    return return_parse_data_into_json_smartMeterData;
  }
  break;

  case roomTemp_dataType:
  {
    JsonArray roomTemp = data.createNestedArray("roomTemp");
    for (byte index1 = 0; index1 < global_sample_data[*selectedPosition].numberOfMeasurements; index1++)
    {
      roomTemp.add(roomTemp_measurement[global_sample_data[*selectedPosition].dataPosition].roomTemps[index1]); //connect to correct data
    }
    serializeJson(parsedJsonDoc, sendMessage);
    return return_parse_data_into_json_roomTempData;
  }
  break;

  case boilerTemp_dataType:
  {
    JsonArray pipeTemp1 = data.createNestedArray("pipeTemp1");
    JsonArray pipeTemp2 = data.createNestedArray("pipeTemp2");
    for (byte index1 = 0; index1 < global_sample_data[*selectedPosition].numberOfMeasurements; index1++)
    {
      pipeTemp1.add(boilerTemp_measurement[global_sample_data[*selectedPosition].dataPosition].pipeTemps1[index1]); //connect to correct data
      pipeTemp2.add(boilerTemp_measurement[global_sample_data[*selectedPosition].dataPosition].pipeTemps2[index1]); //connect to correct data
    }
    serializeJson(parsedJsonDoc, sendMessage);
    return return_parse_data_into_json_boilerTempData;
  }
  break;
  }
  return return_parse_data_into_json_error;
}

boolean makePostRequest2(uint8_t sendMode)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    //Serial.printf("[HTTPS] Connected to Wifi\n");
    WiFiClientSecure *client = new WiFiClientSecure;
    if (client)
    {
      //Serial.printf("[HTTPS] Made client succesfully\n");
      //client->setCACert(rootCACertificate);

      {
        HTTPClient https;
        //Serial.printf("[HTTPS] Connected to server...\n");
        if (https.begin(*client, serverCredentials1.serverAddress, serverCredentials1.serverPort, serverCredentials1.serverURI[sendMode], true))
        {

          int httpCode = https.POST(sendMessage);

          if (httpCode > 0)
          {
            //Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
              //String payload = https.getString();
              //Serial.println(payload);
              https.end();
              delete client;
              return true;
            }
          }
          else
          {
            Serial.printf("[HTTPS] failed, error: %s\n", https.errorToString(httpCode).c_str());
          }

          https.end();
        }
        else
        {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
      }
      delete client;
    }
    else
    {
      Serial.println("Unable to create client");
      return false;
    }
  }
  return false;
}
