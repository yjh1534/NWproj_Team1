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
    m_packetSize(1000),
    m_running(false),
    m_packetsSent(0), 
    m_socket(0),
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
        if (m_socket->Bind() == -1)
            NS_FATAL_ERROR("Failed to bind");
        m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_address), m_port));
    }
    m_running = true;
    m_socket->SetRecvCallback (MakeCallback (&ChatClient::HandleRead, this));
    if(m_socket->Listen())
        std::cout<<"Listen!!\n";
    ScheduleTx (Seconds(1.0));
}

void ChatClient::ScheduleTx(Time dt){
    m_sendEvent = Simulator::Schedule(dt, &ChatClient::SendPacket, this);
}

void ChatClient::SendPacket(void){
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet = Create<Packet> (m_packetSize);
    ChatHeader shdr;
    std::vector<uint32_t> d_to_send;
    if(ClientNumber == 0){
        d_to_send.push_back(0);
    }
    else{
        if (ChatRoom.empty()){
            d_to_send.push_back(2);
        }
        else{
            uint32_t m = (uint32_t)std::rand() % 100;
            if (m < 90){
                d_to_send.push_back(1);
                }
            else if (ChatRoom.empty()){
                d_to_send.push_back(2);
                }
            }
    }
    shdr.SetData(d_to_send);
    packet->AddHeader(shdr);
    m_txTrace(packet);
    m_socket->Send(packet);
    std::cout<<d_to_send[0]<<" Send "<<m_address<< "\n";
    ScheduleTx(Seconds(1.0));
}

void ChatClient::HandleRead(Ptr<Socket> socket){
    Ptr<Packet> packet;
    Address from;
    std::cout<<data[0]<<"recieved\n";
    while ((packet = m_socket->Recv()))
    {
        if(packet->GetSize() > 0)
        {
            m_rxTrace(packet);
            ChatHeader hdr;
            packet->RemoveHeader(hdr);
            data = hdr.GetData();
            uint32_t m = data[0];
            if(m==0){
                ClientNumber = data[1];
            }
            else if (m==1){
                SentRoom = data[1];
                SentClient = data[2]; 
            }
            else if (m==2){
                ChatRoom.push_back(data[1]);
            }
            else if (m==3){
                otherClients.push_back(data[1]);
            }
            else if (m==4){
                ChatRoom.erase(std::find(ChatRoom.begin(), ChatRoom.end(), data[1])); 
            }
        }
    }
    std::cout<<data[0]<<"recieved\n";
}

void ChatClient::StopApplication (void){
    NS_LOG_FUNCTION(this);
    m_running = false;
    if(m_sendEvent.IsRunning()){
        Simulator::Cancel(m_sendEvent);
    }
    if(m_socket){
        m_socket->Close();
    }
}

