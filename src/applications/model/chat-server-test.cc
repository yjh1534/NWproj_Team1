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
#include "ns3/chat-server-test.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestServerApplication");

NS_OBJECT_ENSURE_REGISTERED (TestServer);


TypeId TestServer::GetTypeId (void){
    static TypeId tid = TypeId ("ns3::TestServer")
        .SetParent<Application>()
        .AddConstructor<TestServer>()
        .AddAttribute("Address", "Server Address", AddressValue(),MakeAddressAccessor(&TestServer::m_address), MakeAddressChecker())
        .AddAttribute("Port", "Serve Port", UintegerValue(0), MakeUintegerAccessor(&TestServer::m_port), MakeUintegerChecker<uint16_t> ())
        .AddTraceSource("Tx", "Packet send", MakeTraceSourceAccessor(&TestServer::m_txTrace), "ns3::Packet::TracedCallback")
         .AddTraceSource("Rx", "Packet send", MakeTraceSourceAccessor(&TestServer::m_rxTrace), "ns3::Packet::TracedCallback")
;
    return tid;
}

TestServer::TestServer()
    :ClientNumber(0),
    m_packetSize(512),
    m_running(false),
    m_packetsSent(0), 
    m_socket(0),
    r_socket({}),
    m_sendEvent(EventId())
{
    NS_LOG_FUNCTION(this);
}

void TestServer::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    if(!m_socket){
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(),tid);
        r_socket.push_back(Socket::CreateSocket(GetNode(),tid));
        r_socket.push_back(Socket::CreateSocket(GetNode(),tid));
        r_socket[0]->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));
        r_socket[0]->Close();
        r_socket[1]->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port+1));
        r_socket[1]->Close();
        if(m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_address), m_port)) == -1)
            std::cout << "failed connected\n";
    }
    m_running = true;
    if(r_socket[0]->Listen() == -1)
        std::cout<<"Failed\n";
     if(r_socket[1]->Listen() == -1)
        std::cout<<"Failed\n";
    r_socket[0]->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&TestServer::onAccept, this));
    r_socket[1]->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&TestServer::onAccept, this));
    ScheduleTx (Seconds(1.0));
}
void TestServer::onAccept(Ptr<Socket> s, const Address& from){
    std::cout <<m_socket<< "AAA\n";
    s->SetRecvCallback(MakeCallback(&TestServer::HandleRead, this));
    /*TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
    r_socket.push_back(Socket::CreateSocket(GetNode(), tid));
    if(r_socket.back()->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port + 1)) == -1)
        std::cout<<"Failed to bind in 2nd Socket\n";
    r_socket.back()->Close();
    if(r_socket.back()->Listen() == -1)
        std::cout << "Failed to Listen in 2nd socket\b";
    r_socket.back()->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address&> (), MakeCallback(&TestServer::onAccept, this)); 
    std::cout<<m_port<< " in Second Socket\n";*/
}

void TestServer::ScheduleTx(Time dt){
    m_sendEvent = Simulator::Schedule(dt, &TestServer::SendPacket, this);
}

void TestServer::SendPacket(void){
    NS_LOG_FUNCTION(this);
    //Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size() * 8);
    ChatHeader shdr;
    std::vector<uint32_t> d_to_send;
    if(ClientNumber == 0){
        d_to_send.push_back(0);
    }
    else{
        uint32_t m = (uint32_t)std::rand() % 100;
        if (m < 90){
            //send 1:1 message
            if(ChatRoom.empty() || m < 60){
                d_to_send.push_back(1);
                //d_to_send.push_back(someone's number);
            }
            //send n:n message
            else{
                d_to_send.push_back(2);
                //d_to_send.push_back(#random chat room in ChatRoom);
            }
        }
        //Create new Room
        else{
            d_to_send.push_back(3);
            //random integer K in (0, otherClient.size() )
            //select K members from otherClient
            //push_back all of them.
        }
        d_to_send.push_back(ClientNumber);
        // Attach current client number
    
    }
    shdr.SetData(d_to_send);
    Ptr<Packet> packet = Create<Packet> (m_packetSize - d_to_send.size() * 4); 
    packet->AddHeader(shdr);
    m_txTrace(packet);
    m_socket->Send(packet);
    std::cout<< r_socket.size()<<"test server"<<d_to_send[0]<<" Send "<<d_to_send.size()<< "\n";
    ScheduleTx(Seconds(1.0));
}

void TestServer::HandleRead(Ptr<Socket> socket){
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
            //Get Client number from server
            if(m==0){
                ClientNumber = _data[1];
            }
            //Receive 1:1 message
            else if (m==1){
                SentClient = _data[1]; 
            }
            //Receive n:n message
            else if (m==2){
                SentClient = _data[1];
                SentRoom = _data[2];
            }
            //Receive invitation to chatting room
            else if (m==3){
                ChatRoom.push_back(_data[1]);
            }
            //Receive New Client's info
            else if (m==4){
                otherClients.push_back(_data[0]);
            }
        std::cout<<"Port:"<<m_port<<" Mode: "<<_data[0] << "recieved\n";
        }
    }
}

void TestServer::StopApplication (void){
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

