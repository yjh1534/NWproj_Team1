#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

using namespace ns3;


int
main (int argc, char *argv[])
{
    ns3::RngSeedManager::SetSeed(15);
    
    CommandLine cmd;
    cmd.Parse(argc, argv);

    NodeContainer nodes;
    nodes.Create(11);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("5Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("0us"));


    NetDeviceContainer link = csma.Install (nodes);

    InternetStackHelper internet;
    internet.Install (nodes);
    
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer if1 = ipv4.Assign(link);
   

    ApplicationContainer cap; 
    uint16_t port = 9;
    for(uint16_t i = 0; i < 10; i++){
    ChatClientHelper cc (if1.GetAddress(0),port + i);
    cap.Add(cc.Install(nodes.Get(i+1)));
    }

    ChatServerHelper cs (port, 10);

    ApplicationContainer sap = cs.Install(nodes.Get(0));

    sap.Start(Seconds(0));
    cap.Start(Seconds(1));
    sap.Stop(Seconds(5));
    cap.Stop(Seconds(5));

    Simulator::Stop(Seconds (5));
    Simulator::Run();
    Simulator::Destroy();
}
