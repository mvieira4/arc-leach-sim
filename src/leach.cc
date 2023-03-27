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

#include <cmath>



// Constants
const bool verbose = true;
const uint32_t nWifi = 5;


// Globals
uint32_t deadNode = 0; 
uint32_t sentPacks = 0;
uint32_t recvPacks = 0;


using namespace ns3;

void SendCb(Ptr<const Packet> packet){
    sentPacks++;

    std::cout << ">>>>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "Sent: " << sentPacks << std::endl;
    std::cout << "Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10  << std::endl;
    std::cout << ">>>>>>>>>>>>>>>>>>>>" << std::endl;
    std::cout << "" << std::endl;
}

void RecvCb(Ptr<const Packet> packet){
    recvPacks++;

    std::cout << "<<<<<<<<<<<<<<<<<<<<" << std::endl;
    std::cout << "Received: " << recvPacks << std::endl;
    std::cout << "Receive Percentage: " << std::round(double(recvPacks) / double(sentPacks) * 100) / 100 << std::endl;
    std::cout << "Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10  << std::endl;
    std::cout << "<<<<<<<<<<<<<<<<<<<<" << std::endl;
    std::cout << "" << std::endl;
}

void EnergyCb(){
    deadNode++;

    std::cout << "!!!!!!!!!!!!!!!!!!!!" << std::endl;
    std::cout << "Energy Change" << std::endl;
    std::cout << "Alive Nodes: " << nWifi - deadNode << std::endl;
    std::cout << "Time: " << std::round(Simulator::Now().GetSeconds() * 10) / 10  << std::endl;
    std::cout << "!!!!!!!!!!!!!!!!!!!!" << std::endl;
    std::cout << "" << std::endl;
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
        LogComponentEnable("LeachNodeApplication", LOG_LEVEL_DEBUG);
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
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    wifi.SetStandard(WIFI_STANDARD_80211b); // 2.4GHz

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // Configure Mobility Model
    MobilityHelper mobility;

    // Set fixed position 
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(0.4),
                                  "DeltaY", DoubleValue(0.4),
                                  "GridWidth", UintegerValue(20),
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

    Config::Set("NodeList/0/ApplicationList/1/$ns3::LeachNodeApplication/IsMal", BooleanValue(true));

    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::LeachNodeApplication/Rx", MakeCallback(&RecvCb));
    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::LeachNodeApplication/Tx", MakeCallback(&SendCb));
    Config::ConnectWithoutContext("NodeList/*/ApplicationList/*/$ns3::LeachNodeApplication/RemainingEnergy",
            MakeCallback(&EnergyCb));

    // Run Sim
    Simulator::Stop(Seconds(20));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
