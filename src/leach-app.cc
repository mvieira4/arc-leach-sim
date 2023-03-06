#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/udp-socket.h"

#include "leach-app.h"


LeachNodeApplication::LeachNodeApplication(): m_lossCounter(0){
    m_socket = nullptr;
    m_sendEvent = ns3::EventId();
    m_interval = ns3::Seconds(1.0);
    m_sent = 0;
    m_count = 0;
    m_data = nullptr;
    m_dataSize = 0;
}

LeachNodeApplication::~LeachNodeApplication(){
    m_socket = nullptr;
    m_sendEvent = ns3::EventId();
}

void LeachNodeApplication::StartApplication(){
    NS_LOG_FUNCTION(this);
 
    if (!m_socket){
        ns3::TypeId tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = ns3::Socket::CreateSocket(GetNode(), tid);
        ns3::InetSocketAddress local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1){
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (ns3::addressUtils::IsMulticast(m_localAddress)){
            ns3::Ptr<ns3::UdpSocket> udpSocket = DynamicCast<ns3::UdpSocket>(m_socket);
            if (udpSocket){
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup(0, m_localAddress);
            }
            else{
                NS_FATAL_ERROR("Error: Failed to join multicast group");
            }
        }
    }
 
    if (!m_socket6){
        ns3::TypeId tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket6 = ns3::Socket::CreateSocket(GetNode(), tid);
        ns3::Inet6SocketAddress local6 = ns3::Inet6SocketAddress(ns3::Ipv6Address::GetAny(), m_port);
        if (m_socket6->Bind(local6) == -1){
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (ns3::addressUtils::IsMulticast(local6)){
            ns3::Ptr<ns3::UdpSocket> udpSocket = ns3::DynamicCast<ns3::UdpSocket>(m_socket6);
            if (udpSocket){
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup(0, local6);
            }
            else{
                NS_FATAL_ERROR("Error: Failed to join multicast group");
            }
        }
    }
 
    m_socket->SetRecvCallback(MakeCallback(&LeachNodeApplication::HandleRead, this));
    m_socket6->SetRecvCallback(MakeCallback(&LeachNodeApplication::HandleRead, this));
}

void LeachNodeApplication::StopApplication(){
    NS_LOG_FUNCTION(this);
 
    if (m_socket){
        m_socket->SetRecvCallback(ns3::MakeNullCallback<void, ns3::Ptr<ns3::Socket>>());
    }
}

ns3::TypeId LeachNodeApplication::GetTypeId(){
    ns3::TypeId tid = ns3::TypeId("ns3::UdpServer");
    tid.SetParent<Application>();
    tid.SetGroupName("Applications");
    tid.AddConstructor<ns3::UdpServer>();
    tid.AddAttribute("Port", "Port on which we listen for incoming packets.", ns3::UintegerValue(100),
                        MakeUintegerAccessor(&LeachNodeApplication::m_port), ns3::MakeUintegerChecker<uint16_t>());
    tid.AddTraceSource("Rx", "A packet has been received", MakeTraceSourceAccessor(&LeachNodeApplication::m_rxTrace),
                        "ns3::Packet::TracedCallback");
    tid.AddTraceSource("RxWithAddresses", "A packet has been received",
                        MakeTraceSourceAccessor(&LeachNodeApplication::m_rxTraceWithAddresses),
                        "ns3::Packet::TwoAddressTracedCallback");

    return tid;
}

void LeachNodeApplication::ScheduleTransmit(ns3::Time dt){
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = ns3::Simulator::Schedule(dt, &LeachNodeApplication::Send, this);
}

void LeachNodeApplication::Send(uint8_t data[]){
    NS_LOG_FUNCTION(this);
 
    NS_ASSERT(m_sendEvent.IsExpired());
 
    ns3::Ptr<ns3::Packet> packet;
    if (m_dataSize)
    {
        //
        // If m_dataSize is non-zero, we have a data buffer of the same size that we
        // are expected to copy and send.  This state of affairs is created if one of
        // the Fill functions is called.  In this case, m_size must have been set
        // to agree with m_dataSize
        //
        NS_ASSERT_MSG(m_dataSize == m_size,
                      "UdpEchoClient::Send(): m_size and m_dataSize inconsistent");
        NS_ASSERT_MSG(m_data, "UdpEchoClient::Send(): m_dataSize but no m_data");
        packet = ns3::Create<ns3::Packet>(m_data, m_dataSize);
    }
    else
    {
        //
        // If m_dataSize is zero, the client has indicated that it doesn't care
        // about the data itself either by specifying the data size by setting
        // the corresponding attribute or by not calling a SetFill function.  In
        // this case, we don't worry about it either.  But we do allow m_size
        // to have a value different from the (zero) m_dataSize.
        //
        packet = ns3::Create<ns3::Packet>(m_size);
    }
    ns3::Address localAddress;
    m_socket->GetSockName(localAddress);
    // call to the trace sinks before the packet is actually sent,
    // so that tags added to the packet can be sent as well
    m_txTrace(packet);
    if (ns3::Ipv4Address::IsMatchingType(m_targetAddress))
    {
        m_txTraceWithAddresses(packet, localAddress,
            ns3::InetSocketAddress(ns3::Ipv4Address::ConvertFrom(m_targetAddress), m_port));
    }
    else if (ns3::Ipv6Address::IsMatchingType(m_targetAddress))
    {
        m_txTraceWithAddresses(packet, localAddress,
                                ns3::Inet6SocketAddress(ns3::Ipv6Address::ConvertFrom(m_targetAddress), m_port));
    }
    m_socket->Send(packet);
    ++m_sent;
 
    if (ns3::Ipv4Address::IsMatchingType(m_targetAddress)){
        NS_LOG_INFO("At time " << ns3::Simulator::Now().As(ns3::Time::S) << " client sent " << m_size
                        << " bytes to " << ns3::Ipv4Address::ConvertFrom(m_targetAddress)
                        << " port " << m_port);
    }
    else if (ns3::Ipv6Address::IsMatchingType(m_targetAddress)){
        NS_LOG_INFO("At time " << ns3::Simulator::Now().As(ns3::Time::S) << " client sent " << m_size
                        << " bytes to " << ns3::Ipv6Address::ConvertFrom(m_targetAddress)
                        << " port " << m_port);
    }
    else if (ns3::InetSocketAddress::IsMatchingType(m_targetAddress)){
        NS_LOG_INFO("At time " << ns3::Simulator::Now().As(ns3::Time::S) << " client sent " << m_size << " bytes to "
                       << ns3::InetSocketAddress::ConvertFrom(m_targetAddress).GetIpv4() << " port "
                       << ns3::InetSocketAddress::ConvertFrom(m_targetAddress).GetPort());
    }
    else if (ns3::Inet6SocketAddress::IsMatchingType(m_targetAddress)){
        NS_LOG_INFO("At time " << ns3::Simulator::Now().As(ns3::Time::S) << " client sent " << m_size << " bytes to "
                       << ns3::Inet6SocketAddress::ConvertFrom(m_targetAddress).GetIpv6() << " port "
                       << ns3::Inet6SocketAddress::ConvertFrom(m_targetAddress).GetPort());
    }
 
    if (m_sent < m_count || m_count == 0){
        ScheduleTransmit(m_interval);
    }
}

void LeachNodeApplication::HandleRead(ns3::Ptr<ns3::Socket> socket){
    NS_LOG_FUNCTION(this << socket);
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;
    ns3::Address local;
    while ((packet = socket->RecvFrom(from))){
        socket->GetSockName(local);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, local);
        if (packet->GetSize() > 0){
            uint32_t receivedSize = packet->GetSize();
            ns3::SeqTsHeader seqTs;
            packet->RemoveHeader(seqTs);
            uint32_t currentSequenceNumber = seqTs.GetSeq();
            if (ns3::InetSocketAddress::IsMatchingType(from)){
                NS_LOG_INFO("TraceDelay: RX " << receivedSize << " bytes from "
                                << ns3::InetSocketAddress::ConvertFrom(from).GetIpv4()
                                << " Sequence Number: " << currentSequenceNumber
                                << " Uid: " << packet->GetUid() << " TXtime: "
                                << seqTs.GetTs() << " RXtime: " << ns3::Simulator::Now()
                                << " Delay: " << ns3::Simulator::Now() - seqTs.GetTs());
            }
            else if (ns3::Inet6SocketAddress::IsMatchingType(from)){
                NS_LOG_INFO("TraceDelay: RX " << receivedSize << " bytes from "
                                << ns3::Inet6SocketAddress::ConvertFrom(from).GetIpv6()
                                << " Sequence Number: " << currentSequenceNumber
                                << " Uid: " << packet->GetUid() << " TXtime: "
                                << seqTs.GetTs() << " RXtime: " << ns3::Simulator::Now()
                                << " Delay: " << ns3::Simulator::Now() - seqTs.GetTs());
            }
 
            m_lossCounter.NotifyReceived(currentSequenceNumber);
            m_received++;
        }
    }
}

void LeachNodeApplication::FindClusterHead(){
    // Find Closest Cluster Head 
}

void LeachNodeApplication::AdvertiseClusterHead(){
    // Broadcast to Surrounding Nodes
}

void LeachNodeApplication::ReportEvent(){
    // Send Regular Packet to Base
}
