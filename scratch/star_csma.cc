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
NS_LOG_COMPONENT_DEFINE("star_csma");
using namespace ns3;

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
    RngSeedManager::SetSeed(15);

    uint32_t client_n = 11;
    bool verbose=false;
    std::string DataRate="10Mbps";

    CommandLine cmd;
    cmd.AddValue("verbose","Logging or not",verbose);
    cmd.AddValue("client_n","The number of clients",client_n);
    cmd.AddValue("datarate","Setting the datarate of the channel", DataRate);
    cmd.Parse (argc, argv);

    if(verbose)
    {
        LogComponentEnable("star_csma",LOG_LEVEL_INFO);
        LogComponentEnable("ChatServerApplication", LOG_LEVEL_INFO);
        LogComponentEnable("ChatClientApplication", LOG_LEVEL_INFO);
    }

    /* Set csma */
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue (DataRate));
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
//        ChatClientHelper chatClient (star.GetHubIpv4Address(i), port);
        ChatClientHelper chatClient (star.GetHubIpv4Address(0), port);
//        chatClient.SetAttribute("Interval", TimeValue(Seconds(0.1)));
        chatClient.SetAttribute("Interval", TimeValue(Seconds(1)));
        clientApps[i].Add(chatClient.Install (star.GetSpokeNode (i)));
        clientApps[i].Start (Seconds (2.0 + ((double_t) i / 10)));
//        clientApps[i].Stop (Seconds (20.0));
        clientApps[i].Stop (Seconds (10 + i));
    }

    /* Set server */
    ChatServerHelper chatServer (port, client_n);

    ApplicationContainer serverApp = chatServer.Install (star.GetHub ());
//    ser1 = StaticCast<ChatServer> (serverApp.Get(0));
    ser1=DynamicCast<ChatServer>(serverApp.Get(0));
    serverApp.Start (Seconds (1.0));
    serverApp.Stop (Seconds (21.0));

    /* Set etc */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Simulator::Schedule(Seconds(0.0), &CalculateThroughput);
    
    if(verbose)
    {
        csma.EnablePcapAll ("star_csma", false);
    }

    Simulator::Stop(Seconds(21.0));
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
