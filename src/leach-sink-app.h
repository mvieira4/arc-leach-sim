/*
 *
 *  Author: Marcel Vieira
 *
 *  Description:
 *      Header file for LeachSinkApplication class. This class implements LEACH protocol on a sink.
 *
 */


#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/wifi-radio-energy-model.h"

#include <array>
#include <vector>
#include <cstdlib>


namespace ns3{
    class LeachSinkApplication: public Application{
        public:
            LeachSinkApplication(bool isCh = true, bool isMal = false);

            ~LeachSinkApplication() override;

            static TypeId GetTypeId();

            void SetInterval(Time time);

            Time GetInterval();

        protected:
            void DoDispose() override;
            
        private:
            void StartApplication() override;

            void StopApplication() override;

            void ScheduleTransmit(Time dt, Ptr<Packet> packet, Ipv4Address address);

            void Send(Ptr<Packet>, Ipv4Address);

            void HandleRead(Ptr<Socket> socket);


            Ptr<Socket> m_socket;

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

            std::vector<Ipv4Address> blacklist;

            // ?Dictionary of potential blacklist nodes

            uint32_t m_sent;
            uint32_t m_received;

            uint32_t m_port;
            uint32_t m_packetSize;

            uint32_t m_roundEvents;
            uint32_t m_roundEventN;

            uint32_t m_rounds;
            uint32_t m_roundN;

            uint32_t m_executedRounds;

            Ptr<WifiRadioEnergyModel> m_energyModel;
    }; 
}

