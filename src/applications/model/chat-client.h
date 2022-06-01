#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/chat-header.h"
#include "ns3/chat-client.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace ns3;


class ChatClient : public Application
{
    public:
        static TypeId GetTypeId (void);
        ChatClient ();
        //virtual ~ChatClient();

    private:
        virtual void StartApplication (void);
        virtual void StopApplication (void);

        void ScheduleTx(Time dt);
        void SendPacket(void);
        void HandleRead(Ptr<Socket> socket);
        void onAccept(Ptr<Socket> s);//, const Address& from);
        Address m_address;
        uint16_t m_port;
        uint32_t ClientNumber;
        std::vector<uint32_t> ChatRoom;
        std::vector<uint32_t> otherClients;
        uint32_t SentClient;
        uint32_t SentRoom;
        uint32_t m_packetSize;
        bool m_running;
        uint32_t m_packetsSent;
        Ptr<Socket> r_socket;
        EventId m_sendEvent;
        Time m_interval;

        TracedCallback<Ptr<const Packet> > m_txTrace;
        TracedCallback<Ptr<const Packet> > m_rxTrace;
};
#endif
