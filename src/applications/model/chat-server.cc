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
#include <iterator>
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
    m_totalRx = 0;
}
 uint64_t ChatServer::GetTotalRx () const
{
   NS_LOG_FUNCTION (this);
   return m_totalRx;
 }

void ChatServer::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    for (uint32_t i= 0; i < n_socket; i++){
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        t_socket.push_back(Socket::CreateSocket(GetNode(),tid));
        if(t_socket.back()->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port + i)) == -1)
		NS_LOG_WARN("Bind Error for " << m_port + i);
        t_socket.back()->Close();
        if(t_socket.back()->Listen() == -1)
		NS_LOG_WARN("Listen Failed");
        t_socket.back()->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&ChatServer::onAccept, this));
    }    
}

void ChatServer::onAccept(Ptr<Socket> s, const Address& from){
    s->SetRecvCallback(MakeCallback(&ChatServer::HandleRead, this));
}

void ChatServer::SendPacket(std::vector<uint32_t> d_to_send){
    ChatHeader shdr;
    shdr.SetData(d_to_send);
    Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size()*4 - 4);
    packet->AddHeader(shdr);

    for (uint32_t i= 1 ; i <= ClientNumber; ++i){
        if(ClientSocketmap[i])
            ClientSocketmap[i]->Send(packet->Copy());
    }
}
void ChatServer::SendPacket(bool is_room, uint32_t dest, std::vector<uint32_t> d_to_send){
    ChatHeader shdr;
    shdr.SetData(d_to_send);
    Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size()*4 - 4);
    packet->AddHeader(shdr);
    if (is_room){
        for(uint32_t i=0; i < chatroom[dest].size(); i++){
            if (ClientSocketmap[chatroom[dest][i]])
                ClientSocketmap[chatroom[dest][i]]->Send(packet->Copy());
        }
   }
    else{
        if(!ClientSocketmap[dest]){
            packet->RemoveHeader(shdr);
            int tmp = dest;
            dest = d_to_send.back();
            d_to_send[0]=4;
            d_to_send[1]=tmp;
            shdr.SetData(d_to_send);
            packet->AddHeader(shdr);
        }
        ClientSocketmap[dest]->Send(packet);
   }
}   

std::string get_members(std::vector<uint32_t> room){
    std::stringstream ss;
    copy( room.begin(), room.end(), std::ostream_iterator<uint32_t>(ss, " "));
    std::string s = ss.str();
    s = s.substr(0, s.length()-1);
    return s;
}

void ChatServer::HandleRead(Ptr<Socket> socket){
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->Recv()))
    {
        if(packet->GetSize() > 0)
        {
            m_totalRx += packet->GetSize();
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
                SendPacket(d);
            }
            else if(mod==1){
                //data[1] : Destination
                d.push_back(_data[2]);
                 NS_LOG_INFO("1:1msg\t"  << Simulator::Now().GetSeconds() << "\t" << "Client "<< _data[2] << " send to Client" << _data[1]);
                SendPacket(false, _data[1], d);
            }
            else if(mod==2){
                d.push_back(_data[2]);
                d.push_back(_data[1]);
                NS_LOG_INFO("room" << _data[1] << "\t" << Simulator::Now().GetSeconds()<< "\t" << "Client " << _data[2] << " send message");
                SendPacket(true, _data[1], d);
            }
            //making new chatroom in Server
            else if(mod==3){
                std::vector<uint32_t> new_members;
                for(uint32_t i = 1; i < _data.size();i++){
                    new_members.push_back(_data[i]);
                }
                d.push_back(chatroom.size());
                chatroom.push_back(new_members);
                NS_LOG_INFO("room" << d.back() << "\t" << Simulator::Now().GetSeconds() << "\t" << "members: " << get_members(chatroom[d.back()]));
                NS_LOG_INFO("room" << d.back() << "\t" << Simulator::Now().GetSeconds() << "\t" << "Client " << _data.back() << " make new chatting room");
                SendPacket(true, d.back(), d);
            }
            else if(mod==4){
                d.push_back(_data[1]);
                ClientSocketmap[_data[1]]=nullptr;
                for(auto r : chatroom)
                {
                    for(auto &x : r)
                    {
                        if(x==_data[1]) x=0;
                    }
                }


                NS_LOG_INFO(Simulator::Now().GetSeconds()<<"\t"<<"In Server Client" << _data[1] << " is exited");
                //SendPacket(d);
            }
        }
    }
}

void ChatServer::StopApplication(void){
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
