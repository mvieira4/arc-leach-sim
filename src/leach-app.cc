#include "ns3/address-utils.h"
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
    
    LeachNodeApplication::LeachNodeApplication(bool isCh, bool isMal): m_lossCounter(0){
        NS_LOG_FUNCTION(this);

        m_isCh = isCh;
        m_isMal = isMal;
        
        m_dead = false; 

        m_socket = nullptr;
        m_sendEvent = EventId();
        m_interval = Seconds(0.4);

        m_sent = 0;
        m_received = 0;

        m_roundEventN = 0;
        m_roundEvents = 50;

        m_roundN = 0;
        m_rounds = 100;

        m_executedRounds = 0;

        m_packetSize = 1024;

        m_port = 50;

        m_agroPacket = nullptr;
    }


    LeachNodeApplication::~LeachNodeApplication(){
        NS_LOG_FUNCTION(this);


        m_socket = nullptr;
        m_sendEvent = EventId();

        m_agroPacket = nullptr;
    }


    // Getters & Setters
    TypeId LeachNodeApplication::GetTypeId(){
        static TypeId tid = TypeId("ns3::LeachNodeApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<LeachNodeApplication>()
            .AddAttribute("IsCh", "If node is cluster head", BooleanValue(true), 
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


    // Methods
    void LeachNodeApplication::DoDispose(){
        NS_LOG_FUNCTION(this);

        Application::DoDispose();
    }


    void LeachNodeApplication::StartApplication(){
        NS_LOG_FUNCTION(this);

        Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
        m_localAddress = ipv4->GetAddress (1, 0).GetLocal();
        m_chAddress = m_localAddress;

        m_agroPacket = Create<Packet>();

        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        m_socket->SetAllowBroadcast(true);
        m_socket->BindToNetDevice(GetNode()->GetDevice(0)); 
        m_socket->Bind(InetSocketAddress(m_localAddress, m_port)); 
        m_socket->SetRecvCallback(MakeCallback(&LeachNodeApplication::HandleRead, this));
        m_socket->Listen();

        Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
        udpSocket->MulticastJoinGroup (0, InetSocketAddress(m_localAddress, m_port));

        ScheduleNextRound(Seconds(0));
    }


    void LeachNodeApplication::StopApplication(){
        NS_LOG_FUNCTION(this);
     
        if (m_socket){
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }


    void LeachNodeApplication::ScheduleAdvertise(Time dt){
        NS_LOG_FUNCTION(this);

        Simulator::Schedule(Seconds(0), &LeachNodeApplication::Advertise, this);
    }


    void LeachNodeApplication::Advertise(){
        NS_LOG_FUNCTION(this);

        DeviceNameTag tag;
        tag.SetDeviceName("AD");

        Ptr<Packet> packet;
        packet = Create<Packet>(m_packetSize);
        //packet->AddPacketTag(tag);

        Ptr<Channel> channel = GetNode()->GetDevice(0)->GetChannel();

        for (uint32_t i = 0; i < channel->GetNDevices(); i++){
            Ptr<Ipv4> ipv4= channel->GetDevice(i)->GetNode()->GetObject<Ipv4>();
            Ipv4Address address = ipv4->GetAddress (1, 0).GetLocal();

            if(address != m_localAddress){
                ScheduleTransmit(MilliSeconds(20*i), packet, address);
            }
        }
    }



    void LeachNodeApplication::ScheduleNextRound(Time dt){
        NS_LOG_FUNCTION(this);

        if(m_energyModel->GetCurrentState() == WifiPhyState::OFF){
            return;
        }

        if(m_roundN < m_rounds){
            m_roundEvent = Simulator::Schedule(dt, &LeachNodeApplication::ExecuteRound, this);

        }

        m_roundN++;
    }


    void LeachNodeApplication::ExecuteRound(){
        NS_LOG_FUNCTION(this);

        m_executedRounds++;
    
        if(m_isCh){
            ScheduleAdvertise(Seconds(0));
        }


        ScheduleNextEvent(Seconds(0));

        ScheduleNextRound(m_interval);

        m_chAddress = m_localAddress;
    }


    void LeachNodeApplication::ScheduleNextEvent(Time dt){
        NS_LOG_FUNCTION(this << dt);

        m_received = 0;

        if(m_energyModel->GetCurrentState() == WifiPhyState::OFF){
            return;
        }

        if(m_roundEventN < m_roundEvents){
            m_roundEvent = Simulator::Schedule(dt, &LeachNodeApplication::ReportEvent, this);
        }

        m_roundEventN++;
    }

    void LeachNodeApplication::ReportEvent(){
        NS_LOG_FUNCTION(this);

        Ipv4Address address = m_chAddress;

        DeviceNameTag tag;
        tag.SetDeviceName("RE");

        Ptr<Packet> packet;
        packet = Create<Packet>(m_packetSize);
        packet->AddPacketTag(tag);

        ScheduleTransmit(m_interval, packet, address);

        ScheduleNextEvent(m_interval);

    }

    void LeachNodeApplication::ScheduleTransmit(Time dt, Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << dt);

        m_sendEvent = Simulator::Schedule(dt, &LeachNodeApplication::Send, this, packet, address);
    }



    void LeachNodeApplication::Send(Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << address);

        if(m_energyModel->GetCurrentState() == WifiPhyState::OFF){
            if(!m_dead){
                NS_LOG_DEBUG("State: OFF");
                NS_LOG_DEBUG("");
                m_dead = true;
                m_energyTrace();
            }

            return;
        }

        m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));

        m_sent++;

        m_txTrace(packet);
        m_rxTraceWithAddresses(packet, m_localAddress, address);

        NS_LOG_DEBUG("-------PACKET SENT------");
        NS_LOG_DEBUG("Round: " << m_executedRounds);
        NS_LOG_DEBUG("From ip " << m_localAddress << " at time " 
                            << Simulator::Now().As(Time::S) << " to "
                            << address);
        NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
        NS_LOG_DEBUG("Sent Packets: " << m_sent);
        NS_LOG_DEBUG("------------------------");
        NS_LOG_DEBUG("");
    }



    void LeachNodeApplication::HandleRead(Ptr<Socket> socket){
        NS_LOG_FUNCTION(this << socket);

        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        DeviceNameTag tag;

        while ((packet = socket->RecvFrom(from))){

            socket->GetSockName(localAddress);

            m_received++;


            m_rxTrace(packet);
            m_rxTraceWithAddresses(packet, from, localAddress);

            NS_LOG_DEBUG("----PACKET RECEIVED----");
            NS_LOG_DEBUG("Recv at ip " << m_localAddress << " at time " 
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
                    NS_LOG_DEBUG("Append to pakcet");
                    NS_LOG_DEBUG("");
                }
                else if(!m_isCh && name == "AD" && m_localAddress == m_chAddress){
                    m_chAddress = InetSocketAddress::ConvertFrom(from).GetIpv4();
                    NS_LOG_DEBUG("Set cluster head");
                    NS_LOG_DEBUG("");
                }
            }

        }
    }

} // ns3
