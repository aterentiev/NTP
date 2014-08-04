#include <SPI.h>
#include <Ethernet.h>
#include <NTP.h>

byte mac[] = { 0x12, 0x34, 0x56, 0x00, 0x00, 0x01 };
byte ip[] = { 192, 168, 11, 10 };

NTP ntpServer;

void setup()
{ 
    Serial.begin(9600);
    Ethernet.begin(mac, ip);
    ntpServer.Initialize("pool.ntp.org", 3600000); // Hourly check
    ntpServer.attachInterrupt(ntpServer_OnResponse);
    ntpServer.forceOnce(); // Check the time once after reboot, then hourly
}

// Polling
void loop()
{
    ntpServer.Do();
}

// Callbacks
void ntpServer_OnResponse(unsigned long time)
{
    if (time) {
        Serial.println(F("NTP response received"));
        Serial.print(F("New time: "));
        Serial.println(time);
    } else {
        Serial.println(F("No NTP response received within specified timeout"));
    }
}
