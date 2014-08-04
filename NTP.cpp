#include "NTP.h"

NTP::NTP()
{
    _UDP = new EthernetUDP;
    _DNS = new DNSClient; 
    _callback = 0;
}

NTP::~NTP()
{
}

void NTP::Initialize(const char *server, unsigned long period)
{
    _server = server;
    _period = period;
    _UDP->begin(NTP_CLIENT_PORT);
    _DNS->begin(Ethernet.dnsServerIP());
}

void NTP::forceOnce()
{
    _last_call = millis() - _period - 1; // Guaranteed call now
}

void NTP::attachInterrupt(NTPServerCallbackType callback)
{
    _callback = callback;
}

void NTP::detachInterrupt()
{
    _callback = 0;
}

void NTP::Do()
{
    // Simple "call-answer after timeout" state machine
    if (abs(millis() - _last_call) > _period) {
        _Request();
        _last_call = millis();
        _requested = true;
    }
    if ((_requested) && (abs(millis() - _last_call) > NTP_RESPONSE_TIMEOUT)) {
        if (_UDP->parsePacket()) {
            // There was an answer
            _UDP->read(_PacketBuffer, NTP_PACKET_SIZE);
            // Simplified NTP frame decoding
            unsigned long highWord = word(_PacketBuffer[40], _PacketBuffer[41]);
            unsigned long lowWord = word(_PacketBuffer[42], _PacketBuffer[43]);  
            unsigned long secsSince1900 = highWord << 16 | lowWord;	 		
            const unsigned long seventyYears = 2208988800UL;	 
            uint32_t epoch = secsSince1900 - seventyYears;
            if (_callback) _callback(epoch);
        } else {
            // No answer within timeout, maybe server unavailable? Return 0
            if (_callback) _callback(0);
        }
        _requested = false;
    }
}

void NTP::_Request()
{
    if (_DNS->getHostByName(_server, _IP) == 1) {
        // clear all bytes in the buffer
        memset(_PacketBuffer, 0, NTP_PACKET_SIZE); 

        // Initialize values needed to form NTP request
        _PacketBuffer[0] = 0b11100011;  // LI, Version, Mode
        _PacketBuffer[1] = 0; // Stratum, or type of clock
        _PacketBuffer[2] = 6; // Polling Interval
        _PacketBuffer[3] = 0xEC; // Peer Clock Precision
        // 8 bytes of zero for Root Delay & Root Dispersion
        _PacketBuffer[12] = 49; 
        _PacketBuffer[13] = 0x4E;
        _PacketBuffer[14] = 49;
        _PacketBuffer[15] = 52;
	
        _UDP->beginPacket(_IP, 123); 
        _UDP->write(_PacketBuffer, NTP_PACKET_SIZE);
        _UDP->endPacket(); 
    }
}
