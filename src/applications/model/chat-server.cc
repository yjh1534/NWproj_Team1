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

TypeId ChatServer::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::ChatServer")
        .SetParent<Application>()
        .AddConstructor<ChatServer>()
        .AddAttribute("Port", "Client Port", UintegerValue(0), MakeUintegerAccessor(&ChatServer::m_port), MakeUintegerChecker<uint16_t> ())
        .AddAttribute("SocketNumber","max Socket", UintegerValue(0), MakeUintegerAccessor(&ChatServer::n_socket), MakeUintegerChecker<uint32_t>())
        .AddTraceSource("Tx", "Packet send", MakeTraceSourceAccessor(&ChatServer::m_txTrace), "ns3::Packet::TracedCallback")
         .AddTraceSource("Rx", "Packet send", MakeTraceSourceAccessor(&ChatServer::m_rxTrace), "ns3::Packet::TracedCallback")
;
    return tid;
}

ChatServer::ChatServer()
    :ClientNumber(0),
    m_packetSize(200),
    m_running(false),
    m_packetsSent(0), 
    t_socket({}),
    m_sendEvent(EventId())
{
    NS_LOG_FUNCTION(this);
    ClientSocketmap.clear();
    chatroom.clear();
}

void ChatServer::StartApplication(void)
{
    std::cout<<"Start Server\n";
    NS_LOG_FUNCTION(this);
    for (uint32_t i= 0; i < n_socket; i++){
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        t_socket.push_back(Socket::CreateSocket(GetNode(),tid));
        if(t_socket.back()->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port + i)) == -1)
            std::cout << "Bind Error for "<< m_port + i << "\n";
        t_socket.back()->Close();
        if(t_socket.back()->Listen() == -1)
            std::cout<<"Listen Failed\n";
        t_socket.back()->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&ChatServer::onAccept, this));
    }    
}

void ChatServer::onAccept(Ptr<Socket> s, const Address& from){
    std::cout<<"Accpeted\n";
    s->SetRecvCallback(MakeCallback(&ChatServer::HandleRead, this));
}

void ChatServer::SendPacket(std::vector<uint32_t> d_to_send){
    ChatHeader shdr;
    shdr.SetData(d_to_send);
    Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size()*4 - 4);
    packet->AddHeader(shdr);

    for (uint32_t i= 1 ; i < ClientNumber+1; ++i){
        ClientSocketmap[i]->Send(packet);
    }
    std::cout<<"Server Send New Client "<<ClientNumber<<" to all "<<packet->GetSize()<<" \n";
}
void ChatServer::SendPacket(bool is_room, uint32_t dest, std::vector<uint32_t> d_to_send){
    ChatHeader shdr;
    shdr.SetData(d_to_send);
    Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size()*4 - 4);
    packet->AddHeader(shdr);
    if(is_room){
        for(uint32_t i=0; i < chatroom[dest].size(); i++){
            ClientSocketmap[chatroom[dest][i]]->Send(packet);
        }
        std::cout<<"Message to Room "<<dest<<" from "<<d_to_send.back()<<"\n";
   }
    else{
        ClientSocketmap[dest]->Send(packet);
         std::cout<<"Message to "<<dest<<" from "<<d_to_send.back()<<"\n";
   }
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
            std::vector<uint32_t> _data = {};
            std::vector<uint32_t> d = {};
            _data = hdr.GetData();
            uint32_t mod = _data[0];
            d.push_back(mod);
            if(mod==0){
                ClientNumber++;
                ClientSocketmap[ClientNumber] = socket;
                d.push_back(ClientNumber);
                std::cout<<"Received New Client : "<<ClientNumber<<"\n";
                SendPacket(d);
            }
            else if(mod==1){
                //data[1] : Destination
                d.push_back(_data[2]);
                 std::cout<<"Server: 1to1 from"<< _data[2] << " to "<< _data[1]<<"\n";
                SendPacket(false, _data[1], d);
            }
            else if(mod==2){
                d.push_back(_data[2]);
                d.push_back(_data[1]);
                 std::cout<<"Server: Room from"<< _data[2] << " to Room "<< _data[1]<<"\n";
                SendPacket(true, _data[1], d);
            }
            else{
                std::vector<uint32_t> new_members;
                for(uint32_t i = 1; i < _data.size();i++){
                    new_members.push_back(_data[i]);
                }
                d.push_back(chatroom.size());
                chatroom.push_back(new_members);
                std::cout<<"Server: Invite from"<< _data.back() << " to Room"<< d.back()<<"\n";
                SendPacket(true, d.back(), d);
            }
        }
    }
}

void ChatServer::StopApplication (void){
    NS_LOG_FUNCTION(this);
    m_running = false;
    if(m_sendEvent.IsRunning()){
        Simulator::Cancel(m_sendEvent);
    }
    if(t_socket[0]){
        for (uint32_t i = 0; i < t_socket.size(); ++i)
            t_socket[i]->Close();
    }
}
