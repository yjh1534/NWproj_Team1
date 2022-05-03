#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

using namespace ns3;


int
main(int argc, char*argv[])
{
    CommandLine cmd;
    std::string delay="10", datarate="10";

    cmd.AddValue("datarate", "datarate without Mbps", datarate);
    cmd.AddValue("delay", "delay in us", delay); 

    cmd.Parse(argc,argv);

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(datarate + "Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue(delay + "us"));


    NetDeviceContainer devices;
    devices = p2p.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper addr;
    addr.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = addr.Assign(devices);

	UdpClientHelper Client(interfaces.GetAddress(1), 9);
	Client.SetAttribute("MaxPackets", UintegerValue(10000));
	Client.SetAttribute("Interval", TimeValue(MilliSeconds(1)));
	Client.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps;
	clientApps.Add(Client.Install(nodes.Get(0)));
	clientApps.Start(Seconds(1.0));
	clientApps.Stop(Seconds(10.0));

	UdpServerHelper echoServer(9);
	ApplicationContainer serverApps;
    serverApps.Add(echoServer.Install(nodes.Get(1)));
	serverApps.Start(Seconds(0.0));
	serverApps.Stop(Seconds(11.0));

    p2p.EnablePcapAll("2017314561");

    Simulator::Run();
	Simulator::Destroy();

    return 0;
}
