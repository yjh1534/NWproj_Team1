#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/csma-star-helper.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Network topology (default)
//
//            n2     +          +     n3          .
//             | ... |\        /| ... |           .
//             ======= \      / =======           .
//              CSMA    \    /   CSMA             .
//                       \  /                     .
//            n1     +--- n0 ---+     n4          .
//             | ... |   /  \   | ... |           .
//             =======  /    \  =======           .
//              CSMA   /      \  CSMA             .
//                    /        \                  .
//            n6     +          +     n5          .
//             | ... |          | ... |           .
//             =======          =======           .
//              CSMA             CSMA             .
//

using namespace ns3;

int
main (int argc, char *argv [])
{
    RngSeedManager::SetSeed(15);

    uint32_t client_n = 4;

    CommandLine cmd;
    cmd.Parse (argc, argv);

    /* Set csma */
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", StringValue ("1ms"));
    CsmaStarHelper star (client_n, csma);
 
    /* Set Internet */
    InternetStackHelper stack;
    star.InstallStack (stack);

    /* Set ip */
    star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));
    
    /* Set client */
    uint16_t port = 9;
    ApplicationContainer clientApps[client_n];
    for (uint32_t i = 0; i < star.SpokeCount () ; i++){
        ChatClientHelper chatClient (star.GetHubIpv4Address(i), port);
        clientApps[i].Add(chatClient.Install (star.GetSpokeNode (i)));
        clientApps[i].Start (Seconds (2.0 + ((double_t) i / 10)));
        clientApps[i].Stop (Seconds (10.0));
    }

    /* Set server */
    ChatServerHelper chatServer (port, client_n);

    ApplicationContainer serverApp = chatServer.Install (star.GetHub ());
    serverApp.Start (Seconds (1.0));
    serverApp.Stop (Seconds (10.0));

    /* Set etc */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    csma.EnablePcapAll ("star_csma", false);

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
