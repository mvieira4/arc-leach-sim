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
#include "ns3/leach-helper.h"
#include "ns3/leach-sink-helper.h"
#include "ns3/gnuplot-helper.h"

#include <cmath>



// Constants
const bool verbose = true;
const uint32_t nWifi = 12;

// Globals
uint32_t deadNode = 0; 
uint32_t sentPacks = 0;
uint32_t recvPacks = 0;


using namespace ns3;

Gnuplot2dDataset data;

NS_LOG_COMPONENT_DEFINE("Main");


void SendCb(Ptr<const Packet> packet){

    DeviceNameTag tag;
    packet->PeekPacketTag(tag);

    if(tag.GetDeviceName() != "AD"){
        sentPacks++;

        NS_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>");
        NS_LOG_DEBUG("Sent: " << sentPacks);
        NS_LOG_DEBUG("Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10);
        NS_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>");
        NS_LOG_DEBUG("");
    }
}

void RecvCb(Ptr<const Packet> packet){
    DeviceNameTag tag;
    packet->PeekPacketTag(tag);

    if(tag.GetDeviceName() != "AD"){
        recvPacks++;

        NS_LOG_DEBUG("<<<<<<<<<<<<<<<<<<<<");
        NS_LOG_DEBUG("Received: " << recvPacks);
        NS_LOG_DEBUG("Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10);
        NS_LOG_DEBUG("Receive Percentage: " << std::round(double(recvPacks) / double(sentPacks) * 100) / 100);
        NS_LOG_DEBUG("<<<<<<<<<<<<<<<<<<<<");
        NS_LOG_DEBUG("");

        data.Add(Simulator::Now().GetSeconds(), 
                    std::round(double(recvPacks) / double(sentPacks) * 100));
    }
}

void EnergyCb(){
    deadNode++;

    NS_LOG_DEBUG("!!!!!!!!!!!!!!!!!!!!");
    NS_LOG_DEBUG("Energy Change");
    NS_LOG_DEBUG("Alive Nodes: " << nWifi - deadNode);
    NS_LOG_DEBUG("Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10);
    NS_LOG_DEBUG("!!!!!!!!!!!!!!!!!!!!");
    NS_LOG_DEBUG("");
}


int main(int argc, char* argv[]){
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
        LogComponentEnable("LeachSinkApplication", LOG_LEVEL_ALL);
        //LogComponentEnable("LeachNodeHelper", LOG_LEVEL_ALL);
        LogComponentEnable("Main", LOG_LEVEL_DEBUG);
    }

    // Create Nodes
    NodeContainer nodes;
    nodes.Create(nWifi + 1);

    NodeContainer sink;
    sink = nodes.Get(0);

    NodeContainer sensors;

    for (NodeContainer::Iterator i = nodes.Begin() + 1; i != nodes.End(); ++i) {
        sensors.Add(*i);
    }


    // Create & Configure Wifi Devices
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel");


    YansWifiPhyHelper phy;
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel(channel.Create());
    //phy.Set("RxGain", DoubleValue(40));    
    phy.Set("TxPowerStart", DoubleValue(100));    
    phy.Set("TxPowerEnd", DoubleValue(100));    
    phy.Set("Antennas", UintegerValue(4));    
    //phy.Set("CcaEdThreshold", DoubleValue(-100));

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    


    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    wifi.SetStandard(WIFI_STANDARD_80211g);

    NetDeviceContainer sinkDevices = wifi.Install(phy, mac, sink);
    NetDeviceContainer sensorDevices = wifi.Install(phy, mac, sensors);

    // Configure Mobility Model
    MobilityHelper mobility;

    // Set fixed position 
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(1.0),
                                  "DeltaY", DoubleValue(1.0),
                                  "GridWidth", UintegerValue(20),
                                  "LayoutType", StringValue("RowFirst"));

    mobility.Install(nodes);

    // Config Ineternet Stack
    InternetStackHelper internet;
    internet.Install(sink);
    internet.Install(sensors);

    Ipv4AddressHelper address;

    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer nodeInterfaces = address.Assign(sinkDevices);
    Ipv4InterfaceContainer sensorInterfaces = address.Assign(sensorDevices);

    // Install Batteries on Nodes
    BasicEnergySourceHelper battery;
    EnergySourceContainer sensorBatteries = battery.Install(sensors);

    // Install Energy Model on Nodes
    WifiRadioEnergyModelHelper energyModel;
    DeviceEnergyModelContainer energyModels = energyModel.Install(sensorDevices, sensorBatteries);


    LeachSinkHelper sinkLeach(nWifi);

    ApplicationContainer sinkApp = sinkLeach.Install(sink);

    sinkApp.Start(Seconds(0.5));
    sinkApp.Stop(Seconds(20.0));

    LeachNodeHelper nodeLeach(nWifi);

    ApplicationContainer nodeApps = nodeLeach.Install(sensors, energyModels);

    nodeApps.Start(Seconds(1.0));
    nodeApps.Stop(Seconds(20.0));



    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Config::Set("NodeList/0/ApplicationList/1/$ns3::LeachNodeApplication/IsMal", BooleanValue(true));

    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::LeachSinkApplication/Rx", MakeCallback(&RecvCb));
    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::LeachNodeApplication/Tx", MakeCallback(&SendCb));
    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::LeachNodeApplication/RemainingEnergy",
            MakeCallback(&EnergyCb));

    //Config::SetDefault("ns3::ArpCache::PendingQueueSize", UintegerValue(20));

    // Run Sim
    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();

    std::string fileNameWithNoExtension = "leach-plot";
    std::string graphicsFileName        = fileNameWithNoExtension + ".png";
    std::string plotFileName            = fileNameWithNoExtension + ".plt";
    std::string plotTitle               = "2-D Plot";
    std::string dataTitle               = "2-D Data";

    Gnuplot2dDataset dataset;
    data.SetTitle(dataTitle);
    data.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    Gnuplot plot(graphicsFileName);
    plot.SetTitle(plotTitle);

    plot.SetTerminal("png");
    plot.SetLegend("Time", "Percentage");
    plot.AppendExtra("set xrange [1:15]");

    plot.AddDataset(data);

    std::ofstream plotFile(plotFileName.c_str());

    plot.GenerateOutput(plotFile);

    plotFile.close();

    return 0;
}
