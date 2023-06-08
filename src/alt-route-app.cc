#include "ns3/packet-metadata.h"
#include "ns3/address-utils.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/udp-socket.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/basic-energy-source.h"

#include "ns3/alt-route-app.h"
#include "ns3/leach-tag-list.h"

#include <cmath>
#include <random>

namespace ns3{
    NS_LOG_COMPONENT_DEFINE("AltRouteNodeApplication");
    
    // Constructor
    AltRouteNodeApplication::AltRouteNodeApplication(): m_lossCounter(0){
        NS_LOG_FUNCTION(this << Simulator::Now());

        // Time between events
        m_interval = MilliSeconds(700);

        // Event values per round
        m_roundEventN = 0;
        m_roundEvents = 50; 

        // Round values
        m_roundN = 0;
        m_rounds = 100;

        // Alt routing values
        m_routes = 8;

        // Port number 
        m_port = 50;
        
        // Default packet size
        m_packetSize = 1029; // 1024 + 4 Byte Ip + 1 Pack Type

        // Max aggrogated packet size
        m_maxPacketSize = 0;

        // Maximum number of nodes to use
        m_maxNodes = 40;

        // Set sequence number to 0;
        m_seqNum = 1;

        m_blacklist = {};
        m_nodes = {};
    }


    // Deconstructor
    AltRouteNodeApplication::~AltRouteNodeApplication(){
        NS_LOG_FUNCTION(this << Simulator::Now());

        // Clear aggrogated packet
        m_agroPacket = nullptr;

        // Clear socket
        m_socket = nullptr;

        // Clear energy model
        m_energyModel = nullptr;
    }


    // Destroys application 
    void AltRouteNodeApplication::DoDispose(){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << Simulator::Now().As(Time::S));

