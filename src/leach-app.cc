#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/udp-socket.h"

#include "leach-app.h"

namespace ns3{
    NS_LOG_COMPONENT_DEFINE("LeachNodeApplication");

    LeachNodeApplication::LeachNodeApplication(): m_lossCounter(0){
        NS_LOG_FUNCTION(this);

        m_socket = nullptr;
        m_socket6 = nullptr;
        m_sendEvent = EventId();
        m_interval = Seconds(1.0);
        m_sent = 0;
        m_count = 0;
        m_data = nullptr;
        m_dataSize = 0;
    }

    LeachNodeApplication::~LeachNodeApplication(){
        NS_LOG_FUNCTION(this);

        m_socket = nullptr;
        m_socket6 = nullptr;
        m_sendEvent = EventId();

        delete[] m_data;
        m_data = nullptr;
        m_dataSize = 0;
    }

    void LeachNodeApplication::SetInterval(Time time){
        m_interval = time;
    }

    void LeachNodeApplication::SetChProb(double prob){
        m_chProb = prob;
    }

    Time LeachNodeApplication::GetInterval(){
        return m_interval;
    }

    double LeachNodeApplication::GetChProb(){
        return m_chProb;
    }

    TypeId LeachNodeApplication::GetTypeId(){
        static TypeId tid = TypeId("UdpServer")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpServer>()
            .AddAttribute("Port", "Port on which we listen for incoming packets.", UintegerValue(100),
                            MakeUintegerAccessor(&LeachNodeApplication::m_port), MakeUintegerChecker<uint16_t>())
            .AddTraceSource("Rx", "A packet has been received", MakeTraceSourceAccessor(&LeachNodeApplication::m_rxTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses", "A packet has been received",
                            MakeTraceSourceAccessor(&LeachNodeApplication::m_rxTraceWithAddresses),
                            "Packet::TwoAddressTracedCallback");

        return tid;
    }

    void LeachNodeApplication::DoDispose(){
        NS_LOG_FUNCTION(this);

        Application::DoDispose();
    }

    void LeachNodeApplication::StartApplication(){
        NS_LOG_FUNCTION(this);
     
        if (!m_socket){
            TypeId tid = TypeId::LookupByName("UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            if (m_socket->Bind(local) == -1){
                NS_FATAL_ERROR("Failed to bind socket");
            }
            if (addressUtils::IsMulticast(m_localAddress)){
                Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket);
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
            TypeId tid = TypeId::LookupByName("UdpSocketFactory");
            m_socket6 = Socket::CreateSocket(GetNode(), tid);
            Inet6SocketAddress local6 = Inet6SocketAddress(Ipv6Address::GetAny(), m_port);
            if (m_socket6->Bind(local6) == -1){
                NS_FATAL_ERROR("Failed to bind socket");
            }
            if (addressUtils::IsMulticast(local6)){
                Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket6);
                if (udpSocket){
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
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }


    void LeachNodeApplication::ScheduleTransmit(Time dt){
        NS_LOG_FUNCTION(this << dt);

        m_sendEvent = Simulator::Schedule(dt, &LeachNodeApplication::Send, this);
    }

    void LeachNodeApplication::Send(){
        NS_LOG_FUNCTION(this);
     
        NS_ASSERT(m_sendEvent.IsExpired());
     
        Ptr<Packet> packet;
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
            packet = Create<Packet>(m_data, m_dataSize);
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
            packet = Create<Packet>(m_size);
        }
        Address localAddress;
        m_socket->GetSockName(localAddress);
        // call to the trace sinks before the packet is actually sent,
        // so that tags added to the packet can be sent as well
        m_txTrace(packet);
        if (Ipv4Address::IsMatchingType(m_targetAddress))
        {
            m_txTraceWithAddresses(packet, localAddress,
                InetSocketAddress(Ipv4Address::ConvertFrom(m_targetAddress), m_port));
        }
        else if (Ipv6Address::IsMatchingType(m_targetAddress))
        {
            m_txTraceWithAddresses(packet, localAddress,
                                    Inet6SocketAddress(Ipv6Address::ConvertFrom(m_targetAddress), m_port));
        }
        m_socket->Send(packet);
        ++m_sent;
     
        if (Ipv4Address::IsMatchingType(m_targetAddress)){
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size
                            << " bytes to " << Ipv4Address::ConvertFrom(m_targetAddress)
                            << " port " << m_port);
        }
        else if (Ipv6Address::IsMatchingType(m_targetAddress)){
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size
                            << " bytes to " << Ipv6Address::ConvertFrom(m_targetAddress)
                            << " port " << m_port);
        }
        else if (InetSocketAddress::IsMatchingType(m_targetAddress)){
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to "
                           << InetSocketAddress::ConvertFrom(m_targetAddress).GetIpv4() << " port "
                           << InetSocketAddress::ConvertFrom(m_targetAddress).GetPort());
        }
        else if (Inet6SocketAddress::IsMatchingType(m_targetAddress)){
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to "
                           << Inet6SocketAddress::ConvertFrom(m_targetAddress).GetIpv6() << " port "
                           << Inet6SocketAddress::ConvertFrom(m_targetAddress).GetPort());
        }
     
        if (m_sent < m_count || m_count == 0){
            ScheduleTransmit(m_interval);
        }
    }

    void LeachNodeApplication::HandleRead(Ptr<Socket> socket){
        NS_LOG_FUNCTION(this << socket);
        Ptr<Packet> packet;
        Address from;
        Address local;
        while ((packet = socket->RecvFrom(from))){
            socket->GetSockName(local);
            m_rxTrace(packet);
            m_rxTraceWithAddresses(packet, from, local);
            if (packet->GetSize() > 0){
                uint32_t receivedSize = packet->GetSize();
                SeqTsHeader seqTs;
                packet->RemoveHeader(seqTs);
                uint32_t currentSequenceNumber = seqTs.GetSeq();
                if (InetSocketAddress::IsMatchingType(from)){
                    NS_LOG_INFO("TraceDelay: RX " << receivedSize << " bytes from "
                                    << InetSocketAddress::ConvertFrom(from).GetIpv4()
                                    << " Sequence Number: " << currentSequenceNumber
                                    << " Uid: " << packet->GetUid() << " TXtime: "
                                    << seqTs.GetTs() << " RXtime: " << Simulator::Now()
                                    << " Delay: " << Simulator::Now() - seqTs.GetTs());
                }
                else if (Inet6SocketAddress::IsMatchingType(from)){
                    NS_LOG_INFO("TraceDelay: RX " << receivedSize << " bytes from "
                                    << Inet6SocketAddress::ConvertFrom(from).GetIpv6()
                                    << " Sequence Number: " << currentSequenceNumber
                                    << " Uid: " << packet->GetUid() << " TXtime: "
                                    << seqTs.GetTs() << " RXtime: " << Simulator::Now()
                                    << " Delay: " << Simulator::Now() - seqTs.GetTs());
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
} // ns3
