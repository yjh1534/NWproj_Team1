#ifndef CHAT_SERVER_TEST_HELPER_H
#define CHAT_SERVER_TEST_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"

using namespace ns3;

class TestServerHelper{
    public:
        TestServerHelper(Address address, uint16_t port);
        void SetAttribute(std::string name, const AttributeValue &value);
        ApplicationContainer Install (Ptr<Node> node) const;
        ApplicationContainer Install (std::string nodeName) const;
        ApplicationContainer Install (NodeContainer c)const;

    private:
        Ptr<Application> InstallPriv(Ptr<Node> node) const;
        ObjectFactory m_factory;
};
#endif
