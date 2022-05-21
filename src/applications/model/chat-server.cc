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
#include "ns3/uinteger.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ChatServerApplication");

NS_OBJECT_ENSURE_REGISTERED (ChatServer);

//  need modified
TypeId ChatServer::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::ChatServer")
        .SetParent<Application>()
        .AddConstructor<ChatServer>()
        .AddAttribute("Address", "Server Address", AddressValue(),MakeAddressAccessor(&ChatServer::m_address), MakeAddressChecker())
        .AddAttribute("Port", "Server Port", UintegerValue(0), MakeUintegerAccessor(&ChatServer::m_port), MakeUintegerChecker<uint16_t> ())
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
    r_socket({}),
    m_sendEvent(EventId()), 
    mod(0),
    clientId({}),
    chatroom(100)

{
    NS_LOG_FUNCTION(this);
}

void ChatServer::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    if(!m_socket){
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(),tid);
   //     r_socket.push_back(Socket::CreateSocket(GetNode(),tid));
        uint32_t i = 0;
        while(i<100){
            r_socket.push_back(Socket::CreateSocket(GetNode(),tid));
            i++;
        }
        i=0;
        while(i<100){
            r_socket[i]->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port+i));
            r_socket[i]->Close();
            i++;
        }
   //     r_socket[1]->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port+1));
   //     r_socket[1]->Close();
        if(m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_address), m_port)) == -1)
            std::cout << "failed connected\n";
    }
    m_running = true;
    i=0;
    while(i<100){
        if(r_socket[i]->Listen() == -1)
            std::cout<< i <<"Failed\n";
        i++;
    }
 /*   if(r_socket[0]->Listen() == -1)
        std::cout<<"Failed\n";
     if(r_socket[1]->Listen() == -1)
        std::cout<<"Failed\n";*/
    i=0;
    while(i<100){
        r_socket[i]->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&TestServer::onAccept, this));
    }
 //   r_socket[0]->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&TestServer::onAccept, this));
//    r_socket[1]->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&TestServer::onAccept, this));
    ScheduleTx (Seconds(1.0));
/*
    NS_LOG_FUNCTION(this);
    if(!m_socket){
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
*/
}
void ChatServer::onAccept(Ptr<Socket> s, const Address& from){
    std::cout <<m_socket<< "AAA\n";
    s->SetRecvCallback(MakeCallback(&ChatServer::HandleRead, this));
}

void ChatServer::ScheduleTx(Time dt){
    m_sendEvent = Simulator::Schedule(dt, &ChatServer::SendPacket, this);
}

void ChatServer::SendPacket(void){
    NS_LOG_FUNCTION(this);
   // Ptr<Packet> packet = Create<Packet> (m_packetSize);
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
    shdr.SetData(d_to_send);
    Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size() * 4); 
    packet->AddHeader(shdr);
    m_txTrace(packet);
    m_socket->Send(packet);
    std::cout<< r_socket.size()<<"test server"<<d_to_send[0]<<" Send "<<d_to_send.size()<< "\n";
    ScheduleTx(Seconds(1.0));
    /*
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
            m_socket = r_socket.at(OtherClientNumber);
            m_socket->Send(packet);
            std::cout<<d_to_send[0]<<" Send Personal "<<d_to_send.size()<< "\n";
            ScheduleTx(Seconds(1.0));  
        }
        else {
            for(uint32_t i = chatroom[SentRoom].begin();i!=chatroom[SentRoom].end();i++){
                m_socket = r_socket.at(i);
                m_socket->Send(packet);
                std::cout<<d_to_send[0]<<" Send Group "<<d_to_send.size()<< "\n";
                ScheduleTx(Seconds(1.0));  
            }
        }                 
    */
 //   ScheduleTx(Seconds(1.0));
}

void ChatServer::HandleRead(Ptr<Socket> socket){
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
            mod = _data[0];
            if(mod%4==0){
                ClientNumber = clientId.size()+1;   //give id
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
            std::cout<<"Port:"<<m_port<<" Mode: "<<_data[0] << "recieved\n";
        }
    }

}

void ChatServer::StopApplication (void){
    NS_LOG_FUNCTION(this);
    m_running = false;
    if(m_sendEvent.IsRunning()){
        Simulator::Cancel(m_sendEvent);
    }
    if(m_socket){
        m_socket->Close();
        for(uint16_t i = 0; i < r_socket.size(); ++i)
            r_socket[i]->Close();
    }
}