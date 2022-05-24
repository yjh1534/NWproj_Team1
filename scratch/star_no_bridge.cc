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
    
    uint32_t client_n = 4;

    CommandLine cmd;
    cmd.Parse (argc, argv);

    /* Create nodes */
    NodeContainer nodes;
    nodes.Create (client_n + 1);

    /* Create csma */
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
    csma.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (10)));

    /* Set link */
    NetDeviceContainer link = csma.Install (nodes);

    /* Set internet */
    InternetStackHelper internet;
    internet.Install (nodes);

    /* Set IP */
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface = ipv4.Assign(link);
     
    /* Set client */
    uint16_t port = 9;
    ApplicationContainer clientApps[client_n];
    for (int i = 0; i < (int) client_n; i++){
        ChatClientHelper chatClient (interface.GetAddress (0), port + i);
        clientApps[i].Add (chatClient.Install (nodes.Get (i))); 
        clientApps[i].Start (Seconds (2.0 + ((double_t) i / 10)));
        clientApps[i].Stop (Seconds (10.0));
    }

    /* Set server */
    ChatServerHelper chatServer (port, client_n);

    ApplicationContainer serverApp = chatServer.Install (nodes.Get (0));
    serverApp.Start (Seconds (1.0));
    serverApp.Stop (Seconds (10.0));

    /* Set etc */
    csma.EnablePcapAll ("star", false);

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
