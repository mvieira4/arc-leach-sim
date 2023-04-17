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


#include "leach-app.h"

namespace ns3{
    NS_LOG_COMPONENT_DEFINE("LeachNodeApplication");
    
    // Constructor
    LeachNodeApplication::LeachNodeApplication(): m_lossCounter(0){
        NS_LOG_FUNCTION(this);

        // Node states 
        m_dead = false; 
        m_isMal = false;
        m_isCh = true;

        // Time between events
        m_interval = MilliSeconds(1000);

        // Event values per round
        m_roundEventN = 0;
        m_roundEvents = 2;

        // Round values
        m_roundN = 0;
        m_rounds = 3;

        // Socket config 
        m_socket = nullptr;
        m_port = 50;

        // Default packet size
        m_packetSize = 1024;

        // Empty agrogated packet for Ch
        m_agroPacket = nullptr;

        // Local packet stats
        m_sent = 0;
        m_received = 0;
    }


    // Deconstructor
    LeachNodeApplication::~LeachNodeApplication(){
        NS_LOG_FUNCTION(this);

        // Clear socket
        m_socket = nullptr;

        // Clear aggrogated packet
        m_agroPacket = nullptr;
    }


    // Getters & Setters


    TypeId LeachNodeApplication::GetTypeId(){
        static TypeId tid = TypeId("ns3::LeachNodeApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<LeachNodeApplication>()
            .AddAttribute("IsCh", "If node is cluster head", BooleanValue(false), 
                            MakeDoubleAccessor(&LeachNodeApplication::m_isCh), MakeBooleanChecker())
            .AddAttribute("IsMal", "If node is malicious", BooleanValue(false), 
                            MakeDoubleAccessor(&LeachNodeApplication::m_isMal), MakeBooleanChecker())
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


    // Destroys application 
    void LeachNodeApplication::DoDispose(){
        NS_LOG_FUNCTION(this);

        Application::DoDispose();
    }


    // Initilize application
    void LeachNodeApplication::StartApplication(){
        NS_LOG_FUNCTION(this);

        // Set important ipv4 addresses 
        Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
        m_localAddress = ipv4->GetAddress (1, 0).GetLocal();
        m_sinkAddress = Ipv4Address("10.0.0.1");
        m_chAddress = m_sinkAddress;

        // Initilize aggrogated packet
        m_agroPacket = Create<Packet>();

        // Create & config udp socket
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        m_socket->SetAllowBroadcast(true);
        m_socket->BindToNetDevice(GetNode()->GetDevice(0)); 
        m_socket->Bind(InetSocketAddress(m_localAddress, m_port)); 
        m_socket->SetRecvCallback(MakeCallback(&LeachNodeApplication::HandleRead, this));
        m_socket->Listen();

        // Start application based on node id
        ScheduleNextRound(MilliSeconds(GetNode()->GetId() * 30));
        NS_LOG_DEBUG("Start: " << GetNode()->GetId() * 30);
    }

    // Terminate application
    void LeachNodeApplication::StopApplication(){
        NS_LOG_FUNCTION(this);
     
        // Disable receive callback 
        if (m_socket){
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }


    // Schedule round
    void LeachNodeApplication::ScheduleNextRound(Time dt){
        NS_LOG_FUNCTION(this);

        // Exit if node dead
        if(m_energyModel->GetCurrentState() == WifiPhyState::OFF){
            return;
        }

        // Schedule next round unless target number reached
        if(m_roundN < m_rounds){
            // Schedule round
            m_roundEvent = Simulator::Schedule(dt, &LeachNodeApplication::ExecuteRound, this);

            // Reset number of execute events
            m_roundEventN = 0;
        }
    }


    // Run the round
    void LeachNodeApplication::ExecuteRound(){
        NS_LOG_FUNCTION(this);

        // Advertise is Ch
        if(m_isCh){
            //ScheduleAdvertise(Seconds(0));
        }

        // Set begining of execution phase
        ScheduleNextEvent(MilliSeconds(600));

        m_roundN++;
    }

    // 
    void LeachNodeApplication::ScheduleAdvertise(Time dt){
        NS_LOG_FUNCTION(this);

        Simulator::Schedule(Seconds(0), &LeachNodeApplication::Advertise, this);
    }


    void LeachNodeApplication::Advertise(){
        NS_LOG_FUNCTION(this);

        // Create & config packet tag
        DeviceNameTag typeTag;
        typeTag.SetDeviceName("AD");


        Ptr<PacketMetadata> metadata = CreateObject<PacketMetadata>();

        // Create & config packet
        Ptr<Packet> packet;
        packet = Create<Packet>(m_packetSize);
        packet->AddPacketTag(typeTag);

        // Get wifi channel
        Ptr<Channel> channel = GetNode()->GetDevice(0)->GetChannel();

        // Send message to all nodes on channel 
        for (uint32_t i = 0; i < channel->GetNDevices(); i++){
            Ptr<Ipv4> ipv4= channel->GetDevice(i)->GetNode()->GetObject<Ipv4>();
            Ipv4Address address = ipv4->GetAddress (1, 0).GetLocal();

            if(address != m_localAddress){
                // Schedule advertise to prevent collisions
                ScheduleTransmit(MilliSeconds(30*i*GetNode()->GetId()), packet, address);
            }
        }
    }

    void LeachNodeApplication::ScheduleNextEvent(Time dt){
        NS_LOG_FUNCTION(this << dt);

        m_received = 0;

        if(m_roundEventN < m_roundEvents){
            m_roundEvent = Simulator::Schedule(dt, &LeachNodeApplication::ReportEvent, this);
        }
        else{
            ScheduleTransmit(Seconds(0), m_agroPacket, m_sinkAddress);
            ScheduleNextRound(m_interval);
            m_chAddress = m_sinkAddress;
        }
    }

    void LeachNodeApplication::ReportEvent(){
        NS_LOG_FUNCTION(this);

        Ipv4Address address = m_chAddress;

        DeviceNameTag tag;
        tag.SetDeviceName("RE");

        Ptr<Packet> packet;
        packet = Create<Packet>(m_packetSize);
        packet->AddPacketTag(tag);

        ScheduleTransmit(Seconds(0), packet, address);

        if(!m_isCh){
            ScheduleTransmit(Seconds(0), packet, address);
        }
        else{
            m_agroPacket->AddAtEnd(packet);
            NS_LOG_DEBUG("Append to packet");
            NS_LOG_DEBUG("");
        }


        ScheduleNextEvent(m_interval);

        m_roundEventN++;

        m_statusTrace();
    }

    void LeachNodeApplication::ScheduleTransmit(Time dt, Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << dt);

        m_sendEvent = Simulator::Schedule(dt, &LeachNodeApplication::Send, this, packet, address);
    }



    void LeachNodeApplication::Send(Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << address);

        NS_LOG_DEBUG("----------STATE---------");
        NS_LOG_DEBUG("State: " << m_energyModel->GetCurrentState());
        NS_LOG_DEBUG("------------------------");
        NS_LOG_DEBUG("");

        if(m_energyModel->GetCurrentState() == WifiPhyState::OFF){
            if(!m_dead){
                NS_LOG_DEBUG("State: OFF");
                NS_LOG_DEBUG("");
                m_dead = true;
                m_energyTrace();
            }

            return;
        }

        if(!m_isMal){
            m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));

            DeviceNameTag tag;
            packet->PeekPacketTag(tag);

            NS_LOG_DEBUG("-------PACKET SENT------");
            NS_LOG_DEBUG("Id: " << GetNode()->GetId());
            NS_LOG_DEBUG("Ch: " << m_isCh);
            NS_LOG_DEBUG("Mal: " << m_isMal);
            NS_LOG_DEBUG("Round: " << m_roundN);
            NS_LOG_DEBUG("From ip " << m_localAddress << " at time " 
                                << Simulator::Now().As(Time::S) << " to "
                                << address);
            NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
            NS_LOG_DEBUG("Packet Tag: " << tag.GetDeviceName());
            NS_LOG_DEBUG("Sent Packets: " << m_sent);
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");

            m_sent++;

            m_txTrace(packet);
            m_rxTraceWithAddresses(packet, m_localAddress, address);
        }

    }



    void LeachNodeApplication::HandleRead(Ptr<Socket> socket){
        NS_LOG_FUNCTION(this << socket);

        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        DeviceNameTag tag;

        Ipv4Address fromAddress = InetSocketAddress::ConvertFrom(from).GetIpv4();


        while ((packet = socket->RecvFrom(from))){
            NS_LOG_DEBUG("----------STATE---------");
            NS_LOG_DEBUG("State: " << m_energyModel->GetCurrentState());
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");

            socket->GetSockName(localAddress);

            NS_LOG_DEBUG("----PACKET RECEIVED----");
            NS_LOG_DEBUG("Recv at node ip " << m_localAddress << " at time " 
                                << Simulator::Now().As(Time::S) << " from "
                                << InetSocketAddress::ConvertFrom(from).GetIpv4());
            NS_LOG_DEBUG("Received Packets: " << m_received);
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");


            if(packet != nullptr){
                packet->PeekPacketTag(tag);
                NS_LOG_DEBUG("-------PACKET TAG-------");
                NS_LOG_DEBUG("Packet Tag: " << tag.GetDeviceName());
                NS_LOG_DEBUG("From: " << from);
                NS_LOG_DEBUG("------------------------");
                NS_LOG_DEBUG("");

                std::string name = tag.GetDeviceName();

                if(m_isCh && name == "RE"){
                    m_agroPacket->AddAtEnd(packet);
                    NS_LOG_DEBUG("Append to packet");
                    NS_LOG_DEBUG("");
                }
                else if(!m_isCh && name == "AD" && m_localAddress != m_chAddress){
                    m_chAddress = fromAddress;
                    NS_LOG_DEBUG("Set cluster head");
                    NS_LOG_DEBUG("");
                }
            }

            m_rxTrace(packet);
            m_rxTraceWithAddresses(packet, from, localAddress);

            m_received++;
        }
    }

} // ns3
