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
    csma.SetChannelAttribute("DataRate", StringValue("10Mbps"));
    csma.SetChannelAttribute("Delay", StringValue("1us"));


    NetDeviceContainer link = csma.Install (nodes);

    InternetStackHelper internet;
    internet.Install (nodes);
    
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer if1 = ipv4.Assign(link);
   
     ApplicationContainer cap_list[11]; 
    uint16_t port = 9;
    for(uint16_t i = 1; i <= 10; i++){
        ChatClientHelper cc (if1.GetAddress(0),port + i);
        cc.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        cap_list[i].Add(cc.Install(nodes.Get(i)));
        cap_list[i].Start(Seconds(1 + ((double_t) i / 10)));
        cap_list[i].Stop(Seconds(20));
    }
    ChatServerHelper cs (port, 10);

    ApplicationContainer sap = cs.Install(nodes.Get(0));

    sap.Start(Seconds(0));
    sap.Stop(Seconds(20));
    
    // ApplicationContainer cap;
    // for(uint16_t i = 1; i <= 10; i++){
    //     ChatClientHelper cc (if1.GetAddress(0),port + i);
    //     cap.Add(cc.Install(nodes.Get(i)));
    // }
    // cap.Start(Seconds(1));
    // cap.Stop(Seconds(20));

    Simulator::Stop(Seconds (20));
    Simulator::Run();
    Simulator::Destroy();
}
