/*
 * Copyright (c) 2009 The Boeing Company
 *
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
 *
 */

// This script configures two nodes on an 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to the other node.  The physical layer is configured
// to receive at a fixed RSS (regardless of the distance and transmit
// power); therefore, changing position of the nodes has no effect.
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./ns3 run "wifi-simple-adhoc --help"
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when rss drops below -97 dBm.
// To see this effect, try running:
//
// ./ns3 run "wifi-simple-adhoc --rss=-97 --numPackets=20"
// ./ns3 run "wifi-simple-adhoc --rss=-98 --numPackets=20"
// ./ns3 run "wifi-simple-adhoc --rss=-99 --numPackets=20"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the documentation.
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
//
// ./ns3 run "wifi-simple-adhoc --verbose=1"
//
// When you are done, you will notice two pcap trace files in your directory.
// If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-0-0.pcap -nn -tt
//

#include "ns3/applications-module.h"
#include "ns3/core-config.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ssid.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/basic-energy-source-helper.h"

#include <string>
#include <iostream>
#include <vector>



// Constants
const std::string PHY_MODE = "DsssRate1Mbps";
const int NUM_NODES = 2;
const int NUM_PACK = 1;
const uint32_t PACK_SIZE = 1024; // Bytes
const double PACK_INV = 1.0; // Seconds
const bool VERBOSE = false;
const bool TRACING = false;


int main(int argc, char **argv){
    if(VERBOSE){
        ns3::LogComponentEnable("UdpEchoClientApplication", ns3::LOG_LEVEL_INFO);
        ns3::LogComponentEnable("UdpEchoServerApplication", ns3::LOG_LEVEL_INFO);
    }



// Create Nodes
    ns3::NodeContainer nodes;
    nodes.Create(NUM_NODES);


// Setup Wireless Communication
    // Create & Config Wifi Devices
    ns3::YansWifiChannelHelper wifiChannel = ns3::YansWifiChannelHelper::Default();
    //wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    //wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

    ns3::YansWifiPhyHelper wifiPhy;
    //wifiPhy.SetPcapDataLinkType(ns3::WifiPhyHelper::DLT_IEEE802_11_RADIO);
    wifiPhy.SetChannel(wifiChannel.Create());

    ns3::WifiMacHelper wifiMac;

    ns3::WifiHelper wifi;
    //wifi.SetStandard(ns3::WIFI_STANDARD_80211b);
    //wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                    //"DataMode", ns3::StringValue(PHY_MODE),
                                    //"ControlMode", ns3::StringValue(PHY_MODE));

    ns3::NetDeviceContainer wifiDevices = wifi.Install(wifiPhy, wifiMac, nodes);

    //if(TRACING){
        //wifiPhy.SetPcapDataLinkType(ns3::WifiPhyHelper::DLT_IEEE802_11_RADIO);
        //wifiPhy.EnablePcap("leach-rev", wifiDevices.Get(1));
    //}



    // Enable Wifi Pcap[Tracing] 
    //wifiPhy.EnablePcap("wifi-simple-adhoc", wifiDevices);


    // Create Positioning Grid & Place Nodes
    ns3::MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                    "MinX", ns3::DoubleValue(0.0),
                                    "MinY", ns3::DoubleValue(0.0),
                                    "DeltaX", ns3::DoubleValue(1.0),
                                    "DeltaY", ns3::DoubleValue(1.0),
                                    "GridWidth", ns3::UintegerValue(3),
                                    "LayoutType", ns3::StringValue("RowFirst")); 
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);


    // Add TCP & UDP Capability to Network
    ns3::InternetStackHelper internet;
    internet.Install(nodes);


    // Config Ip & Assign Addresses
    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");

    ns3::Ipv4InterfaceContainer wifiInterfaces = ipv4.Assign(wifiDevices);



// Setup Power Consumption
/*
    // Create & Add Batteries to Nodes
    ns3::BasicEnergySourceHelper battery;
    ns3::EnergySourceContainer batteries = battery.Install(nodes);
    std::cout << batteries.GetN() << " Batteries Installed" << "\n";


    // Config Power Consumption Model
    ns3::WifiRadioEnergyModelHelper energyModel;
    energyModel.Install(wifiDevices, batteries);
*/


// Install Application
    ns3::UdpServerHelper echoServer(9);

    ns3::ApplicationContainer serverApps = echoServer.Install(nodes);
    serverApps.Start(ns3::Seconds(1.0));
    serverApps.Stop(ns3::Seconds(10.0));

    ns3::UdpEchoClientHelper echoClient(wifiInterfaces.GetAddress(0), 9);
    //echoClient.SetAttribute("MaxPackets", ns3::UintegerValue(1));
    //echoClient.SetAttribute("Interval", ns3::TimeValue(ns3::Seconds(1.0)));
    //echoClient.SetAttribute("PacketSize", ns3::UintegerValue(1024));

    ns3::ApplicationContainer clientApps = echoClient.Install(nodes);
    clientApps.Start(ns3::Seconds(2.0));
    clientApps.Stop(ns3::Seconds(10.0));

    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();



// Run Sim
    //ns3::Simulator::ScheduleWithContext(source->GetNode()->GetId(), ns3::Seconds(1.0), 
                                        //&GenerateTrafic, source, 
                                        //PACK_SIZE, NUM_PACK, PACK_INV);

    ns3::Simulator::Stop(ns3::Seconds(4.0));
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    return 0;
}
