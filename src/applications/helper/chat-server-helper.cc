#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/chat-server-helper.h"
#include "ns3/names.h"
#include "ns3/uinteger.h"
using namespace ns3;

ChatServerHelper::ChatServerHelper (uint16_t port, uint32_t max_client){
    m_factory.SetTypeId("ns3::ChatServer");
    m_factory.Set("SocketNumber", UintegerValue(max_client));
    m_factory.Set("Port", UintegerValue (port));
}

void ChatServerHelper::SetAttribute(std::string name, const AttributeValue & value){
    m_factory.Set(name, value);
}

ApplicationContainer ChatServerHelper::Install (Ptr<Node> node) const{
    return ApplicationContainer (InstallPriv(node));
}

ApplicationContainer ChatServerHelper::Install (std::string nodeName) const{
    Ptr<Node> node = Names::Find<Node> (nodeName);
    return ApplicationContainer (InstallPriv(node));
}

ApplicationContainer ChatServerHelper::Install (NodeContainer c) const{
    ApplicationContainer apps;
    for(NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
        apps.Add (InstallPriv(*i));
    return apps;
}

Ptr<Application>
ChatServerHelper::InstallPriv (Ptr <Node> node) const
{
    Ptr <Application> app = m_factory.Create<Application> ();
    node->AddApplication(app);

    return app;
}

