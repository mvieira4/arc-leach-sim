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
    
    LeachNodeApplication::LeachNodeApplication(): m_lossCounter(0){
        NS_LOG_FUNCTION(this);

        m_socket = nullptr;
        m_sendEvent = EventId();
        m_interval = Seconds(4.0);
        m_sent = 0;
        m_count = 0;
        m_dataSize = 0;
        m_size = 1024;
        m_data = nullptr;
    }



    LeachNodeApplication::~LeachNodeApplication(){
        NS_LOG_FUNCTION(this);

        m_socket = nullptr;
        m_sendEvent = EventId();

        delete[] m_data;
        m_data = nullptr;
        m_dataSize = 0;
    }
    



    // Getters & Setters
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
    TypeId LeachNodeApplication::GetTypeId(){
        static TypeId tid = TypeId("ns3::LeachNodeApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<LeachNodeApplication>()
            .AddAttribute("IsCh", "If node is cluster head", BooleanValue(true), 
                            MakeDoubleAccessor(&LeachNodeApplication::m_isCh), MakeBooleanChecker())
            .AddAttribute("IsMal", "If node is malicious", BooleanValue(false), 
                            MakeDoubleAccessor(&LeachNodeApplication::m_isMal), MakeBooleanChecker())
            .AddTraceSource("Rx", "A packet has been received", MakeTraceSourceAccessor(&LeachNodeApplication::m_rxTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses", "A packet has been sent",
                            MakeTraceSourceAccessor(&LeachNodeApplication::m_rxTraceWithAddresses),
                            "Packet::TwoAddressTracedCallback")
            .AddTraceSource("Tx", "A packet has been sent", MakeTraceSourceAccessor(&LeachNodeApplication::m_txTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("TxWithAddresses", "A packet has been sent",
                            MakeTraceSourceAccessor(&LeachNodeApplication::m_txTraceWithAddresses),
                            "Packet::TwoAddressTracedCallback");

        return tid;
    }



    void LeachNodeApplication::DoDispose(){
        NS_LOG_FUNCTION(this);

        Application::DoDispose();
    }



    void LeachNodeApplication::StartApplication(){
        NS_LOG_FUNCTION(this);

        Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
        m_localAddress = ipv4->GetAddress (1, 0).GetLocal();

        TypeId tid = TypeId::LookupByName("ns3::Ipv4RawSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        m_socket->SetAllowBroadcast(true);


        if (m_socket->Bind(InetSocketAddress(m_localAddress, m_port)) == -1){
            NS_FATAL_ERROR("Failed to bind socket");
        }

        m_socket->SetRecvCallback(MakeCallback(&LeachNodeApplication::HandleRead, this));


        ScheduleNextRound(Seconds(0));
    }



    void LeachNodeApplication::ScheduleNextRound(Time dt){
        NS_LOG_FUNCTION(this << dt);

        m_roundEvent = Simulator::Schedule(dt, &LeachNodeApplication::ExecuteRound, this);
    }



    void LeachNodeApplication::ExecuteRound(){
        Advertise();

        ScheduleNextRound(m_interval);
    }



    void LeachNodeApplication::ScheduleTransmit(Time dt){
        NS_LOG_FUNCTION(this << dt);

        m_sendEvent = Simulator::Schedule(dt, &LeachNodeApplication::Send, this);
    }



    void LeachNodeApplication::Send(){

        Ptr<WifiNetDevice> device = DynamicCast<WifiNetDevice>(GetNode()->GetDevice(0));

        Ptr<Packet> packet = Create<Packet>(m_size);

        Ptr<Channel> channel = GetNode()->GetDevice(0)->GetChannel();

        for (uint32_t i = 0; i < channel->GetNDevices(); i++){
            Ptr<Ipv4> ipv4= channel->GetDevice(i)->GetNode()->GetObject<Ipv4>();
            Ipv4Address address = ipv4->GetAddress (1, 0).GetLocal();

            if(address != m_localAddress){
                m_socket->SendTo(packet, 0, InetSocketAddress(address, m_port));

                NS_LOG_DEBUG("Sent from ip " << m_localAddress
                                    << " at time " << Simulator::Now().As(Time::S)
                                    << " to ip " << address);
                m_txTrace(packet);
                m_rxTraceWithAddresses(packet, m_localAddress, address);
            }
        }
    }



    void LeachNodeApplication::HandleRead(Ptr<Socket> socket){
        NS_LOG_FUNCTION(this << socket);

        Ptr<Packet> packet;
        Address from;
        Address localAddress;

        while ((packet = socket->RecvFrom(from))){
            NS_LOG_INFO("Recv at ip " << m_localAddress << " at time " 
                                << Simulator::Now().As(Time::S) << " from "
                                << InetSocketAddress::ConvertFrom(from).GetIpv4());

            socket->GetSockName(localAddress);

            m_rxTrace(packet);
            m_rxTraceWithAddresses(packet, from, localAddress);
        }
    }



    void LeachNodeApplication::FindCh(){
        NS_LOG_FUNCTION(this << m_socket);

        Ptr<Packet> packet;
        Address from;
        Address local;

        while ((packet = m_socket->RecvFrom(from))){
            m_socket->GetSockName(local);
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
                } else if (Inet6SocketAddress::IsMatchingType(from)){
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



    void LeachNodeApplication::Advertise(){
        ScheduleTransmit(MilliSeconds(200));
    }



    void LeachNodeApplication::ReportEvent(){

        ScheduleTransmit(MilliSeconds(200));
    }



    void LeachNodeApplication::StopApplication(){
        NS_LOG_FUNCTION(this);
     
        if (m_socket){
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }
} // ns3
