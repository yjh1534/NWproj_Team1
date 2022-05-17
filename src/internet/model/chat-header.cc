#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include <iostream>
#include <vector>
#include "chat-header.h"

using namespace ns3;

TypeId ChatHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ChatHeader")
        .SetParent<Header> ()
        .AddConstructor<ChatHeader>()
        ;
    return tid;
}

TypeId ChatHeader::GetInstanceTypeId (void) const
{
    return GetTypeId();
}

void ChatHeader::Print (std::ostream &os) const
{
    for (uint16_t i = 0; i < m_data.size(); ++i){
        os << m_data[i] << "/";
    }
}

uint32_t ChatHeader::GetSerializedSize (void) const
{
    return m_data.size() * 4;
}

void ChatHeader::Serialize (Buffer::Iterator start) const
{
   for(uint16_t i = 0; i < m_data.size(); ++i){
       start.WriteHtonU32(m_data[i]);
   }
}

uint32_t ChatHeader::Deserialize (Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    m_data.clear();
    while (!i.IsEnd()){
        m_data.push_back(i.ReadNtohU32 ());
    }
    return i.GetDistanceFrom(start);
}

void ChatHeader::SetData (std::vector<uint32_t> data)
{
    m_data = data;
}

std::vector<uint32_t> ChatHeader::GetData (void)const
{
    return m_data;
}
