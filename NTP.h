#ifndef __ntp_h__
#define __ntp_h__

#include "Arduino.h"
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "Dns.h"

#define NTP_CLIENT_PORT 8888
#define NTP_PACKET_SIZE 48
#define NTP_RESPONSE_TIMEOUT 1000

    class NTP
    {
    
    public:    
        NTP();
        ~NTP();

        void Initialize(const char *, unsigned long);
        void forceOnce();

        typedef void (*NTPServerCallbackType)(uint32_t);

        // Attach and detach callback for NTP response
        void attachInterrupt(NTPServerCallbackType);
        void detachInterrupt();

        void Do();

    private:

        byte _PacketBuffer[NTP_PACKET_SIZE];

        unsigned long _last_call;

        unsigned long _period;
        const char *_server;

        // Callback placeholder
        NTPServerCallbackType _callback;

        void _Request();
        EthernetUDP *_UDP;
        DNSClient *_DNS;
        IPAddress _IP;

        bool _requested;

    };

#endif
