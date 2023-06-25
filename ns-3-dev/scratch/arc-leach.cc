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

#include "ns3/arc-leach-helper.h"
#include "ns3/arc-leach-sink-helper.h"
#include "ns3/leach-tag.h"

#include <cmath>




using namespace ns3;

// Constants
const bool verbose = true;
const uint32_t nWifi = 12;

// Globals
uint32_t deadNode = 0; 
uint32_t sentPacks = 0;
uint32_t recvPacks = 0;
Gnuplot2dDataset packData;
Gnuplot2dDataset aliveData;


NS_LOG_COMPONENT_DEFINE("Main");


void SendCb(Ptr<const Packet> packet){
    LeachTag tag;
    packet->PeekPacketTag(tag);

    if(tag.GetPackType() == LeachTag::RE || tag.GetPackType() == LeachTag::ARE){
        sentPacks ++;

        NS_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>");
        NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
        NS_LOG_DEBUG("Sent: " << sentPacks);
        NS_LOG_DEBUG("Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10);
        NS_LOG_DEBUG("Info Id: " << (uint32_t)tag.GetPackType());
        NS_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>");
        NS_LOG_DEBUG("");
    }
    else{
        NS_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>");
        NS_LOG_DEBUG("Info Id: " << (uint32_t)tag.GetPackType());
        NS_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>");
    }
}


void RecvCb(Ptr<const Packet> packet){
    LeachTag tag;
    packet->PeekPacketTag(tag);

    if(tag.GetPackType() == LeachTag::RE || tag.GetPackType() == LeachTag::ARE){
        recvPacks ++;

        NS_LOG_DEBUG("<<<<<<<<<<<<<<<<<<<<");
        NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
        NS_LOG_DEBUG("Received: " << recvPacks);
        NS_LOG_DEBUG("Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10);
        NS_LOG_DEBUG("Receive Percentage: " << std::round(double(recvPacks) / double(sentPacks) * 100) / 100);
        NS_LOG_DEBUG("Info Id: " << (uint32_t)tag.GetPackType());
        NS_LOG_DEBUG("<<<<<<<<<<<<<<<<<<<<");
        NS_LOG_DEBUG("");
    }
    else{
        NS_LOG_DEBUG("<<<<<<<<<<<<<<<<<<<<");
        NS_LOG_DEBUG("Info Id: " << (uint32_t)tag.GetPackType());
        NS_LOG_DEBUG("<<<<<<<<<<<<<<<<<<<<");
        NS_LOG_DEBUG("");
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

void StatusCb(uint32_t round){
    aliveData.Add(round, nWifi - deadNode);

    if(recvPacks){
        packData.Add(round, 
                        std::round(double(recvPacks) / double(sentPacks) * 100));
    }
    else{
        packData.Add(round, 0);
    }

    NS_LOG_DEBUG("%%%%%%%%%%%%%%%%%%%%");
    NS_LOG_DEBUG("Status Update");
    NS_LOG_DEBUG("Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10);
    NS_LOG_DEBUG("Alive Nodes: " << nWifi - deadNode);
    NS_LOG_DEBUG("Receive Percentage: " << std::round(double(recvPacks) / double(sentPacks) * 100) / 100);
    NS_LOG_DEBUG("%%%%%%%%%%%%%%%%%%%%");
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
        LogComponentEnable("ArcLeachNodeApplication", LOG_LEVEL_ALL);
        LogComponentEnable("ArcLeachSinkApplication", LOG_LEVEL_ALL);
        //LogComponentEnable("LeachTagList", LOG_LEVEL_DEBUG);
        //LogComponentEnable("LeachTag", LOG_LEVEL_ALL);
        //LogComponentEnable("ArcLeachNodeHelper", LOG_LEVEL_ALL);
        LogComponentEnable("Main", LOG_LEVEL_ALL);
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


    // Create & Configure Wifi Deviceshttps://umassd.zoom.us/j/93281343753?pwd=UWd5TGsweFpyMC9ydWhzaWErZnlndz09
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel");


    YansWifiPhyHelper phy;
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel(channel.Create());
    phy.Set("TxPowerStart", DoubleValue(100));    
    phy.Set("TxPowerEnd", DoubleValue(100));    
    phy.Set("Antennas", UintegerValue(4));    

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
                                  "DeltaX", DoubleValue(2.0),
                                  "DeltaY", DoubleValue(2.0),
                                  "GridWidth", UintegerValue(40),
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
    battery.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(500000));

    EnergySourceContainer sensorBatteries = battery.Install(sensors);

    // Install Energy Model on Nodes
    WifiRadioEnergyModelHelper energyModel;
    DeviceEnergyModelContainer energyModels = energyModel.Install(sensorDevices, sensorBatteries);


    ArcLeachSinkHelper sinkLeach(nWifi);

    ApplicationContainer sinkApp = sinkLeach.Install(sink);

    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(5000000.0));

    ArcLeachNodeHelper nodeLeach(nWifi);

    ApplicationContainer nodeApps = nodeLeach.Install(sensors, energyModels);

    nodeApps.Start(Seconds(0.0));
    nodeApps.Stop(Seconds(5000000.0));



    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::ArcLeachSinkApplication/Rx", MakeCallback(&RecvCb));
    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::ArcLeachNodeApplication/Tx", MakeCallback(&SendCb));
    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::ArcLeachNodeApplication/RemainingEnergy",
            MakeCallback(&EnergyCb));
    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::ArcLeachNodeApplication/Status",
            MakeCallback(&StatusCb));

    Config::Set("NodeList/1/ApplicationList/0/$ns3::ArcLeachNodeApplication/IsMal", BooleanValue(true));
    //Config::Set("NodeList/2/ApplicationList/0/$ns3::ArcLeachNodeApplication/IsMal", BooleanValue(true));
    //Config::Set("NodeList/3/ApplicationList/0/$ns3::ArcLeachNodeApplication/IsMal", BooleanValue(true));

    // Run Sim
    Simulator::Stop(Seconds(5000000.0));
    Simulator::Run();
    Simulator::Destroy();

    std::string fileNameWithNoExtension = "arc-pack-plot";
    std::string graphicsFileName        = fileNameWithNoExtension + ".png";
    std::string plotFileName            = fileNameWithNoExtension + ".plt";
    std::string plotTitle               = "2-D Plot";
    std::string dataTitle               = "2-D Data";

    packData.SetTitle(dataTitle);
    packData.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    Gnuplot plot1(graphicsFileName);
    plot1.SetTitle(plotTitle);

    plot1.SetTerminal("png");
    plot1.SetLegend("Time", "Percentage");
    plot1.AppendExtra("set yrange [0:110]");

    plot1.AddDataset(packData);

    std::ofstream plotFile1(plotFileName.c_str());

    plot1.GenerateOutput(plotFile1);

    plotFile1.close();



    fileNameWithNoExtension = "arc-alive-plot";
    graphicsFileName        = fileNameWithNoExtension + ".png";
    plotFileName            = fileNameWithNoExtension + ".plt";
    plotTitle               = "2-D Plot";
    dataTitle               = "2-D Data";

    packData.SetTitle(dataTitle);
    packData.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    Gnuplot plot2(graphicsFileName);
    plot2.SetTitle(plotTitle);

    plot2.SetTerminal("png");
    plot2.SetLegend("Time", "Percentage");
    plot2.AppendExtra("set yrange [0:45]");
    //plot2.AppendExtra("set xrange [20000:25000]");

    plot2.AddDataset(aliveData);

    std::ofstream plotFile2(plotFileName.c_str());

    plot2.GenerateOutput(plotFile2);

    plotFile2.close();

    return 0;
}
