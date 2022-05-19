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
    m_sendEvent(EventId()),
    clientId(0),
    chatroom(100),
    personal(0),
    group(0),
    init(0)

{
    NS_LOG_FUNCTION(this);
}

void ChatServer::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    if(!m_socket){
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(),tid);
        r_socket = Socket::CreateSocket(GetNode(),tid);
        r_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));
        r_socket->Close();
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
    if(personal==1){          //personal chat
        d_to_send.push_back(1);
        d_to_send.push_back(OtherClientNumber);
        personal = 0;
        OtherClientNumber=0;
    }
    if(group!=0){           //group chat
        d_to_send.push_back(2);
        d_to_send.push_back(ClientNumber); 
        d_to_send.push_back(group);
        group = 0;
    }
    if(initid!=0){            //first connect give id
        d_to_send.push_back(0);
        d_to_send.push_back(initid);
        initid=0;
    }
    if(initgr!=0){           //first connect in group chat 
        d_to_send.push_back(3);
        d_to_send.push_back(initgr);
        d_to_send.push_back(ClientNumber);
        initgr = 0;
    }
    if(nowgroup!=0){          //group out
        d_to_send.push_back(4);
        d_to_send.push_back(nowgroup);
        d_to_send.push_back(ClientNumber); // Client id;
        nowgroup = 0;
    }
    shdr.SetData(d_to_send);
    Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size() * 4); 
    packet->AddHeader(shdr);
    m_txTrace(packet);
    m_socket->Send(packet);
    std::cout<<d_to_send[0]<<" Send "<<d_to_send.size()<< "\n";
    ScheduleTx(Seconds(1.0));                             //need to add group send 
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
            uint32_t m = _data[0];
            std::vector<uint32_t> tmp;
            if(m==0){
                ClientNumber = cliendId.size()+1;   //give id
                tmp.push_back(ClientNumber);
                tmp.push_back(m_address);
                clienId.push_back(tmp);
                initid = 1;
            }
            else if(m==1){
                personal = 1;
         //       SentMsg = data[1];
                OtherClientNumber = _data[1];
                ClientNumber = _data[2];   
            }
            else if(m==2){
                group = _data[1];
                SentRoom = _data[1];
                ClientNumber = _data[2];
            }
            else if(m==3){
                uint32_t tm = _data[1];
                ClientNumber = _data[2];
                chatroom[tm].push_back(ClientNumber); //client id
                initgr = tm;
            }
            else{
                uint32_t tm = _data[1];   
                ClientNumber = _data[2];
                nowgroup = tm;  
                chatroom.erase(remove(chatroom.begin(), chatroom.end(), tm), chatroom.end());
            }
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