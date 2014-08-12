#ifndef __ntp_h__
#define __ntp_h__

#include "Arduino.h"
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "Dns.h"

#define NTP_CLIENT_PORT 8888
#define NTP_PACKET_SIZE 48
#define NTP_RESPONSE_TIMEOUT 1000

#define DNS_SUCCESS           1
#define DNS_TIMED_OUT        -1
#define DNS_INVALID_SERVER   -2
#define DNS_TRUNCATED        -3
#define DNS_INVALID_RESPONSE -4

class NTP
{
    
    public:    
        NTP();
        ~NTP();

        void Initialize(const char *, unsigned long);
        void forceOnce();

        typedef void (*NTPServerResponseCallbackType)(unsigned long);

        // Attach and detach callback for NTP response
        void attachInterrupt(NTPServerResponseCallbackType);
        void detachInterrupt();

        void Do();

    private:

        byte _PacketBuffer[NTP_PACKET_SIZE];

        unsigned long _last_call;
        unsigned long _period;
        const char *_server;

        // Callback placeholder
        NTPServerResponseCallbackType _callback;

        void _Request();
        bool _Resolve();
        bool _requested;
        bool _resolved;

        EthernetUDP *_UDP;
        DNSClient *_DNS;
        IPAddress _IP;

};

#endif
