#include "NTP.h"

NTP::NTP()
{
    _callback = 0;
    _resolved = false;
    _forced = false;
}

NTP::~NTP()
{
}

void NTP::Initialize(const char *server, long period)
{
    _server = server;
    _period = period; // period = -1 means no cyclical requests
}

void NTP::forceOnce()
{
    if (!_requested) _forced = true;
}

void NTP::setPeriod(long period)
{
    _period = period; // period = -1 means no cyclical requests
}

void NTP::attachInterrupt(NTPServerResponseCallbackType callback)
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
    if (((_period != -1) && (abs(millis() - _last_call) > _period)) || // if cyclic check is allowed and the last request is older than period
        (_forced && !_requested)) {                                    // or if forced for one shot, but not started yet
        if (_Resolve()) {
            _UDP = new EthernetUDP;
            _UDP->begin(NTP_CLIENT_PORT);
            _Request();
            _requested = true;
        }
        _last_call = millis();
        _forced = false;
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
            unsigned long epoch = secsSince1900 - seventyYears;
            if (_callback) _callback(epoch);
        } else {
            // No answer within timeout, maybe server unavailable? Return 0
            if (_callback) _callback(0);
        }
        _requested = false;
        _UDP->stop();
        delete _UDP;
    }
}

bool NTP::_Resolve()
{
    bool _successful = true;
    IPAddress _DNSIP;
    _DNS = new DNSClient;
    _DNS->begin(Ethernet.dnsServerIP());
    int _DNSResult = _DNS->getHostByName(_server, _DNSIP);
    if (_DNSResult == DNS_SUCCESS) {
#ifdef NTP_DEBUG
        Serial.print(F("NTPClient: DNS resolving successful: ")); Serial.println(_DNSIP);
#endif
        _IP = _DNSIP;
        _resolved = true;
    } else {
#ifdef NTP_DEBUG
        Serial.print(F("NTPClient: DNS resolving not successful, error code: "));
        switch (_DNSResult) {
            case DNS_TIMED_OUT:
                Serial.print(F("TIMED_OUT, ")); break;
            case DNS_INVALID_SERVER:
                Serial.print(F("INVALID_SERVER, ")); break;
            case DNS_TRUNCATED:
                Serial.print(F("TRUNCATED, ")); break;
            case DNS_INVALID_RESPONSE:
                Serial.print(F("INVALID_RESPONSE, ")); break;
            default:
                Serial.print(F("UNKNOWN, "));
        }
#endif
        if (_resolved) {
#ifdef NTP_DEBUG
            Serial.print(F("using last known IP address: ")); Serial.println(_IP);
#endif
        } else {
            // Can happen only directly after restart if no resolving was made earlier
#ifdef NTP_DEBUG
            Serial.println("waiting for the next time, no NTP requset will be sent");
#endif
            _successful = false;
        }
    }
    delete _DNS;
    _DNS = 0;
    return _successful;
}

void NTP::_Request()
{
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
