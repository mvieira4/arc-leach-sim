/*
 *
 *  Author: Marcel Vieira
 *
 *  Description:
 *      Source file for ArcLeachSinkApplication class. This class implements LEACH protocol on a sink.
 *
 */


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
#include "ns3/wifi-net-device.h"

#include "ns3/arc-leach-sink-app.h"
#include "ns3/leach-tag-list.h"

namespace ns3{
    NS_LOG_COMPONENT_DEFINE("ArcLeachSinkApplication");
    
    ArcLeachSinkApplication::ArcLeachSinkApplication(): m_lossCounter(0){
        NS_LOG_FUNCTION(this << Simulator::Now().As(Time::S));

        m_socket = nullptr;
        m_sendEvent = EventId();

        m_sent = 0;
        m_received = 0;

        m_packetSize = 100;

        m_port = 50;
    }


    ArcLeachSinkApplication::~ArcLeachSinkApplication(){
        NS_LOG_FUNCTION(this << Simulator::Now().As(Time::S));

        m_socket = nullptr;
        m_sendEvent = EventId();
    }


    // Getters & Setters
    TypeId ArcLeachSinkApplication::GetTypeId(){
        static TypeId tid = TypeId("ns3::ArcLeachSinkApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<ArcLeachSinkApplication>()
            .AddAttribute("RoundEvents", "Number of evnets in a round", IntegerValue(5), 
                            MakeDoubleAccessor(&ArcLeachSinkApplication::m_roundEvents), MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Rx", "A packet has been received", MakeTraceSourceAccessor(&ArcLeachSinkApplication::m_rxTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses", "A packet has been sent",
                            MakeTraceSourceAccessor(&ArcLeachSinkApplication::m_rxTraceWithAddresses),
                            "Packet::TwoAddressTracedCallback")
            .AddTraceSource("Tx", "A packet has been sent", MakeTraceSourceAccessor(&ArcLeachSinkApplication::m_txTrace),
                            "Packet::TracedCallback");

        return tid;
    }

    // Methods
    void ArcLeachSinkApplication::DoDispose(){
        NS_LOG_FUNCTION(this << Simulator::Now().As(Time::S));

        Application::DoDispose();
    }


    void ArcLeachSinkApplication::StartApplication(){
        NS_LOG_FUNCTION(this << Simulator::Now().As(Time::S));

        Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
        m_localAddress = ipv4->GetAddress (1, 0).GetLocal();

        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        m_socket->SetAllowBroadcast(true);
        m_socket->BindToNetDevice(GetNode()->GetDevice(0)); 
        m_socket->Bind(InetSocketAddress(m_localAddress, m_port)); 
        m_socket->SetRecvCallback(MakeCallback(&ArcLeachSinkApplication::HandleRead, this));
        m_socket->Listen();

        Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
        udpSocket->MulticastJoinGroup (0, InetSocketAddress(m_localAddress, m_port));
    }


    void ArcLeachSinkApplication::StopApplication(){
        NS_LOG_FUNCTION(this << Simulator::Now().As(Time::S));
     
        if (m_socket){
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }


    void ArcLeachSinkApplication::ScheduleTransmit(Time dt, Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << dt);

        m_sendEvent = Simulator::Schedule(dt, &ArcLeachSinkApplication::Send, this, packet, address);
    }


    void ArcLeachSinkApplication::Send(Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << address);

        LeachTag tag;

        packet->PeekPacketTag(tag);

        m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));

        m_sent++;

        NS_LOG_DEBUG("--------SINK SENT-------");
        NS_LOG_DEBUG("Id: " << GetNode()->GetId());
        NS_LOG_DEBUG("Ip: " << m_localAddress);
        NS_LOG_DEBUG("To: " << address);
        NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());
        NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
        NS_LOG_DEBUG("Time Received: " << Simulator::Now().As(Time::S));
        NS_LOG_DEBUG("Packet Num: " << (uint32_t)tag.GetSeqNum());
        NS_LOG_DEBUG("Prev Packet Num: " << (uint32_t)m_packets[tag.GetNodeAddress()]);
        NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
        NS_LOG_DEBUG("------------------------");
        NS_LOG_DEBUG("");

        m_txTrace(packet);
    }


    void ArcLeachSinkApplication::HandleRead(Ptr<Socket> socket){
        NS_LOG_FUNCTION(this << socket);

        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        Time delay;

        LeachTag tag;

        while ((packet = socket->RecvFrom(from))){
            Ipv4Address fromAddress = InetSocketAddress::ConvertFrom(from).GetIpv4();

            packet->PeekPacketTag(tag);

            delay = Simulator::Now() - tag.GetTimestamp();

            Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice>(m_node->GetDevice(0));

            socket->GetSockName(localAddress);

            m_received++;

            NS_LOG_DEBUG("-----SINK RECEIVED-----");
            NS_LOG_DEBUG("Id: " << GetNode()->GetId());
            NS_LOG_DEBUG("Ip: " << m_localAddress);
            NS_LOG_DEBUG("From: " << fromAddress);
            NS_LOG_DEBUG("Packet Type: " << (uint32_t)tag.GetPackType());
            NS_LOG_DEBUG("Time Sent: " << tag.GetTimestamp().As(Time::S));
            NS_LOG_DEBUG("Time Received: " << Simulator::Now().As(Time::S));
            NS_LOG_DEBUG("Packet Num: " << (uint32_t)tag.GetSeqNum());
            NS_LOG_DEBUG("Prev Packet Num: " << (uint32_t)m_packets[tag.GetNodeAddress()]);
            NS_LOG_DEBUG("Pack Address: " << tag.GetNodeAddress());
            NS_LOG_DEBUG("Delay: " << delay.As(Time::S));
            NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");

            m_rxTrace(packet);

            if(tag.GetPackType() == LeachTag::ARE || tag.GetPackType() == LeachTag::RE){

                if((m_packets.count(tag.GetNodeAddress()) && m_packets[tag.GetNodeAddress()] != tag.GetSeqNum() - 1) 
                        || (m_packets.empty() && tag.GetSeqNum() == 1)){

                    if(m_chStrikes.count(tag.GetCHPrevAddress())){
                        LeachTag tag2;
                        tag2.SetPackType(LeachTag::BK);
                        tag2.SetNodeAddress(tag.GetCHPrevAddress());
                        tag2.SetTimestamp(Simulator::Now());

                        packet = Create<Packet>(m_packetSize);
                        packet->AddPacketTag(tag2);

                        if(tag.GetCHAddress() != m_localAddress){
                            Send(packet, tag.GetCHAddress());
                        }
                        else{
                            Send(packet, tag.GetNodeAddress());
                        }
                    }
                    else{
                        m_chStrikes[tag.GetCHPrevAddress()] = tag.GetNodeAddress(); 
                    }
                }

                m_packets[tag.GetNodeAddress()] = tag.GetSeqNum();
            }
        }
    }
} // ns3
