# u-espacios

Arduino Project.

u-espacios is a communication project with the goal of providing real-time occupation data of spaces inside a building. As a vertical prototype, we tested the core communication system between the acquired sensor data and the Android application responsible to show this data.

Here we host the code of an esp8266 WiFi module responsible for uploading the received occupation data to the internet. The idea is to synchronize sensor data from several sources (several esp8266) with a common online database which can be accessed later by an user application (e.g. and Android application). The database chosen is Firebase (free prototyping!). Time stamps are added before uploading, thanks to an initial time setting of the module with an NTP server.

Visit:
https://github.com/firebase/firebase-arduino/tree/master/examples/FirebaseDemo_ESP8266
for installation instructions of Firebase-Arduino.

Visit:
http://firebase-arduino.readthedocs.io/en/latest/
for Firebase-Arduino docs.
