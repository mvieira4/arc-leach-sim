/*
 *
 *  Author: Marcel Vieira
 *
 *  Description:
 *      Source file for LeachSinkApplication class. This class implements LEACH protocol on a sink.
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

#include "leach-sink-app.h"


namespace ns3{
    NS_LOG_COMPONENT_DEFINE("LeachSinkApplication");
    
    LeachSinkApplication::LeachSinkApplication(): m_lossCounter(0){
        NS_LOG_FUNCTION(this);

        m_socket = nullptr;
        m_sendEvent = EventId();

        m_sent = 0;
        m_received = 0;

        m_packetSize = 1024;

        m_port = 50;
    }


    LeachSinkApplication::~LeachSinkApplication(){
        NS_LOG_FUNCTION(this);

        m_socket = nullptr;
        m_sendEvent = EventId();
    }


    // Getters & Setters
    TypeId LeachSinkApplication::GetTypeId(){
        static TypeId tid = TypeId("ns3::LeachSinkApplication")

            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<LeachSinkApplication>()
            .AddAttribute("RoundEvents", "Number of evnets in a round", IntegerValue(5), 
                            MakeDoubleAccessor(&LeachSinkApplication::m_roundEvents), MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Rx", "A packet has been received", MakeTraceSourceAccessor(&LeachSinkApplication::m_rxTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses", "A packet has been sent",
                            MakeTraceSourceAccessor(&LeachSinkApplication::m_rxTraceWithAddresses),
                            "Packet::TwoAddressTracedCallback")
            .AddTraceSource("Tx", "A packet has been sent", MakeTraceSourceAccessor(&LeachSinkApplication::m_txTrace),
                            "Packet::TracedCallback");

        return tid;
    }

    // Methods
    void LeachSinkApplication::DoDispose(){
        NS_LOG_FUNCTION(this);

        Application::DoDispose();
    }


    void LeachSinkApplication::StartApplication(){
        NS_LOG_FUNCTION(this);

        Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
        m_localAddress = ipv4->GetAddress (1, 0).GetLocal();

        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        m_socket->SetAllowBroadcast(true);
        m_socket->BindToNetDevice(GetNode()->GetDevice(0)); 
        m_socket->Bind(InetSocketAddress(m_localAddress, m_port)); 
        m_socket->SetRecvCallback(MakeCallback(&LeachSinkApplication::HandleRead, this));
        m_socket->Listen();

        Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
        udpSocket->MulticastJoinGroup (0, InetSocketAddress(m_localAddress, m_port));
    }


    void LeachSinkApplication::StopApplication(){
        NS_LOG_FUNCTION(this);
     
        if (m_socket){
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }





    void LeachSinkApplication::ScheduleTransmit(Time dt, Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << dt);

        m_sendEvent = Simulator::Schedule(dt, &LeachSinkApplication::Send, this, packet, address);
    }



    void LeachSinkApplication::Send(Ptr<Packet> packet, Ipv4Address address){
        NS_LOG_FUNCTION(this << address);

        NS_LOG_DEBUG("----------STATE---------");
        NS_LOG_DEBUG("State: " << m_node->GetObject<WifiPhy>()->GetState());
        NS_LOG_DEBUG("------------------------");
        NS_LOG_DEBUG("");

        m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));

        m_sent++;

        NS_LOG_DEBUG("-------PACKET SENT------");
        NS_LOG_DEBUG("Round: " << m_executedRounds);
        NS_LOG_DEBUG("From ip " << m_localAddress << " at time " 
                            << Simulator::Now().As(Time::S) << " to "
                            << address);
        NS_LOG_DEBUG("Packet Size: " << packet->GetSize());
        NS_LOG_DEBUG("Sent Packets: " << m_sent);
        NS_LOG_DEBUG("------------------------");
        NS_LOG_DEBUG("");

        m_txTrace(packet);
        m_rxTraceWithAddresses(packet, m_localAddress, address);
    }



    void LeachSinkApplication::HandleRead(Ptr<Socket> socket){
        NS_LOG_FUNCTION(this << socket);

        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        DeviceNameTag tag;

        while ((packet = socket->RecvFrom(from))){

            Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice>(m_node->GetDevice(0));

            NS_LOG_DEBUG("----------STATE---------");
            NS_LOG_DEBUG("State: " << wifiNetDevice->GetPhy()->GetState()->GetState());
            NS_LOG_DEBUG("------------------------");
            NS_LOG_DEBUG("");

            socket->GetSockName(localAddress);

            m_received++;

            NS_LOG_DEBUG("----PACKET RECEIVED----");
            NS_LOG_DEBUG("Recv at sink ip " << m_localAddress << " at time " 
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
            }

            m_rxTrace(packet);
            m_rxTraceWithAddresses(packet, from, localAddress);
        }
    }

} // ns3
