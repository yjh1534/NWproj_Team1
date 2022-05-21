#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/chat-server-test-helper.h"

using namespace ns3;
/*
static void
Rxtime (std::string context, Ptr<const Packet> p, const Address &a)
{
    static double bytes0, bytes1, bytes2=0;
    if (context == "BackGround"){
        bytes0 =p->GetSize();
        NS_LOG_UNCOND("0\t" << Simulator::Now().GetSeconds()*10 << "\t" << bytes0*8);
    }
    else if (context == "Flow1"){
        bytes1 =p->GetSize();
        NS_LOG_UNCOND("1\t" << Simulator::Now().GetSeconds()*10 << "\t" << bytes1*8);
    }
    else if (context == "Flow2"){
        bytes2 =p->GetSize(); 
        NS_LOG_UNCOND("2\t" << Simulator::Now().GetSeconds()*10 << "\t" << bytes2*8); 
    } 
}
*/
int
main (int argc, char *argv[])
{

    Ptr<Node> n1 = CreateObject<Node> ();
    Ptr<Node> n2 = CreateObject<Node> ();
    Ptr<Node> n3 = CreateObject<Node> ();

    NodeContainer nodess = NodeContainer(n1,n2,n3);
    NodeContainer nodes = NodeContainer(n1, n2);
    NodeContainer nodes2 = NodeContainer(n2, n3);
    /*PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    PointToPointHelper p2p2;
    p2p2.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p2.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer devices;
    devices = p2p.Install (nodes);
    NetDeviceContainer devices2;
    devices2 = p2p2.Install (nodes2);
*/
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (nodess);

    InternetStackHelper stack;
    stack.Install(nodess);
    
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ifs = ipv4.Assign(csmaDevices);

    ChatClientHelper cc(ifs.GetAddress(1), 9);
    ApplicationContainer clientApps;
    clientApps.Add(cc.Install(nodess.Get(0)));
    TestServerHelper cc2(ifs.GetAddress(0),9);
    clientApps.Add(cc2.Install(nodess.Get(1)));
    ChatClientHelper cc3(ifs.GetAddress(1),10);
    ApplicationContainer clientApps2;
    clientApps2.Add(cc3.Install(nodess.Get(2)));


    clientApps.Start(Seconds(0.5));
    clientApps.Stop(Seconds(5));
    clientApps2.Start(Seconds(0.5));
    clientApps2.Stop(Seconds(5));
    Simulator::Run();
    Simulator::Stop(Seconds(5));
    
    Simulator::Destroy();

    std::cout <<"test over";
}
