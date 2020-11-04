#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h> // Eventueel met WiFi.h http request implementen, schilt een hoop flash geheugen
#include <utils.h>

#define MAXTELEGRAMLENGTH 1500 // Length of chars to read for decoding

// Backoffice URL endpoint
const char* url = "http://192.168.178.101:8000/slimmemeter";

//Pin setup
HardwareSerial P1Poort(1); // Use UART1
const int dataReceivePin = 17; // Data receive pin
const int dataReqPin = 26; // Request pin on 26

char telegram[MAXTELEGRAMLENGTH]; // Variable for telegram data

// Slimme meter data:
byte dsmrVersion;       // DSMR versie zonder punt
long elecUsedT1;   // Elektriciteit verbruikt tarief 1 in Watt
long elecUsedT2;   // Elektriciteit verbruikt tarief 2 in Watt
long elecDeliveredT1;    // Elektriciteit geleverd tarief 1 in Watt
long elecDeliveredT2;    // Elektriciteit geleverd tarief 1 in Watt
byte currentTarrif;   // Huidig tafief
long elecCurrentUsage; // Huidig elektriciteitsverbruik In Watt
long elecCurrentDeliver;    // Huidig elektriciteit levering in Watt
long gasUsage;       // Gasverbruik in dm3
char timeGasMeasurement[12]; // Tijdstip waarop gas voor het laats is gemeten YY:MM:DD:HH:MM:SS
char timeRead[12]; // Tijdstip waarop meter voor het laats is uitgelezen YY:MM:DD:HH:MM:SS

// Debug values:
bool printAllData = true;
bool printDecodedData = true;

// CRC
unsigned int currentCRC = 0;

// Function declaration
void getData(int);
void printData(int, int);
int FindCharInArrayRev(char[], char, int, int);
void printValue(int, int);
bool checkValues(int, bool);
void decodeTelegram(int, int);
bool procesData(int);
bool crcCheck(int, int);
unsigned int CRC16(unsigned int, unsigned char*, int);
String convertToString(char[]); 
void makePostRequest();

void setup() {
  // Start terminal communication
  Serial.begin(115200);
  Serial.println("Starting...");

  //WiFi.mode(WIFI_STA);

  // Configure P1Poort serial connection
  pinMode(dataReqPin, OUTPUT);
  digitalWrite(dataReqPin, LOW);
  P1Poort.begin(115200, SERIAL_8N1, dataReceivePin, -1);  // Start HardwareSerial. RX, TX

  
  // Connecting to Wi-Fi
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to WiFi");
  
  

}

void loop() {
  Serial.println();
  memset(telegram, 0, sizeof(telegram)); // Empty telegram
  int maxRead = 0;
  getData(maxRead);

  makePostRequest();

  delay(10000);
}

// Reads data and calls function procesData(int) to proces the data
void getData(int maxRead){
  while(P1Poort.available()){ // Buffer leegmaken
    yield();
    P1Poort.read();
  }
  digitalWrite(dataReqPin, HIGH); // Request data

  int counter = 0;
  int timeOutCounter = 0;
  bool startFound = false;
  bool endFound = false;
  bool crcFound = false;

  while(!crcFound && timeOutCounter<15000){ // Reads data untill one telegram is fully readed and copied, after not receiving data for 15 seconds the loop exits 
    yield();
    if(P1Poort.available()){
      char c = P1Poort.read();
      char inChar = (char)c;

      if(!startFound){ // If start is not found yes, wait for start symbol
        if(inChar == '/'){
          startFound = true;
          telegram[counter] = inChar;
          counter++;
        }
      } else if(!endFound){ // If start is found, copy incoming char into telegram until end symbol is found
          telegram[counter] = inChar;
          counter++;
          if(inChar == '!'){
            endFound = true;
          }
      } else{ // If end symbol is found, copy incoming char into telegram until \n is found
          telegram[counter] = inChar;
          counter++;
          if(inChar == '\n'){
            crcFound = true;
          }
      }

    } else{ // Wait 1 ms if no data available
      timeOutCounter++;
      delay(1);
    }
  }
  maxRead++;
  digitalWrite(dataReqPin, LOW); // Stop requesting data
  procesData(maxRead);
}

