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

#define MIN_MULTI_CHAT_CLI_NUM 3

NS_LOG_COMPONENT_DEFINE ("ChatClientApplication");

NS_OBJECT_ENSURE_REGISTERED (ChatClient);


TypeId ChatClient::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::ChatClient")
        .SetParent<Application>()
        .AddConstructor<ChatClient>()
        .AddAttribute("Address", "Server Address", AddressValue(),MakeAddressAccessor(&ChatClient::m_address), MakeAddressChecker())
        .AddAttribute("Port", "Serve Port", UintegerValue(0), MakeUintegerAccessor(&ChatClient::m_port), MakeUintegerChecker<uint16_t> ())
        .AddTraceSource("Tx", "Packet send", MakeTraceSourceAccessor(&ChatClient::m_txTrace), "ns3::Packet::TracedCallback")
         .AddTraceSource("Rx", "Packet send", MakeTraceSourceAccessor(&ChatClient::m_rxTrace), "ns3::Packet::TracedCallback")
;
    return tid;
}

ChatClient::ChatClient()
    :ClientNumber(0),
    m_packetSize(512),
    m_running(false),
    m_packetsSent(0), 
    m_socket(0),
    r_socket(0),
    m_sendEvent(EventId())
{
    NS_LOG_FUNCTION(this);
}

void ChatClient::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    if(!m_socket){
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(),tid);
        r_socket = Socket::CreateSocket(GetNode(),tid);
        r_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));
        r_socket->Close();
        if(m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_address), m_port)) == -1)
            std::cout << "failed connected\n";
    }
    m_running = true;
    if(r_socket->Listen() == -1)
        std::cout<<"Failed\n";
    r_socket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&ChatClient::onAccept, this));
    ScheduleTx (Seconds(1.0));
}
void ChatClient::onAccept(Ptr<Socket> s, const Address& from){
    std::cout << "AAA\n";
    s->SetRecvCallback(MakeCallback(&ChatClient::HandleRead, this));
}

void ChatClient::ScheduleTx(Time dt){
    m_sendEvent = Simulator::Schedule(dt, &ChatClient::SendPacket, this);
}

void ChatClient::SendPacket(void){
    NS_LOG_FUNCTION(this);
    //Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size() * 8);
    ChatHeader shdr;
    std::vector<uint32_t> d_to_send;
    uint32_t send_prob = rand() % 100;
    // Send packet by 50% probability.
    if (send_prob < 50) {
        if (ClientNumber == 0)
            d_to_send.push_back(0);
        
        else {
            uint32_t m = (uint32_t)std::rand() % 100;
            if (m < 95){
                //send 1:1 message
                if(otherClients.size() && m < 60){
                    d_to_send.push_back(1);
                    uint32_t n = rand() % otherClients.size();
                    d_to_send.push_back(n);
                    d_to_send.push_back(ClientNumber); // Attach current client number
                }
                //send n:n message
                else if (!ChatRoom.empty()) {
                    d_to_send.push_back(2);
                    uint32_t n = rand() % ChatRoom.size();
                    d_to_send.push_back(n);
                    d_to_send.push_back(ClientNumber); // Attach current client number
                    //d_to_send.push_back(#random chat room in ChatRoom);
                }
            }

            //Create new Room when other clients exists more than MIN_MULTI_CHAT_CLI_NUM 
            else if (otherClients.size() > MIN_MULTI_CHAT_CLI_NUM){
                d_to_send.push_back(3);
                // random integer n in (0, otherClient.size() )
                uint32_t n = (rand() % ((otherClients.size() - MIN_MULTI_CHAT_CLI_NUM))) + (MIN_MULTI_CHAT_CLI_NUM); // MIN_MULTI_CHAT_CLI_NUM ~ (otherClients.size() - 1)
                std::random_shuffle(otherClients.begin(), otherClients.end());
                // select n members from otherClient
                for (int i = 0; i < n; i++) d_to_send.push_back(otherClients.at(i));
                // Attach current client number
                d_to_send.push_back(ClientNumber);
            }
        }

        // Send packet when d_to_send is not empty
        if (d_to_send.size()) {
            shdr.SetData(d_to_send);
            Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size() * 4); 
            packet->AddHeader(shdr);
            m_txTrace(packet);
            m_socket->Send(packet);
            std::cout<<d_to_send[0]<<" Send "<<d_to_send.size()<< "\n";
            ScheduleTx(Seconds(1.0));
        }
    }
}

void ChatClient::HandleRead(Ptr<Socket> socket){
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->Recv()))
    {
        if(packet->GetSize() > 0)
        {
            m_rxTrace(packet);
            ChatHeader hdr;
            packet->RemoveHeader(hdr);
            std::vector<uint32_t> _data;
            _data = hdr.GetData();
            uint32_t m = _data[0];

            // Get Client number from server
            if (m == 0){
                uint32_t receivedClientNumber = _data[1];
                // Set current client number and other clients vector if current client is new.
                if (!ClientNumber) {
                    ClientNumber = receivedClientNumber;
                    for (uint32_t i = 1; i < ClientNumber; i++) otherClients.push_back(i);
                }
                // Add new clients in other clients vector
                else 
                    otherClients.push_back(receivedClientNumber);
            }
            //Receive 1:1 message
            else if (m == 1){
                SentClient = _data[1]; 
            }
            //Receive n:n message
            else if (m == 2){
                SentClient = _data[1];
                SentRoom = _data[2];
            }
            //Receive invitation to chatting room
            else if (m == 3){
                ChatRoom.push_back(_data[1]);
            }
        }
    }
    std::cout<<ClientNumber<<"recieved\n";
}

void ChatClient::StopApplication (void){
    NS_LOG_FUNCTION(this);
    m_running = false;
    if(m_sendEvent.IsRunning()){
        Simulator::Cancel(m_sendEvent);
    }
    if(m_socket){
        m_socket->Close();
        r_socket->Close();
    }
}

