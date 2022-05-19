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
#include <algorithm>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ChatServerApplication");

NS_OBJECT_ENSURE_REGISTERED (ChatServer);

  need modified
TypeId ChatServer::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::ChatServer")
        .SetParent<Application>()
        .AddConstructor<ChatClient>()
        .AddAttribute("Address", "Client Address", AddressValue(),MakeAddressAccessor(&ChatServer::m_address), MakeAddressChecker())
        .AddAttribute("Port", "Client Port", UintegerValue(0), MakeUintegerAccessor(&ChatServer::m_port), MakeUintegerChecker<uint16_t> ())
        .AddTraceSource("Tx", "Packet send", MakeTraceSourceAccessor(&ChatServer::m_txTrace), "ns3::Packet::TracedCallback")
         .AddTraceSource("Rx", "Packet send", MakeTraceSourceAccessor(&ChatServer::m_rxTrace), "ns3::Packet::TracedCallback")
;
    return tid;
}

ChatServer::ChatServer()
    :ClientNumber(0),
    m_packetSize(512),
    m_running(false),
    m_packetsSent(0), 
    m_socket(0),
    r_socket(0),
    t_socket(0),
    m_sendEvent(EventId()),
    mod(0),
    clientId(0),
    chatroom(100)

{
    NS_LOG_FUNCTION(this);
}

void ChatServer::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    if(!t_socket.size()){
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(),tid);
        r_socket = Socket::CreateSocket(GetNode(),tid);
        r_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));
        if (m_socket->Bind() == -1)
            NS_FATAL_ERROR("Failed to bind");
        m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_address), m_port));
    }
    m_running = true;
    if(r_socket->Listen() == -1)
        std::cout<<"Failed\n";
    r_socket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&ChatServer::onAccept, this));
    ScheduleTx (Seconds(1.0));
    
}
void ChatServer::onAccept(Ptr<Socket> s, const Address& from){
    s->SetRecvCallback(MakeCallback(&ChatServer::HandleRead, this));
}

void ChatServer::ScheduleTx(Time dt){
    m_sendEvent = Simulator::Schedule(dt, &ChatServer::SendPacket, this);
}

void ChatServer::SendPacket(void){
    NS_LOG_FUNCTION(this);
    Ptr<Packet> packet = Create<Packet> (m_packetSize);
    ChatHeader shdr;
    std::vector<uint32_t> d_to_send;

    if(mod%4==0){            //first connect give id
        d_to_send.push_back(0);
        d_to_send.push_back(ClientNumber);
    }
    else if(mod%4==1){
        d_to_send.push_back(1);
        d_to_send.push_back(ClientNumber); //ip address need to change others
    }
    else if (mod%4==2)
    {
        d_to_send.push_back(2);
        d_to_send.push_back(ClientNumber);
        d_to_send.push_back(SentRoom);
    }
    else{
        d_to_send.push_back(3);
        d_to_send.push_back(SentRoom);
    }
    if (d_to_send.size()) {
        shdr.SetData(d_to_send);
        Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size() * 4); 
        packet->AddHeader(shdr);
        m_txTrace(packet);
        if(mod%4==0){
            m_socket->Send(packet);
            std::cout<<d_to_send[0]<<" Send First Connect "<<d_to_send.size()<< "\n";
            ScheduleTx(Seconds(1.0));  
        }
        else if(mod%4==1){
            m_socket = t_socket.at(OtherClientNumber);
            m_socket->Send(packet);
            std::cout<<d_to_send[0]<<" Send Personal "<<d_to_send.size()<< "\n";
            ScheduleTx(Seconds(1.0));  
        }
        else {
            for(uint32_t i = chatroom[SentRoom].begin();i!=chatroom[SentRoom].end();i++){
                m_socket = t_socket.at(i);
                m_socket->Send(packet);
                std::cout<<d_to_send[0]<<" Send Group "<<d_to_send.size()<< "\n";
                ScheduleTx(Seconds(1.0));  
            }
        }                 
    }
}

void ChatServer::HandleRead(Ptr<Socket> socket){
    Ptr<Packet> packet;
    Address from;
    while ((packet = m_socket->Recv()))
    {
        if(packet->GetSize() > 0)
        {
            m_rxTrace(packet);
            ChatHeader hdr;
            packet->RemoveHeader(hdr);
            std::vector<uint32_t> _data;
            _data = hdr.GetData();
            mod = _data[0];
            if(mod%4==0){
                ClientNumber = cliendId.size()+1;   //give id
                clientId.push_back(std::make_pair(m_address, m_port));
            }
            else if(mod%4==1){
                OtherClientNumber = _data[1];
                ClientNumber = _data[2];   
            }
            else if(mod%4==2){
                SentRoom = _data[1];
                ClientNumber = _data[2];
            }
            else{
                SentRoom = chatroom.size()+1; // make chatroom
                ClientNumber = _data[2];
                for(uint32_t i = _data.begin()+1; i !=_data.end();i++){
                    chatroom[SentRoom].push_back(i);
                }
                 
            }
            t_socket.insert(ClientNumber,m_socket);
        }
    }
    std::cout<< "recieved\n";
}

void ChatServer::StopApplication (void){
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