#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// Network topology (default)
//
//        n2 n3 n4              .
//         \ | /                .
//          \|/                 .
//     n1--- n0---n5            .
//          /|\                 .
//         / | \                .
//        n8 n7 n6              .
//

using namespace ns3;
using namespace std;

int
main (int argc, char *argv [])
{
    RngSeedManager::SetSeed(15);

    uint32_t client_n = 4;

    CommandLine cmd;
    cmd.Parse (argc, argv);

    /* Set p2p */
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    PointToPointStarHelper star (client_n, p2p);
 
    /* Set Internet */
    InternetStackHelper stack;
    star.InstallStack (stack);

    /* Set ip */
    star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));
    
    /* Set client */
    uint16_t port = 9;
    ApplicationContainer clientApps[client_n];
    for (int i = 0; i < (int) client_n; i++){
        ChatClientHelper chatClient (star.GetHubIpv4Address(0), port + i);
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

    p2p.EnablePcapAll ("star_p2p");

    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
