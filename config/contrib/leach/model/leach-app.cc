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

#include "ns3/leach-app.h"
#include "ns3/info-id-tag.h"
#include "ns3/timestamp-tag.h"
#include "ns3/seq-num-tag.h"
#include "ns3/leach-tag-list.h"

#include <cmath>
#include <random>

namespace ns3{
    NS_LOG_COMPONENT_DEFINE("LeachNodeApplication");
    
    // Constructor
    LeachNodeApplication::LeachNodeApplication(): m_lossCounter(0){
        NS_LOG_FUNCTION(this << Simulator::Now());

        // Time between events
        m_interval = MilliSeconds(700);

        // Event values per round
        m_roundEventN = 0;
        m_roundEvents = 75; 

        // Round values
        m_roundN = 0;
        m_rounds = 50;

        // Port number 
        m_port = 50;
        
        // Default packet size
        m_packetSize = 1024;

        // Max aggrogated packet size
        m_maxPacketSize = 0;

        // Maximum number of nodes to use
        m_maxNodes = 40;

        // Set sequence number to 0;
        m_seqNum = 1;
    }


    // Deconstructor
    LeachNodeApplication::~LeachNodeApplication(){
        NS_LOG_FUNCTION(this << Simulator::Now());

        // Clear aggrogated packet
        m_agroPacket = nullptr;

        // Clear socket
        m_socket = nullptr;

        // Clear energy model
        m_energyModel = nullptr;
    }


    // Destroys application 
    void LeachNodeApplication::DoDispose(){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << Simulator::Now().As(Time::S));

