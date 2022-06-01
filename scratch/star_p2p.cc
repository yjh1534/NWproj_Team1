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
NS_LOG_COMPONENT_DEFINE("star_p2p");

using namespace ns3;
using namespace std;

Ptr<ChatServer> ser1;
uint64_t lastTotalRx1 = 0;

static void 
CalculateThroughput()
{
    double cur1 = (ser1->GetTotalRx() - lastTotalRx1) * (double) 8 / 1e6;
    NS_LOG_INFO("tp\t" << Simulator::Now().GetSeconds() << "\t" << cur1);
    lastTotalRx1 = ser1->GetTotalRx();
    Simulator::Schedule(Seconds(1), &CalculateThroughput);
}

int
main (int argc, char *argv [])
{

    uint32_t client_n = 11;
    bool verbose=false;
    std::string DataRate="10Mbps";

    CommandLine cmd;
    cmd.AddValue("verbose","Logging or not",verbose);
    cmd.AddValue("client_n","The number of clients",client_n);
    cmd.Parse (argc, argv);

    if(verbose)
    {
      LogComponentEnable("star_p2p",LOG_LEVEL_INFO);
      LogComponentEnable("ChatServerApplication", LOG_LEVEL_INFO);
      LogComponentEnable("ChatClientApplication", LOG_LEVEL_INFO);
    }
    /* Set p2p */
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue (DataRate));
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
        chatClient.SetAttribute("Interval", TimeValue(Seconds(0.1)));
        clientApps[i].Add(chatClient.Install (star.GetSpokeNode (i))); 
        clientApps[i].Start (Seconds (2.0 + ((double_t) i / 10)));
        clientApps[i].Stop (Seconds (20.0));
    }

    /* Set server */
    ChatServerHelper chatServer (port, client_n);

    ApplicationContainer serverApp = chatServer.Install (star.GetHub ());
    ser1 = StaticCast<ChatServer> (serverApp.Get(0));
    serverApp.Start (Seconds (1.0));
    serverApp.Stop (Seconds (21.0));

    /* Set etc */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Simulator::Schedule(Seconds(0.0), &CalculateThroughput);
    Simulator::Stop(Seconds(21.0));
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
