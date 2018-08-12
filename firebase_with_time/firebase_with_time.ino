#include <ArduinoJson.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>

// Set sensor ID
#define SENSOR_ID "sensor1"

// Set WiFi connection
#define WIFI_SSID "CEJARA5"
#define WIFI_PASSWORD "Cejara798844500"

// Set database keys
#define CAPACITY_KEY "capacidad"
#define OCCUPIED_KEY "ocupados"
#define HISTORY_KEY "historico"
#define HISTORY_OCCUPIED_KEY "ocupados"
#define HISTORY_TIME_KEY "timestamp"

// Set Firebase Variables
#define FIREBASE_HOST "u-espacios.firebaseio.com"
#define FIREBASE_AUTH "KxyiMLVleS889XNRBsTvcmna9ZfS79Pp1XKL32Qx"

// Setup of Network Time Protocol (NTP)
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp; // A UDP instance to let us send and receive packets over UDP

// Variables for Database paths
char capacity_path[20];
char occupied_path[20];
char history_path[20];

// Variables for sensor data
int capacity = 0;
int occupied = 0;

// Variable for timestamp
unsigned long secsSince1970;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);

  // Connect to WiFi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected: ");
  Serial.println(WiFi.localIP());

  // Start UDP
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  // Get initial time from NTP
  bool waiting = true;
  int cb;
  while (waiting) {
    Serial.println("Waiting for packet from NTP server.");
    WiFi.hostByName(ntpServerName, timeServerIP); // Get random server from the pool
    sendNTPpacket(timeServerIP);
    delay(1000);
    cb = udp.parsePacket();
    if (cb) {
      waiting = false;
    }
  }
  Serial.print("Packet received, length=");
  Serial.println(cb);
  unsigned long initial_secsSince1970 = setUnixTime();
  Serial.print("Initial Unix Time:");
  Serial.println(initial_secsSince1970);

  // Setup paths
  sprintf(capacity_path, "%s/%s", SENSOR_ID, CAPACITY_KEY);
  sprintf(occupied_path, "%s/%s", SENSOR_ID, OCCUPIED_KEY);
  sprintf(history_path, "%s/%s", SENSOR_ID, HISTORY_KEY);

  // Setup Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  capacity = Firebase.getInt(capacity_path);

  secsSince1970 = now();
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root[HISTORY_OCCUPIED_KEY] = occupied;
  root[HISTORY_TIME_KEY] = secsSince1970;
  
  Firebase.set(history_path, root);
  Firebase.set(occupied_path, occupied);
}

void loop() {
  // Simulate Button
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);

  // Read incoming data
  secsSince1970 = now();
  if (occupied < capacity) {
    ++occupied;
  } else {
    occupied = 0;
  }
  
  Serial.println(occupied);

  // Send nudes
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root[HISTORY_OCCUPIED_KEY] = occupied;
  root[HISTORY_TIME_KEY] = secsSince1970;
  
  Firebase.setInt(occupied_path, occupied);
  Firebase.push(history_path, root);
  
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

unsigned long setUnixTime() {
  // We've received a packet, read the data from it
  udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  //the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, esxtract the two words:
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  //Serial.print("Seconds since Jan 1 1900 = " );
  //Serial.println(secsSince1900);
  // now convert NTP time into everyday time:
  //Serial.print("Unix time = ");
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  unsigned long epoch = secsSince1900 - seventyYears;
  // print Unix time:
  //Serial.println(epoch);
  //return(epoch);
  setTime(epoch);
  return(epoch);
}



