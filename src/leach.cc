/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/netanim-module.h"

#include "ns3/leach-helper.h"

using namespace ns3;

int main(int argc, char* argv[]){
    bool verbose = true;
    int nWifi = 5;

    if (verbose){
        LogComponentEnable("Packet", LOG_LEVEL_INFO);
        LogComponentEnable("Socket", LOG_LEVEL_INFO);
        LogComponentEnable("WifiRadioEnergyModel", LOG_LEVEL_INFO);
        //LogComponentEnable("BasicEnergySource", LOG_LEVEL_INFO);
        LogComponentEnable("LeachNodeApplication", LOG_LEVEL_INFO);
        LogComponentEnable("LeachNodeHelper", LOG_LEVEL_INFO);
    }

    // Create Nodes
    NodeContainer nodes;
    nodes.Create(nWifi);

    // Create & Configure Wifi Devices
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    //channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel");

    YansWifiPhyHelper phy;
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // Configure Mobility Model
    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(2.0),
                                  "DeltaY", DoubleValue(2.0),
                                  "GridWidth", UintegerValue(4),
                                  "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Config Ineternet Stack
    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;

    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    /*
    // Config Echo Server
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(nodes);
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(12.0));

    // Config Echo Clients
    UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    NodeContainer clientNodes;

    for(int i = 1; i < nWifi; i++){
        clientNodes.Add(nodes.Get(i));
    }

    ApplicationContainer clientApps = echoClient.Install(clientNodes);

    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(12.0));
    */

    NodeContainer sensors;

    for(int i = 1; i < nWifi; i++){
        sensors.Add(nodes.Get(i));
    }


    // Install Batteries on Nodes
    BasicEnergySourceHelper battery;
    EnergySourceContainer batteries = battery.Install(nodes);

    // Install Energy Model on Nodes
    WifiRadioEnergyModelHelper energyModel;
    DeviceEnergyModelContainer energyModels = energyModel.Install(devices, batteries);


    LeachNodeHelper nodeLeach(nWifi);

    ApplicationContainer nodeApps = nodeLeach.Install(nodes, batteries);

    nodeApps.Start(Seconds(0));
    nodeApps.Stop(Seconds(20));


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AnimationInterface anim("animation.xml");

    for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); ++it) {
      Ptr<Node> node = *it;
      anim.SetConstantPosition(node, 10, 10);
      anim.UpdateNodeColor(node, 255, 0, 0);
    }

    // Run Sim
    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