// Checks if data a fully telegram is readed including CRC. If not, data is reread by calling function getData(int) for a maximum of 10 times per loop.
// Also calls function decodeTelegram to attach the readed parameters to variables
bool procesData(int maxRead){
  int startChar = FindCharInArrayRev(telegram, '/', sizeof(telegram), 0); // Plaats van eerste char uit één diagram
  int endChar = FindCharInArrayRev(telegram, '!', sizeof(telegram), startChar); // Plaats van laatste char uit één diagram

  if(startChar >= 0 && endChar >= 0 && (endChar+5)<MAXTELEGRAMLENGTH){ 

    if(printAllData) // For debugging!
    printData(startChar, endChar);

    if(printDecodedData){ // For debugging!
      decodeTelegram(startChar, endChar);
      if(crcCheck(startChar, endChar)){
        return true; // Return true if CRC check is valid
      } else {
        if(maxRead<10){
          Serial.println("CRC check not valid, reading data again");
          getData(maxRead);
        } else{
          Serial.println("CRC check not valid, maxRead has reached it's limit");
          return false; // Return false if CRC check is invalid
        }
      }
    }

  } else{
    if(maxRead<10){
    Serial.println("Not all data found, reading data again");
    //Data was not read correctly, try reading again
    getData(maxRead);
    } else{
      Serial.println("Not all data found, maxRead has reached it's limit");
      return false;
    }
  }
  return false;
}

// Seperates each line and calls the function checkValues(int, bool) to attach the readed parameters to a variable
void decodeTelegram(int startChar, int endChar){
  int placeOfNewLine = startChar;
  bool foundStart = false;
  while(placeOfNewLine < endChar && placeOfNewLine >= 0){
   yield();

   if(!foundStart){
   foundStart = checkValues(placeOfNewLine, foundStart);
   } else {
     checkValues(placeOfNewLine, foundStart);
   }

   placeOfNewLine = FindCharInArrayRev(telegram, '\n', endChar-startChar, placeOfNewLine)+1;
  } 
}

