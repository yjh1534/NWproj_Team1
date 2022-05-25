#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Default Network Topology
// Client  Client  Client  Client  Server
//   n0      n1      n2      n3
//    |       |       |       |
//   n4      n5      n6      n7      n8
//    |       |       |       |       |
//    =================================
//          server

using namespace ns3;
using namespace std;

int
main (int argc, char *argv [])
{
    RngSeedManager::SetSeed(15);

    uint32_t client_n = 4;

    CommandLine cmd;
    cmd.Parse (argc, argv);

    /* Create nodes */
    NodeContainer clientNodes;
    clientNodes.Create (client_n);

    NodeContainer serverNodes;
    serverNodes.Create (client_n);
    serverNodes.Create (1);

    /* Set p2p */
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer clientDevices[client_n];
    for (int i = 0; i < (int) client_n; i++){
        clientDevices[i] = p2p.Install (clientNodes.Get (i), serverNodes.Get (i));
    }

    /* Set csma */
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

    NetDeviceContainer serverDevices;
    serverDevices = csma.Install (serverNodes);

    /* Set Internet */
    InternetStackHelper stack;
    stack.Install (clientNodes);
    stack.Install (serverNodes);

    /* Set ip */
    Ipv4AddressHelper address;

    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer clientInterfaces;
    for (int i = 0; i < (int) client_n; i++){
        clientInterfaces = address.Assign (clientDevices[i]);
    }

    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer serverInterfaces;
    serverInterfaces = address.Assign (serverDevices);
    
    /* Set client */
    uint16_t port = 9;
    ApplicationContainer clientApps[client_n];
    for (int i = 0; i < (int) client_n; i++){
        ChatClientHelper chatClient (serverInterfaces.GetAddress(client_n), port + i);
        clientApps[i].Add(chatClient.Install (clientNodes.Get (i))); 
        clientApps[i].Start (Seconds (2.0 + ((double_t) i / 10)));
        clientApps[i].Stop (Seconds (10.0));
    }

    /* Set server */
    ChatServerHelper chatServer (port, client_n);

    ApplicationContainer serverApp = chatServer.Install (serverNodes.Get (client_n));
    serverApp.Start (Seconds (1.0));
    serverApp.Stop (Seconds (10.0));

    /* Set etc */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    p2p.EnablePcapAll ("bus_p2p");
    csma.EnablePcap ("bus_p2p", serverDevices.Get (client_n), true);

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