        Application::DoDispose();
    }


    // Initilize application
    void LeachNodeApplication::StartApplication(){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << Simulator::Now().As(Time::S));

        // Create a random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
          
        // Define the range of the random number
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        // Node states 
        m_dead = false; 

        double random_double = dis(gen);
        //m_isMal = random_double < 0.25 * 0.33;
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
        m_socket->SetRecvCallback(MakeCallback(&LeachNodeApplication::HandleRead, this));
        m_socket->Listen();

        // Start application based on node id
        NS_LOG_DEBUG("Start: " << MilliSeconds(GetNode()->GetId() * m_interval.GetMilliSeconds()).As(Time::S));


        // Start status collection
        ReportStatus();

        // Start first round
        ScheduleNextRound(MilliSeconds(GetNode()->GetId() * m_interval.GetMilliSeconds()));
    }


    // Terminate application
    void LeachNodeApplication::StopApplication(){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << Simulator::Now().As(Time::S));
     
        // Disable receive callback 
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }


    void LeachNodeApplication::ReportStatus(){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << Simulator::Now().As(Time::S));

        m_statusTrace(m_roundN);

        if(!m_dead && m_roundN < m_rounds){
            NS_LOG_DEBUG("Round: " << m_roundN);
            Simulator::Schedule(Seconds(20), &LeachNodeApplication::ReportStatus, this);
        }
    }


    // Schedule round
    void LeachNodeApplication::ScheduleNextRound(Time dt){
        NS_LOG_FUNCTION(this << GetNode()->GetId());

        // Exit if node dead
        if(CheckDead()){
            return;
        }

        // Schedule next round unless target number reached
        if(m_roundN < m_rounds){
            Simulator::Schedule(dt, &LeachNodeApplication::ExecuteRound, this);
        }
    }


    // Run the round
    void LeachNodeApplication::ExecuteRound(){
        NS_LOG_FUNCTION(this << GetNode()->GetId());

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

        // Reset aggrogated packet
        m_agroPacket = Create<Packet>(0);

        ScheduleAdvertise(Seconds(0));
    }


    void LeachNodeApplication::ScheduleAdvertise(Time dt){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << m_advertisments);

        // Get wifi channel
        Ptr<Channel> channel = GetNode()->GetDevice(0)->GetChannel();


        if(m_advertisments < channel->GetNDevices()){
            Simulator::Schedule(dt, &LeachNodeApplication::Advertise, this);
        }
        else{
            ScheduleNextEvent(dt);
        }
    }


    void LeachNodeApplication::Advertise(){
        NS_LOG_FUNCTION(this << GetNode()->GetId());
        
        if(m_isCh){
            LeachTag tag;
            tag.SetPackType(LeachTag::AD);
            tag.SetTimestamp(Simulator::Now());
            tag.SetNodeAddress(m_localAddress);
            tag.SetCHAddress(m_chAddress);
            tag.SetCHPrevAddress(m_chPrevAddress);

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
        }

        m_advertisments++;

        ScheduleAdvertise(MilliSeconds(40 * m_interval.GetMilliSeconds()));
    }


    void LeachNodeApplication::ScheduleNextEvent(Time dt){
        NS_LOG_FUNCTION(this << GetNode()->GetId() << m_roundEventN << m_roundEvents);

        if(m_roundEventN < m_roundEvents){
            Simulator::Schedule(dt, &LeachNodeApplication::ReportEvent, this);
        }
        else{
            m_roundN++;
            ScheduleNextRound(40 * m_interval);
            //Simulator::Schedule(dt + m_interval, &LeachNodeApplication::CHSend, this, true);
        }
    }


    void LeachNodeApplication::ReportEvent(){
        NS_LOG_FUNCTION(this << GetNode()->GetId());

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

        m_txTrace(packet);

        Send(packet, m_chAddress);
    }


    void LeachNodeApplication::Send(Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << address);

        if(CheckDead()){
            return;
        }

        if(!m_isCh && !m_isMal){
            m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));

            LeachTag tag;
            packet->PeekPacketTag(tag);

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
        else if(m_isCh){
            //AppendPack(packet);
            CHSend(packet);
        }

        m_seqNum++;

        m_roundEventN++;

        ScheduleNextEvent(m_maxNodes * m_interval);
    }

    void LeachNodeApplication::CHSend(Ptr<Packet> packet){
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

        m_agroPacket = Create<Packet>(0);

        //if(last){
        //    m_roundN++;
        //    ScheduleNextRound(m_interval);
        //}
    }


    void LeachNodeApplication::HandleRead(Ptr<Socket> socket){
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
                else if(!m_isCh && type == LeachTag::AD && delay < m_shortestDelay){

                    NS_LOG_DEBUG("---------CH SET---------");
                    NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                    NS_LOG_DEBUG("Ip: " << m_localAddress);
                    NS_LOG_DEBUG("CH: " << fromAddress);
                    NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
                    NS_LOG_DEBUG("Time Received: " << Simulator::Now().As(Time::S));
                    NS_LOG_DEBUG("------------------------");
                    NS_LOG_DEBUG("");

                    m_chAddress = fromAddress;
                    m_shortestDelay = delay;
                }
            }

            m_rxTrace(packet);
        }

    }


    bool LeachNodeApplication::CheckDead(){
        if(m_energyModel->GetCurrentState() == WifiPhyState::OFF){
            if(!m_dead){
                NS_LOG_DEBUG("--------NODE DEAD-------");
                NS_LOG_DEBUG("Id: " << GetNode()->GetId());
                NS_LOG_DEBUG("Ip: " << m_localAddress);
                NS_LOG_DEBUG("Time: " << Simulator::Now().As(Time::S));
                NS_LOG_DEBUG("------------------------");
                NS_LOG_DEBUG("");

                m_dead = true;
                m_energyTrace();
            }
        }

        return m_dead;
    }


    void LeachNodeApplication::AppendPack(Ptr<Packet> packet){
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
    TypeId LeachNodeApplication::GetTypeId(){
        static TypeId tid = TypeId("ns3::LeachNodeApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<LeachNodeApplication>()
            .AddAttribute("IsCh", "If node is cluster head", BooleanValue(false), 
                            MakeBooleanAccessor(&LeachNodeApplication::m_isCh), MakeBooleanChecker())
            .AddAttribute("IsMal", "If node is malicious", BooleanValue(false), 
                            MakeBooleanAccessor(&LeachNodeApplication::m_isMal), MakeBooleanChecker())
            .AddAttribute("RoundEvents", "Number of evnets in a round", IntegerValue(5), 
                            MakeDoubleAccessor(&LeachNodeApplication::m_roundEvents), MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Rx", "A packet has been received", MakeTraceSourceAccessor(&LeachNodeApplication::m_rxTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses", "A packet has been sent",
                            MakeTraceSourceAccessor(&LeachNodeApplication::m_rxTraceWithAddresses),
                            "Packet::TwoAddressTracedCallback")
            .AddTraceSource("Tx", "A packet has been sent", MakeTraceSourceAccessor(&LeachNodeApplication::m_txTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RemainingEnergy", "Energy change callback",
                            MakeTraceSourceAccessor(&LeachNodeApplication::m_energyTrace),
                            "TracedValueCallback::Double")
            .AddTraceSource("Status", "Status report callback",
                            MakeTraceSourceAccessor(&LeachNodeApplication::m_statusTrace),
                            "TracedValueCallback::Double");
        return tid;
    }


    void LeachNodeApplication::SetInterval(Time time){
        m_interval = time;
    }


    Time LeachNodeApplication::GetInterval(){
        return m_interval;
    }


    void LeachNodeApplication::SetIsCh(bool x){
        m_isCh = x;
    }


    bool LeachNodeApplication::GetIsCh(){
        return m_isCh;
    }


    void LeachNodeApplication::SetIsMal(bool x){
        m_isMal = x;
    }


    bool LeachNodeApplication::GetIsMal(){
        return m_isMal;
    }


    void LeachNodeApplication::SetEnergyModel(Ptr<DeviceEnergyModel> model){
        m_energyModel = DynamicCast<WifiRadioEnergyModel>(model);
    }


    Ptr<WifiRadioEnergyModel> LeachNodeApplication::GetEnergyModel(){
        return m_energyModel;
    }
} // ns3
