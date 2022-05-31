#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include <iostream>
#include <vector>

#ifndef CHAT_HEADER_H
#define CHAT_HEADER_H

using namespace ns3;

class ChatHeader : public Header
{
public:
    void SetData (std::vector<uint32_t> data);
    std::vector<uint32_t> GetData (void) const;
    
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual uint32_t GetSerializedSize (void) const;
private:
    std::vector<uint32_t> m_data;
};
#endif
