#include <iostream>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"

NS_LOG_COMPONENT_DEFINE("wifi_ad");

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
    uint32_t client_n = 11;
    bool verbose=false;

    CommandLine cmd;
    cmd.AddValue("verbose","Logging or not",verbose);
    cmd.AddValue("client_n","The number of clients",client_n);
    cmd.Parse (argc, argv);

    if(verbose)
    {
      LogComponentEnable("wifi_ad",LOG_LEVEL_INFO);
      LogComponentEnable("ChatServerApplication", LOG_LEVEL_INFO);
      LogComponentEnable("ChatClientApplication", LOG_LEVEL_INFO);
    }
    /* Create nodes */
    NodeContainer clientNodes;
    clientNodes.Create (client_n);

    NodeContainer serverNode;
    serverNode.Create (1);

    /* Set PHY layer */
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create ());

    /* Set MAC layer */
    WifiMacHelper mac;
    Ssid ssid = Ssid("wifi_ad");
    mac.SetType ("ns3::AdhocWifiMac");

    /* Set WLAN */
    WifiHelper wifi;
    wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate54Mbps"));

    /* Set Devices */
    NetDeviceContainer clientDevices;
    clientDevices = wifi.Install (phy, mac, clientNodes);

    NetDeviceContainer serverDevice;
    serverDevice = wifi.Install (phy, mac, serverNode);

    /* Set internet */
    InternetStackHelper stack;
    stack.Install (clientNodes);
    stack.Install (serverNode);

    /* Set ip */
    Ipv4AddressHelper address;

    address.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer clientInterfaces;
    clientInterfaces = address.Assign (clientDevices);
    Ipv4InterfaceContainer serverInterface;
    serverInterface = address.Assign (serverDevice);

    /* Set mobility */
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

    positionAlloc->Add (Vector(0.0, 0.0, 0.0));
    positionAlloc->Add (Vector(1.0, 0.0, 0.0));
    mobility.SetPositionAllocator (positionAlloc);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (clientNodes);
    mobility.Install (serverNode);
    
    /* Set client */
    uint16_t port = 9;
    ApplicationContainer clientApps[client_n];
    for (int i = 0; i < (int) client_n; i++){
        ChatClientHelper chatClient (serverInterface.GetAddress(0), port + i);
        chatClient.SetAttribute("Interval", TimeValue(Seconds(0.1)));
        clientApps[i].Add(chatClient.Install (clientNodes.Get (i))); 
        clientApps[i].Start (Seconds (2.0 + ((double_t) i / 10)));
        clientApps[i].Stop (Seconds (20.0));
    }

    /* Set server */
    ChatServerHelper chatServer (port, client_n);

    ApplicationContainer serverApp = chatServer.Install (serverNode.Get (0));
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