        Application::DoDispose();
    }


    // Initilize application
    void AltRouteNodeApplication::StartApplication(){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << Simulator::Now().As(Time::S));

        // Create a random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
          
        // Define the range of the random number
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        // Node states 
        m_dead = false; 

        double random_double = dis(gen);
        //m_isMal = random_double < 1.0/12;
        //m_isMal = false;

        random_double = dis(gen);
        m_isCh = random_double < 0.25;

        // Set important ipv4 addresses 
        Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
        m_localAddress = ipv4->GetAddress (1, 0).GetLocal();
        m_sinkAddress = Ipv4Address("10.0.0.1");
        m_chAddress = m_sinkAddress;

        // Create & config udp socket
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        m_socket->SetAllowBroadcast(true);
        m_socket->BindToNetDevice(GetNode()->GetDevice(0)); 
        m_socket->Bind(InetSocketAddress(m_localAddress, m_port)); 
        m_socket->SetRecvCallback(MakeCallback(&AltRouteNodeApplication::HandleRead, this));
        m_socket->Listen();

        // Start application based on node id
        NS_LOG_DEBUG("Start: " << MilliSeconds(GetNode()->GetId() * m_interval.GetMilliSeconds()).As(Time::S));


        // Start status collection
        ReportStatus();

        // Start first round
        ScheduleNextRound(MilliSeconds(GetNode()->GetId() * m_interval.GetMilliSeconds()));
    }


    // Terminate application
    void AltRouteNodeApplication::StopApplication(){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << Simulator::Now().As(Time::S));
     
        // Disable receive callback 
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }


    void AltRouteNodeApplication::ReportStatus(){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << Simulator::Now().As(Time::S));

        m_statusTrace(m_roundN);

        if(!m_dead && m_roundN < m_rounds){
            NS_LOG_DEBUG("Round: " << m_roundN);
            NS_LOG_DEBUG("Energy: " << m_energyModel->GetTotalEnergyConsumption());

            Simulator::Schedule(Seconds(3), &AltRouteNodeApplication::ReportStatus, this);
        }
    }


    // Schedule round
    void AltRouteNodeApplication::ScheduleNextRound(Time dt){
        NS_LOG_FUNCTION(this << GetNode()->GetId());

        // Exit if node dead
        if(CheckDead()){
            return;
        }

        // Schedule next round unless target number reached
        if(m_roundN < m_rounds){
            Simulator::Schedule(dt, &AltRouteNodeApplication::ExecuteRound, this);
        }
    }


    // Run the round
    void AltRouteNodeApplication::ExecuteRound(){
        NS_LOG_FUNCTION(this << GetNode()->GetId());

        m_roundN++;

        //ReportStatus();

        // Create a random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
          
        // Define the range of the random number
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        double random_double = dis(gen);

        if(!m_isCh){
            m_isCh = random_double < 0.33;
        }
        else{
            m_isCh = false;
        }

        // Set default round values
        m_roundEventN = 0;
        m_advertisments = 0;
        m_shortestDelay = Seconds(100);
        m_chPrevAddress = m_chAddress;
        m_chAddress = m_sinkAddress;

        m_nodes.clear();

        // Reset aggrogated packet
        m_agroPacket = Create<Packet>(m_packetSize);

        ScheduleAdvertise(Seconds(0));
    }


    void AltRouteNodeApplication::ScheduleAdvertise(Time dt){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << m_advertisments);

        // Get wifi channel
        Ptr<Channel> channel = GetNode()->GetDevice(0)->GetChannel();


        if(m_advertisments < channel->GetNDevices()){
            Simulator::Schedule(dt, &AltRouteNodeApplication::Advertise, this);
        }
        else{
            ScheduleNextEvent(dt);
        }
    }


    void AltRouteNodeApplication::Advertise(){
        NS_LOG_FUNCTION(this << GetNode()->GetId());
        
        LeachTag tag;
        tag.SetTimestamp(Simulator::Now());
        tag.SetNodeAddress(m_localAddress);
        tag.SetCHAddress(m_chAddress);
        tag.SetCHPrevAddress(m_chPrevAddress);

        if(m_isCh){
            tag.SetPackType(LeachTag::AD);
        }
        else{
            tag.SetPackType(LeachTag::ADN);
        }

        // Create & config packet
        Ptr<Packet> packet;
        packet = Create<Packet>(m_packetSize);
        packet->AddPacketTag(tag);

        // Get wifi channel
        Ptr<Channel> channel = GetNode()->GetDevice(0)->GetChannel();

        // Send message to all nodes on channel 
        Ptr<Ipv4> ipv4= channel->GetDevice(m_advertisments)->GetNode()->GetObject<Ipv4>();
        Ipv4Address address = ipv4->GetAddress (1, 0).GetLocal();


        if(address != m_localAddress){
            if(CheckDead()){
                return;
            }

            m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));

            if(m_isCh){
                NS_LOG_DEBUG("---------CH SENT--------");
                NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                NS_LOG_DEBUG("Ip: " << m_localAddress);
                NS_LOG_DEBUG("To: " << address);
                NS_LOG_DEBUG("Ch: " << m_isCh);
                NS_LOG_DEBUG("Mal: " << m_isMal);
                NS_LOG_DEBUG("Round: " << m_roundN);
                NS_LOG_DEBUG("Event: " << m_roundEventN);
                NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());
                NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
                NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
                NS_LOG_DEBUG("------------------------");
                NS_LOG_DEBUG("");
            }
            else{
                NS_LOG_DEBUG("--------NODE SENT-------");
                NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                NS_LOG_DEBUG("Ip: " << m_localAddress);
                NS_LOG_DEBUG("To: " << address);
                NS_LOG_DEBUG("Ch: " << m_isCh);
                NS_LOG_DEBUG("Mal: " << m_isMal);
                NS_LOG_DEBUG("Round: " << m_roundN);
                NS_LOG_DEBUG("Event: " << m_roundEventN);
                NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());
                NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
                NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
                NS_LOG_DEBUG("------------------------");
                NS_LOG_DEBUG("");

            }
        }

        m_advertisments++;

        ScheduleAdvertise(MilliSeconds(40 * m_interval.GetMilliSeconds()));
    }


    void AltRouteNodeApplication::ScheduleNextEvent(Time dt){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << m_roundEventN << m_roundEvents);

        if(m_roundEventN < m_roundEvents){
            m_routeN = 0;
            m_target = m_chAddress;
            m_ack = false;
            Simulator::Schedule(dt, &AltRouteNodeApplication::AltRoute, this);
        }
        else{
            ScheduleNextRound(dt);
        }
    }


    void AltRouteNodeApplication::AltRoute(){
        NS_LOG_FUNCTION(this);

        if(CheckDead()){
            return;
        }

        if(m_routeN < m_routes){
            ReportEvent();
        }
        else{
            m_roundEventN++;
            ScheduleNextEvent(m_maxNodes * m_interval);
        }

    }


    void AltRouteNodeApplication::ReportEvent(){
        NS_LOG_FUNCTION(this << GetNode()->GetId());

        if(CheckDead()){
            return;
        }

        LeachTag tag;
        tag.SetPackType(LeachTag::RE);
        tag.SetSeqNum(m_seqNum);
        tag.SetTimestamp(Simulator::Now());
        tag.SetNodeAddress(m_localAddress);
        tag.SetCHAddress(m_chAddress);
        tag.SetCHPrevAddress(m_chPrevAddress);

        Ptr<Packet> packet;
        packet = Create<Packet>(m_packetSize);
        packet->AddPacketTag(tag);

        if(m_routeN == 0){
            m_txTrace(packet);
        }

        m_routeN++;

        if(!m_ack){
            Send(packet, m_target);
        }

        if(!m_nodes.empty()){
            m_target = m_nodes.back();
            m_nodes.pop_back();
        }

        Simulator::Schedule(m_interval, &AltRouteNodeApplication::AltRoute, this);
    }


    void AltRouteNodeApplication::Send(Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << address);

        if(CheckDead()){
            return;
        }

        LeachTag tag;
        packet->PeekPacketTag(tag);
        
        if(!m_isCh && !m_isMal){
            m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));


            NS_LOG_DEBUG("--------NODE SENT-------");
            NS_LOG_DEBUG("Id: " << GetNode()->GetId());
            NS_LOG_DEBUG("Ip: " << m_localAddress);
            NS_LOG_DEBUG("To: " << address);
            NS_LOG_DEBUG("Ch: " << m_isCh);
            NS_LOG_DEBUG("Mal: " << m_isMal);
            NS_LOG_DEBUG("Round: " << m_roundN);
            NS_LOG_DEBUG("Event: " << m_roundEventN);
            NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());

            NS_LOG_DEBUG("Packet Num: " << (uint32_t)tag.GetSeqNum());
            NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
            NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");

        }
        else if(m_isMal){
            NS_LOG_DEBUG("--------MAL NODE--------");
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");

        }
        else if(m_isCh){
            //AppendPack(packet);
            CHSend(packet);
        }

        m_seqNum++;

    }

    void AltRouteNodeApplication::CHSend(Ptr<Packet> packet){
        NS_LOG_FUNCTION(this);

        if(CheckDead()){
            return;
        }

        LeachTag tag;

        tag.SetPackType(LeachTag::ARE);

        m_agroPacket->AddPacketTag(tag);

        if(m_isCh && !m_isMal){
            m_socket->SendTo(packet, 0, InetSocketAddress(m_sinkAddress, m_port));

            packet->PeekPacketTag(tag);

            NS_LOG_DEBUG("---------CH SENT--------");
            NS_LOG_DEBUG("Id: " << GetNode()->GetId());
            NS_LOG_DEBUG("Ip: " << m_localAddress);
            NS_LOG_DEBUG("To: " << m_sinkAddress);
            NS_LOG_DEBUG("Mal: " << m_isMal);
            NS_LOG_DEBUG("Round: " << m_roundN);
            NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());
            NS_LOG_DEBUG("Packet Num: " << (uint32_t)tag.GetSeqNum());
            NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
            NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");
        }
        else if(m_isMal){
            NS_LOG_DEBUG("---------MAL CH---------");
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");
        }

        m_agroPacket = Create<Packet>(m_packetSize);
    }

    void AltRouteNodeApplication::CHSend(Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << m_isMal);

        LeachTag tag;
        packet->PeekPacketTag(tag);


        //packet = Create<Packet>(m_packetSize);

        m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));

        NS_LOG_DEBUG("--------ACK SENT--------");
        NS_LOG_DEBUG("Id: " << GetNode()->GetId());
        NS_LOG_DEBUG("Ip: " << m_localAddress);
        NS_LOG_DEBUG("To: " << address);
        NS_LOG_DEBUG("Ch: " << m_isCh);
        NS_LOG_DEBUG("Mal: " << m_isMal);
        NS_LOG_DEBUG("Round: " << m_roundN);
        NS_LOG_DEBUG("Event: " << m_roundEventN);
        NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());
        NS_LOG_DEBUG("Packet Num: " << (uint32_t)tag.GetSeqNum());
        NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
        NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
        NS_LOG_DEBUG("------------------------");
        NS_LOG_DEBUG("");
    }


    void AltRouteNodeApplication::HandleRead(Ptr<Socket> socket){
        NS_LOG_FUNCTION(this << socket);

        Ptr<Packet> packet;
        Time delay;
        Address from;
        LeachTag tag;

        while ((packet = socket->RecvFrom(from))){
            Ipv4Address fromAddress = InetSocketAddress::ConvertFrom(from).GetIpv4();

            if(packet != nullptr){
                packet->PeekPacketTag(tag);

                uint8_t type = tag.GetPackType();

                delay = Simulator::Now() - tag.GetTimestamp();                 

                if(m_isCh){
                    NS_LOG_DEBUG("------CH RECEIVED------");
                    NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                    NS_LOG_DEBUG("Ip: " << m_localAddress);
                    NS_LOG_DEBUG("From: " << fromAddress);
                    NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());
                    NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
                    NS_LOG_DEBUG("Time Received: " << Simulator::Now().As(Time::S));
                    NS_LOG_DEBUG("Packet Num: " << (uint32_t)tag.GetSeqNum());
                    NS_LOG_DEBUG("Delay: " << delay);
                    NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
                    NS_LOG_DEBUG("------------------------");
                    NS_LOG_DEBUG("");
                }
                else{
                    NS_LOG_DEBUG("-----NODE RECEIVED-----");
                    NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                    NS_LOG_DEBUG("Ip: " << m_localAddress);
                    NS_LOG_DEBUG("From: " << fromAddress);
                    NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());
                    NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
                    NS_LOG_DEBUG("Time Received: " << Simulator::Now().As(Time::S));
                    NS_LOG_DEBUG("Packet Num: " << (uint32_t)tag.GetSeqNum());
                    NS_LOG_DEBUG("Delay: " << delay.As(Time::S));
                    NS_LOG_DEBUG("Shortest Delay: " << m_shortestDelay.As(Time::S));
                    NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
                    NS_LOG_DEBUG("------------------------");
                    NS_LOG_DEBUG("");
                }

                if(m_isCh && type == LeachTag::RE){
                    // Add received packet to larger packet
                    // AppendPack(packet);
                    
                    CHSend(packet);
                }
                else if(!m_isCh && type == LeachTag::RE){
                    LeachTag tag2;
                    
                    tag2.SetCHAddress(m_chAddress);
                    tag2.SetNodeAddress(tag.GetNodeAddress());
                    tag2.SetPackType(tag.GetPackType());
                    tag2.SetSeqNum(tag.GetSeqNum());
                    tag2.SetTimestamp(tag.GetTimestamp());
                    
                    packet->RemovePacketTag(tag);
                    packet->AddPacketTag(tag2);
                    
                    NS_LOG_DEBUG("--------ALT ROUTE-------");
                    NS_LOG_DEBUG(tag2.GetCHAddress());
                    NS_LOG_DEBUG("------------------------");
                    NS_LOG_DEBUG("");

                    Send(packet, m_chAddress);
                }
                else if(!m_isCh && type == LeachTag::AD && delay < m_shortestDelay){
                    if(std::find(m_blacklist.begin(), m_blacklist.end(), tag.GetNodeAddress()) == m_blacklist.end()){
                        m_chAddress = fromAddress;
                        m_shortestDelay = delay;

                        NS_LOG_DEBUG("---------CH SET---------");
                        NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                        NS_LOG_DEBUG("Ip: " << m_localAddress);
                        NS_LOG_DEBUG("CH: " << fromAddress);
                        NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
                        NS_LOG_DEBUG("Time Received: " << Simulator::Now().As(Time::S));
                        NS_LOG_DEBUG("------------------------");
                        NS_LOG_DEBUG("");
                    }
                    else{
                        NS_LOG_DEBUG("-------CH NOT SET-------");
                        NS_LOG_DEBUG("This node is blacklisted");
                        NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                        NS_LOG_DEBUG("Ip: " << m_localAddress);
                        NS_LOG_DEBUG("CH: " << fromAddress);
                        NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
                        NS_LOG_DEBUG("Time Received: " << Simulator::Now().As(Time::S));
                        NS_LOG_DEBUG("------------------------");
                        NS_LOG_DEBUG("");
                    }

                }
                else if(type == LeachTag::ADN){
                    NS_LOG_DEBUG("---NEIGHBOR NODE SET---");
                    NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                    NS_LOG_DEBUG("Ip: " << m_localAddress);
                    NS_LOG_DEBUG("CH: " << tag.GetNodeAddress());
                    NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
                    NS_LOG_DEBUG("Time Received: " << Simulator::Now().As(Time::S));
                    NS_LOG_DEBUG("------------------------");
                    NS_LOG_DEBUG("");
                    

                    if(std::find(m_nodes.begin(), m_nodes.end(), tag.GetNodeAddress()) == m_nodes.end()){
                        m_nodes.push_back(tag.GetNodeAddress());
                    }
                }
                else if(type == LeachTag::AK){
                    if(m_isCh && m_localAddress != tag.GetNodeAddress()){
                        CHSend(packet, tag.GetNodeAddress());
                    }
                    else{
                        NS_LOG_DEBUG("------ACK RECEIVED------");
                        NS_LOG_DEBUG(tag.GetNodeAddress());
                        NS_LOG_DEBUG("------------------------");
                        NS_LOG_DEBUG("");

                        m_ack = true;
                    }
                }
            }

            //ReportStatus();

            m_rxTrace(packet);
        }

    }


    bool AltRouteNodeApplication::CheckDead(){
        NS_LOG_FUNCTION(this);

        NS_LOG_DEBUG(m_energyModel);

        if(m_energyModel->GetCurrentState() == WifiPhyState::OFF){
            if(!m_dead){
                NS_LOG_DEBUG("--------NODE DEAD-------");
                NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                NS_LOG_DEBUG("Ip: " << m_localAddress);
                NS_LOG_DEBUG("Time: " << Simulator::Now().As(Time::S));
                NS_LOG_DEBUG("------------------------");
                NS_LOG_DEBUG("");

                m_dead = true;
                m_energyTrace(m_roundN);
            }
        }

        return m_dead;
    }


    void AltRouteNodeApplication::AppendPack(Ptr<Packet> packet){
        NS_LOG_FUNCTION(this);

        // m_agroPacket->AddAtEnd(packet);

        LeachTag tag;
        packet->PeekPacketTag(tag);


        NS_LOG_DEBUG("------APPEND PACKET-----");
        NS_LOG_DEBUG("Id: " << GetNode()->GetId());
        NS_LOG_DEBUG("Ip: " << m_localAddress);
        NS_LOG_DEBUG("From: local");
        NS_LOG_DEBUG("Time Sent: " << Simulator::Now().As(Time::S));
        NS_LOG_DEBUG("Agro Packet Size: " << m_agroPacket->GetSize());
        NS_LOG_DEBUG("------------------------");
        NS_LOG_DEBUG("");

        if(m_agroPacket->GetSize() >= m_maxPacketSize){
            //CHSend();
        }
    }


    // Getters & Setters
    TypeId AltRouteNodeApplication::GetTypeId(){
        static TypeId tid = TypeId("ns3::AltRouteNodeApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<AltRouteNodeApplication>()
            .AddAttribute("IsCh", "If node is cluster head", BooleanValue(false), 
                            MakeBooleanAccessor(&AltRouteNodeApplication::m_isCh), MakeBooleanChecker())
            .AddAttribute("IsMal", "If node is malicious", BooleanValue(false), 
                            MakeBooleanAccessor(&AltRouteNodeApplication::m_isMal), MakeBooleanChecker())

            .AddAttribute("RoundEvents", "Number of evnets in a round", IntegerValue(5), 
                            MakeDoubleAccessor(&AltRouteNodeApplication::m_roundEvents), MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Rx", "A packet has been received", MakeTraceSourceAccessor(&AltRouteNodeApplication::m_rxTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses", "A packet has been sent",
                            MakeTraceSourceAccessor(&AltRouteNodeApplication::m_rxTraceWithAddresses),
                            "Packet::TwoAddressTracedCallback")
            .AddTraceSource("Tx", "A packet has been sent", MakeTraceSourceAccessor(&AltRouteNodeApplication::m_txTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RemainingEnergy", "Energy change callback",
                            MakeTraceSourceAccessor(&AltRouteNodeApplication::m_energyTrace),
                            "TracedValueCallback::Double")
            .AddTraceSource("Status", "Status report callback",
                            MakeTraceSourceAccessor(&AltRouteNodeApplication::m_statusTrace),
                            "TracedValueCallback::Double");
        return tid;
    }


    void AltRouteNodeApplication::SetInterval(Time time){
        m_interval = time;
    }


    Time AltRouteNodeApplication::GetInterval(){
        return m_interval;
    }


    void AltRouteNodeApplication::SetIsCh(bool x){
        m_isCh = x;
    }


    bool AltRouteNodeApplication::GetIsCh(){
        return m_isCh;
    }


    void AltRouteNodeApplication::SetIsMal(bool x){
        m_isMal = x;
    }


    bool AltRouteNodeApplication::GetIsMal(){
        return m_isMal;
    }


    void AltRouteNodeApplication::SetEnergyModel(Ptr<DeviceEnergyModel> model){
        m_energyModel = DynamicCast<WifiRadioEnergyModel>(model);
    }


    Ptr<WifiRadioEnergyModel> AltRouteNodeApplication::GetEnergyModel(){
        return m_energyModel;
    }
} // ns3
