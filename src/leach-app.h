#include <array>
#include <vector>
#include <cstdlib>

#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/wifi-radio-energy-model.h"




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

            Time GetInterval();

            static TypeId GetTypeId();

            void SetIsCh(bool x);

            bool GetIsCh();

            void SetIsMal(bool x);

            bool GetIsMal();

            void SetEnergyModel(Ptr<DeviceEnergyModel> model);

            Ptr<WifiRadioEnergyModel> GetEnergyModel();

        protected:
            void DoDispose() override;
            
        private:
            void StartApplication() override;

            void StopApplication() override;

            void ScheduleNextRound(Time dt);

            void ExecuteRound();

            void ScheduleTransmit(Time dt);

            void Send();

            void HandleRead(Ptr<Socket> socket);

            void FindCh();

            void Advertise();

            void ReportEvent();

            Ptr<Socket> m_socket;
            Ptr<UdpSocket> m_udpSocket;

            PacketLossCounter m_lossCounter;

            TracedCallback<Ptr<const Packet>> m_rxTrace;
            TracedCallback<Ptr<const Packet>> m_txTrace;

            TracedCallback<Ptr<const Packet>, const Address&, 
                                    const Address&> m_rxTraceWithAddresses;
            TracedCallback<Ptr<const Packet>, const Address&, 
                                    const Address&> m_txTraceWithAddresses;

            EventId m_sendEvent;
            EventId m_roundEvent;
            Time m_interval;

            bool m_isCh;
            bool m_isMal;
             
            Ipv4Address m_localAddress;
            Ipv4Address m_targetAddress;

            uint32_t m_sent;
            uint32_t m_received;

            uint32_t m_port;
            uint32_t m_size;
            uint32_t m_count;
            
            uint8_t *m_data;
            uint32_t m_dataSize;

            Ptr<WifiRadioEnergyModel> m_energyModel;
    }; 
}

