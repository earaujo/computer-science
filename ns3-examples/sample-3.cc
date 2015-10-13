#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/propagation-loss-model.h"
#include <string>

using namespace ns3;

void
ReceivePacket (Ptr<Socket> socket)
{
  NS_LOG_UNCOND ("Received one packet!");
}

static void
GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                 uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

NS_LOG_COMPONENT_DEFINE ("Simple");

int
main (int argc, char *argv[])
{
  uint32_t nNodes = 2; //number of nodes
  int seed = 1; //simulation seed

  /*
   * Command line
   */
  CommandLine cmd;
  cmd.AddValue ("seed", "Seed", seed);
  cmd.AddValue ("nNodes", "Number of nodes", nNodes);
  cmd.Parse (argc, argv);

  /*
   * Set simulation seed
   */
  ns3::SeedManager::SetSeed (seed);

  /*
   * Create nodes:
   * There are one container with nNodes.
   */
  NodeContainer nodes;
  nodes.Create (nNodes);

  /*
   * Create Mobility Model:
   */
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (10.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (2),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.Install (nodes);

  /*
   * Wifi:
   * The default state is defined as being an Adhoc MAC layer with an ARF rate
   * control algorithm and both objects using their default attribute values.
   * By default, configure MAC and PHY for 802.11a.
   */
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "MaxSsrc", UintegerValue(0),
                                "MaxSlrc", UintegerValue(0),
                                "RtsCtsThreshold", UintegerValue(0),
                                "DataMode", StringValue("OfdmRate54Mbps"),
                                "ControlMode", StringValue("OfdmRate54Mbps"),
                                "NonUnicastMode",StringValue("OfdmRate54Mbps"));

  /*
   * Create WifiChannel (comunication channel):
   * Create a channel helper in a default working state. By default, we create
   * a channel model with a propagation delay equal to a constant, the speed of
   * light, and a propagation loss based on a log distance model with a
   * reference loss of 46.6777 dB at reference distance of 1m.
   *
   * Create WifiPhy:
   * Create a phy helper without any parameter set. The user must set them all
   * to be able to call Install later.
   */
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();

  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (channel);

  /*
   * Create WifiMac for each wifi node:
   * Create a mac helper in a default working state. i.e., this is an adhoc
   * mac by default
   */
  NqosWifiMacHelper wifiMac;

  char ssid_str[100];
  sprintf (ssid_str, "%s", "myssid");
  std::cout << "ssid = " << ssid_str << std::endl;

  Ssid ssid = Ssid (ssid_str);
  wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac", "Ssid", SsidValue (ssid));

  /*
   * Create netDevices:
   */
  NetDeviceContainer netDevices;
  netDevices = wifi.Install (wifiPhy, wifiMac, nodes);

  /*
   * Create stack protocols:
   */
  InternetStackHelper stack;
  stack.Install (nodes);

  /*
   * Create ip address:
   */
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces;

  char ip_str[100];
  sprintf (ip_str, "%s", "10.1.1.0");
  std::cout << "ip = " << ip_str << std::endl;

  address.SetBase (ip_str, "255.255.255.0");
  interfaces = address.Assign (netDevices);

  /*
   * Traffic:
   */
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (nodes.Get (0), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 2001);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (nodes.Get (1), tid);
  InetSocketAddress remote = InetSocketAddress (interfaces.GetAddress (0), 2001);
  source->Connect (remote);

  /*
   * Simulator configuration
   */
  NS_LOG_UNCOND ("Testing " << 5 << " packets sent" );

  Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                  Seconds (1.0), &GenerateTraffic,
                                  source, 1400, 5, Seconds (1.0));

  Simulator::Stop (Seconds (30.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
