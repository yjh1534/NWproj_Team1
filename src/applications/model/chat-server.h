#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H
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
#include "ns3/chat-server.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <map>

using namespace ns3;


class ChatServer : public Application
{
    public:
        static TypeId GetTypeId (void);
        ChatServer ();

    private:
        virtual void StartApplication (void);
        virtual void StopApplication (void);

        void ScheduleTx(Time dt);
        void SendPacket(void);
        void HandleRead(Ptr<Socket> socket);
        void onAccept(Ptr<Socket> s, const Address& from);
        Address m_address;
        uint16_t m_port;
        uint32_t mod;
        uint32_t ClientNumber;
        uint32_t OtherClientNumber;
        std::vector<uint32_t> data;
        std::vector<uint32_t> ChatRoom;
        std::vector<uint32_t> otherClients;
        uint32_t SentClient;
        uint32_t SentRoom;
        uint32_t m_packetSize;
        bool m_running;
        uint32_t m_packetsSent;
        std::vector<Ptr<Socket> > t_socket;
        Ptr<Socket> m_socket;
        Ptr<Socket> r_socket;
        EventId m_sendEvent;
        std::vector<std::pair<Address, uint16_t> > clientId;
        std::vector<std::vector<uint32_t> > chatroom;
        TracedCallback<Ptr<const Packet> > m_txTrace;
        TracedCallback<Ptr<const Packet> > m_rxTrace;
};
#endif