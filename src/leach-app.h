#include <array>
#include <vector>

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/socket.h"




// Global Variables
const uint32_t PACKET_SIZE = 1024;
const bool ALLOW_BROADCAST = false;
const uint16_t PORT_NUM = 5000;

namespace ns3{
    class LeachNodeApplication: public Application{
        public:
            LeachNodeApplication();

            ~LeachNodeApplication();

            void SetInterval(Time time);

            void SetChProb(double prob);

            Time GetInterval();

            double GetChProb();

            static TypeId GetTypeId();

        protected:
            void DoDispose() override;
            
        private:
            void StartApplication() override;

            void StopApplication() override;

            void ScheduleTransmit(Time dt);

            void ExecuteRound();

            void Send();

            void HandleRead(Ptr<Socket> socket);

            void FindClusterHead();

            void AdvertiseClusterHead();

            void ReportEvent();

            Ptr<Socket> m_socket;
            Ptr<Socket> m_socket6;

            PacketLossCounter m_lossCounter;

            TracedCallback<Ptr<const Packet>> m_rxTrace;
            TracedCallback<Ptr<const Packet>> m_txTrace;

            TracedCallback<Ptr<const Packet>, const Address&, 
                                    const Address&> m_rxTraceWithAddresses;
            TracedCallback<Ptr<const Packet>, const Address&, 
                                    const Address&> m_txTraceWithAddresses;

            std::vector<Address> m_nearbyClusterHeads;
            EventId m_sendEvent;
            Time m_interval;
            double m_chProb;
             
            Address m_localAddress;
            Address m_targetAddress;

            uint32_t m_sent;
            uint32_t m_received;

            uint32_t m_port;
            uint32_t m_size;
            uint32_t m_count;
            uint32_t m_dataSize;
            uint8_t *m_data;
    }; // LeachApplication
} // ns3

