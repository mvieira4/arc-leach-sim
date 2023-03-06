#include <array>
#include <vector>

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/socket.h"

#include "info-id-tag.h"
#include "seq-num-tag.h"



// Global Variables
const uint32_t PACKET_SIZE = 1024;
const bool ALLOW_BROADCAST = false;
const uint16_t PORT_NUM = 5000;


class LeachNodeApplication: protected ns3::Application{
    public:
        LeachNodeApplication();

        ~LeachNodeApplication();

        void SetInterval(ns3::Time time);

    private:
        void StartApplication() override;

        void StopApplication() override;

        ns3::TypeId GetTypeId();
        
        void ScheduleTransmit(ns3::Time dt);

        void ExecuteRound();

        void Send(uint8_t data[]);

        void HandleRead(ns3::Ptr<ns3::Socket> socket);

        void FindClusterHead();

        void AdvertiseClusterHead();

        void ReportEvent();

        ns3::Ptr<ns3::Node> m_targetNode;

        ns3::Ptr<ns3::Socket> m_socket;
        ns3::Ptr<ns3::Socket> m_socket6;

        ns3::PacketLossCounter m_lossCounter;

        ns3::TracedCallback<ns3::Ptr<const ns3::Packet>> m_rxTrace;
        ns3::TracedCallback<ns3::Ptr<const ns3::Packet>> m_txTrace;

        ns3::TracedCallback<ns3::Ptr<const ns3::Packet>, const ns3::Address&, 
                                const ns3::Address&> m_rxTraceWithAddresses;
        ns3::TracedCallback<ns3::Ptr<const ns3::Packet>, const ns3::Address&, 
                                const ns3::Address&> m_txTraceWithAddresses;

        std::vector<ns3::Ptr<ns3::Address>> m_nearbyClusterHeads;
        ns3::EventId m_sendEvent;
        ns3::Time m_interval;
         
        ns3::Address m_localAddress;
        ns3::Address m_targetAddress;

        uint32_t m_sent;
        uint32_t m_received;

        uint32_t m_port;
        uint32_t m_size;
        uint32_t m_count;
        uint32_t m_dataSize;
        uint8_t *m_data;
};
