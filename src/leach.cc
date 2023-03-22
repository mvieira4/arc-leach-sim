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
#include "ns3/gnuplot-helper.h"
#include "ns3/file-helper.h"

#include "ns3/leach-helper.h"

using namespace ns3;

int main(int argc, char* argv[]){
    bool verbose = true;
    int nWifi = 3;

    if (verbose){
        //LogComponentEnable("Ipv4EndPoint", LOG_LEVEL_ALL);
        //LogComponentEnable("Ipv4EndPointDemux", LOG_LEVEL_ALL);
        //LogComponentEnable("Packet", LOG_LEVEL_INFO);
        //LogComponentEnable("Socket", LOG_LEVEL_ALL);
        //LogComponentEnable("UdpSocketImpl", LOG_LEVEL_ALL);
        //LogComponentEnable("ArpCache", ns3::LOG_LEVEL_ALL);
        //LogComponentEnable("ArpL3Protocol", LOG_LEVEL_ALL);
        //LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_INFO);
        //LogComponentEnable("Ipv4Interface", LOG_LEVEL_ALL);
        //LogComponentEnable("IpL4Protocol", LOG_LEVEL_ALL);
        //LogComponentEnable("UdpL4Protocol", LOG_LEVEL_ALL);
        //LogComponentEnable("YansWifiPhy", ns3::LOG_LEVEL_INFO);
        //LogComponentEnable("YansWifiChannel", LOG_LEVEL_INFO);
        //LogComponentEnable("BasicEnergySource", LOG_LEVEL_INFO);
        //LogComponentEnable("WifiRadioEnergyModel", LOG_LEVEL_INFO);
        //LogComponentEnable("WifiMac", LOG_LEVEL_ALL);
        LogComponentEnable("LeachNodeApplication", LOG_LEVEL_ALL);
        //LogComponentEnable("LeachNodeHelper", LOG_LEVEL_ALL);
    }

    // Create Nodes
    NodeContainer nodes;
    nodes.Create(nWifi);

    // Create & Configure Wifi Devices
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel");

    YansWifiPhyHelper phy;
    //phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    //wifi.SetStandard(WIFI_STANDARD_80211b); // 2.4GHz

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // Configure Mobility Model
    MobilityHelper mobility;

    // Set fixed position 
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(1.0),
                                  "DeltaY", DoubleValue(1.0),
                                  "GridWidth", UintegerValue(1),
                                  "LayoutType", StringValue("RowFirst"));

    mobility.Install(nodes);

    // Config Ineternet Stack
    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;

    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Install Batteries on Nodes
    BasicEnergySourceHelper battery;
    EnergySourceContainer batteries = battery.Install(nodes);

    // Install Energy Model on Nodes
    WifiRadioEnergyModelHelper energyModel;
    DeviceEnergyModelContainer energyModels = energyModel.Install(devices, batteries);

    LeachNodeHelper nodeLeach(nWifi);

    ApplicationContainer nodeApps = nodeLeach.Install(nodes, energyModels);

    nodeApps.Start(Seconds(1));
    nodeApps.Stop(Seconds(20));


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


    std::string probeType = "ns3::Ipv4PacketProbe";
    std::string tracePath = "/NodeList/*/$ns3::Ipv4L3Protocol/Tx";

    GnuplotHelper plot;
    plot.ConfigurePlot("leach-node-plot", "Nodes Alive vs Time", "Time", "Nodes Alive");

    plot.PlotProbe(probeType,
                         tracePath,
                         "OutputBytes",
                         "Packet Byte Count",
                         GnuplotAggregator::KEY_BELOW);

    // Use FileHelper to write out the packet byte count over time
    FileHelper fileHelper;

    // Configure the file to be written, and the formatting of output data.
    fileHelper.ConfigureFile("seventh-packet-byte-count", FileAggregator::FORMATTED);

    // Set the labels for this formatted output file.
    fileHelper.Set2dFormat("Time (Seconds) = %.3e\tPacket Byte Count = %.0f");

    // Specify the probe type, trace source path (in configuration namespace), and
    // probe output trace source ("OutputBytes") to write.
    fileHelper.WriteProbe(probeType, tracePath, "OutputBytes");

    // Run Sim
    Simulator::Stop(Seconds(10.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
