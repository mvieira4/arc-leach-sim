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



NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");

int main(int argc, char* argv[]){
    bool verbose = true;
    uint32_t nWifi = 5;
    bool tracing = false;

    if (verbose){
        LogComponentEnable("UdpEchoClientApplication", ns3::LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", ns3::LOG_LEVEL_INFO);
        LogComponentEnable("WifiRadioEnergyModel", ns3::LOG_LEVEL_INFO);
    }


    // Create Nodes
    ns3::NodeContainer nodes;
    nodes.Create(nWifi);



    // Create & Configure Wifi Devices
    ns3::YansWifiChannelHelper channel = ns3::YansWifiChannelHelper::Default();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel");

    ns3::YansWifiPhyHelper phy;
    phy.SetPcapDataLinkType(ns3::WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel(channel.Create());

    ns3::WifiMacHelper mac;

    ns3::WifiHelper wifi;
    wifi.SetStandard(ns3::WIFI_STANDARD_80211b);

    ns3::NetDeviceContainer devices = wifi.Install(phy, mac, nodes);



    // Configure Mobility Model
    ns3::MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", ns3::DoubleValue(0.0),
                                  "MinY", ns3::DoubleValue(0.0),
                                  "DeltaX", ns3::DoubleValue(1.0),
                                  "DeltaY", ns3::DoubleValue(1.0),
                                  "GridWidth", ns3::UintegerValue(8),
                                  "LayoutType", ns3::StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);




    // Config Ineternet Stack
    ns3::InternetStackHelper internet;
    internet.Install(nodes);

    ns3::Ipv4AddressHelper address;

    address.SetBase("10.1.3.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer interfaces = address.Assign(devices);



    // Config Echo Server
    ns3::UdpEchoServerHelper echoServer(9);

    ns3::ApplicationContainer serverApps = echoServer.Install(nodes);
    serverApps.Start(ns3::Seconds(1.0));
    serverApps.Stop(ns3::Seconds(12.0));



    // Config Echo Clients
    ns3::UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
    echoClient.SetAttribute("Interval", ns3::TimeValue(ns3::Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", ns3::UintegerValue(1024));

    ns3::NodeContainer clientNodes;

    for(int i = 1; i < nWifi; i++){
        clientNodes.Add(nodes.Get(i));
    }

    ns3::ApplicationContainer clientApps = echoClient.Install(clientNodes);

    clientApps.Start(ns3::Seconds(2.0));
    clientApps.Stop(ns3::Seconds(12.0));




    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    


    // Install Batteries on Nodes
    ns3::BasicEnergySourceHelper battery;
    ns3::EnergySourceContainer batteries = battery.Install(nodes);


    // Install Energy Model on Nodes
    ns3::WifiRadioEnergyModelHelper energyModel;
    energyModel.Install(devices, batteries);


    // Run Sim
    ns3::Simulator::Stop(ns3::Seconds(12.0));
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    return 0;
}