// Checks to what variable the readed parameter belongs and assigns the parameter to the variable by calling the function updateValue(int, int)
// Also does CRC for each line
bool checkValues(int placeOfNewLine, bool foundStart){
  int endChar = FindCharInArrayRev(telegram, '\n', placeOfNewLine+300, placeOfNewLine);
  if((strncmp(telegram+placeOfNewLine, "/", strlen("/")) == 0) && !foundStart){
    // start found. Reset CRC calculation
    currentCRC=CRC16(0x0000,(unsigned char *) telegram+placeOfNewLine, endChar+1-placeOfNewLine);
    return true;
  } else {
    currentCRC=CRC16(currentCRC, (unsigned char*) telegram+placeOfNewLine, endChar+1-placeOfNewLine);
  }

  long round;
  long decimal;
  if(strncmp(telegram+placeOfNewLine, "1-3:0.2.8", strlen("1-3:0.2.8")) == 0){ // DSMR-versie
    Serial.print("DSMR-versie: ");
    sscanf(telegram+placeOfNewLine, "1-3:0.2.8(%ld", &round);
    dsmrVersion = round;
    Serial.println(dsmrVersion);
  } else if(strncmp(telegram+placeOfNewLine, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0){ // Elektriciteit verbruikt T1
    Serial.print("Elektriciteit verbruikt T1: ");
    sscanf(telegram+placeOfNewLine, "1-0:1.8.1(%ld.%ld", &round, &decimal);
    elecUsedT1 = round*1000+decimal;
    Serial.println(elecUsedT1);
  } else if(strncmp(telegram+placeOfNewLine, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0){ // Elektriciteit verbruikt T2
    Serial.print("Elektriciteit verbruikt T2: ");
    sscanf(telegram+placeOfNewLine, "1-0:1.8.2(%ld.%ld", &round, &decimal);
    elecUsedT2 = round*1000+decimal;
    Serial.println(elecUsedT2);
  } else if(strncmp(telegram+placeOfNewLine, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0){ // Elektriciteit geleverd T1
    Serial.print("Elektriciteit geleverd T1: ");
    sscanf(telegram+placeOfNewLine, "1-0:2.8.1(%ld.%ld", &round, &decimal);
    elecDeliveredT1 = round*1000+decimal;
    Serial.println(elecDeliveredT1);
  } else if(strncmp(telegram+placeOfNewLine, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0){ // Elektriciteit geleverd T2
    Serial.print("Elektriciteit geleverd T2: ");
    sscanf(telegram+placeOfNewLine, "1-0:2.8.2(%ld.%ld", &round, &decimal);
    elecDeliveredT2 = round*1000+decimal;
    Serial.println(elecDeliveredT2);
  } else if(strncmp(telegram+placeOfNewLine, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0){ // Huidig tarief
    Serial.print("Hudig tarief: ");
    sscanf(telegram+placeOfNewLine, "0-0:96.14.0(%ld", &round);
    currentTarrif = round;
    Serial.println(currentTarrif);
  } else if(strncmp(telegram+placeOfNewLine, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0){ // Actueel energie verbruik
    Serial.print("Actueel elektriciteits verbruik: ");
    sscanf(telegram+placeOfNewLine, "1-0:1.7.0(%ld.%ld", &round, &decimal);
    elecCurrentUsage = round*1000+decimal; 
    Serial.println(elecCurrentUsage);
  } else if(strncmp(telegram+placeOfNewLine, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0){ // Actueel energie levering
    Serial.print("Actueel elektriciteit teruglevering: ");
    sscanf(telegram+placeOfNewLine, "1-0:2.7.0(%ld.%ld", &round, &decimal);
    elecCurrentDeliver = round*1000+decimal;
    Serial.println(elecCurrentDeliver);
  } else if(strncmp(telegram+placeOfNewLine, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0){ // Gasverbruik + tijdstip gemeten
    Serial.print("Gasverbruik: ");
    sscanf(telegram+placeOfNewLine+strlen("0-1:24.2.1")+15, "(%ld.%ld", &round, &decimal);
    gasUsage = round*1000+decimal; 
    Serial.println(gasUsage);
    Serial.print("Tijdstip gas gemeten: ");
    int j = 0;
    int countUntil = placeOfNewLine+strlen("0-1:24.2.1")+sizeof(timeGasMeasurement)+1;
    for(int i=placeOfNewLine+strlen("0-1:24.2.1")+1; i<countUntil; i++){
      timeGasMeasurement[j] = telegram[i];
      Serial.print(timeGasMeasurement[j]);
      j++;
    }
    Serial.print("\n");
  } else if(strncmp(telegram+placeOfNewLine, "0-0:1.0.0", strlen("0-0:1.0.0")) == 0){ // Tijd data uitgelezen
    Serial.print("Tijd meter uitgelezen: ");
    int j = 0;
    int countUntil = placeOfNewLine+strlen("0-0:1.0.0")+sizeof(timeRead)+1;
    for(int i=placeOfNewLine+strlen("0-0:1.0.0")+1; i<countUntil; i++){
      timeRead[j] = telegram[i];
      Serial.print(timeRead[j]);
      j++;
    }
    Serial.print("\n");
  } 

  return false;
}

// Assign readed parameters to it's corresponding variable TEMP FUNCTION
void printValue(int placeOfNewLine, int valuePlace){
  for(int i=placeOfNewLine+valuePlace; i<placeOfNewLine+200; i++){
      Serial.print(telegram[i]);
      if(telegram[i] == '\n'){
        break;
      }
    }
}

// Returns the place of a certain char in a char array
int FindCharInArrayRev(char array[], char c, int len, int startCountingFrom) {
  for (int i=startCountingFrom; i<len; i++) {
    if (array[i] == c) {
      return i;
    }
  }
  return -1;
}

// Converts Char to String and returns it TEMP FUNCTION
String convertToString(char c[11]) { 
    String s = ""; 
    for (int i=0; i<12; i++) { 
        s += c[i]; 
    } 
    return s; 
} 

// Prints all the data of one telegram TEMP FUNCTION
void printData(int startChar, int endChar){
  Serial.println("Data is:");
  for(int i=startChar; i<endChar+5; i++){
    Serial.print(telegram[i]);
  } 
  Serial.println("");
}

// Performs the final CRC
bool crcCheck(int startChar, int endChar){
  currentCRC=CRC16(currentCRC,(unsigned char*)telegram+endChar, 1);
  char messageCRC[5];
  strncpy(messageCRC, telegram+endChar+1, 4);
  messageCRC[4] = 0;
  bool validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC); 
  currentCRC = 0;

  if(validCRCFound){
    Serial.println("\nVALID CRC FOUND!"); 
    return true;
  } else {
    Serial.println("\n===INVALID CRC FOUND!===");
    return false;
  }
  return false;
}

//Returns CRC
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len) {
	for (int pos = 0; pos < len; pos++)
	{
		crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}

	return crc;
}

void makePostRequest() {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient httpClient;

    String postData = String("{\"dsmrVersion\": ") +dsmrVersion +
    ", \"elecUsedT1\": " +elecUsedT1 +
    ", \"elecUsedT2\": " +elecUsedT2 +
    ", \"elecDeliveredT1\": " +elecDeliveredT1 +
    ", \"elecDeliveredT2\": " +elecDeliveredT2 +
    ", \"currentTarrif\": " +currentTarrif +
    ", \"elecCurrentUsage\": " +elecCurrentUsage +
    ", \"elecCurrentDeliver\": " +elecCurrentDeliver +
    ", \"gasUsage\": " +gasUsage + 
    ", \"timeGasMeasurement\": " +convertToString(timeGasMeasurement) +
    ", \"timeRead\": " +convertToString(timeRead) +
    "}";
    Serial.println(postData);

    httpClient.begin(url);
    httpClient.addHeader("Content-Type", "application/json");
    
    int httpCode = httpClient.POST(postData);

    if(httpCode > 0){
      String payload = httpClient.getString();
      Serial.println("\nStatuscode: " + String(httpCode));
      Serial.println(payload);
      httpClient.end();
    } else {
        Serial.println("Error has occurred on HTTP request");
      }
  } else {
      Serial.println("Connection lost");
    }
}

