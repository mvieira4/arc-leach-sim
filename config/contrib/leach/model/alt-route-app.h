/*
 *
 *  Author: Marcel Vieira
 *
 *  Descritpion: 
 *      Header file for AltRouteNodeApplication class. This class implements LEACH protocol on a node.
 *
 */


#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/device-energy-model-container.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/yans-wifi-phy.h"

#include <array>
#include <vector>
#include <cstdlib>


namespace ns3{
    class AltRouteNodeApplication: public Application{
        public:
            AltRouteNodeApplication();

            ~AltRouteNodeApplication() override;

            static TypeId GetTypeId();

            void SetInterval(Time time);

            Time GetInterval();

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

            void ReportStatus();

            void ScheduleAdvertise(Time dt);

            void Advertise();

            void ScheduleNextRound(Time dt);

            void ExecuteRound();

            void ScheduleNextEvent(Time dt);

            void AltRoute();

            void ReportEvent();

            void ScheduleTransmit(Time dt, Ptr<Packet> packet, Ipv4Address address);

            void Send(Ptr<Packet> packet, Ipv4Address address);

            void CHSend(Ptr<Packet> packet);

            void CHSend(Ptr<Packet> packet, Ipv4Address address);

            void HandleRead(Ptr<Socket> socket);

            bool CheckDead();

            void AppendPack(Ptr<Packet> packet);

            Ptr<Socket> m_socket;

            PacketLossCounter m_lossCounter;

            TracedCallback<Ptr<const Packet>> m_rxTrace;
            TracedCallback<Ptr<const Packet>, const Address&, 
                                    const Address&> m_rxTraceWithAddresses;

            TracedCallback<Ptr<const Packet>> m_txTrace;
            TracedCallback<Ptr<const Packet>, const Address&, 
                                    const Address&> m_txTraceWithAddresses;

            TracedCallback<uint32_t> m_energyTrace;
            TracedCallback<uint32_t> m_statusTrace;


            EventId m_sendEvent;
            EventId m_roundEvent;

            Time m_interval;

            Time m_shortestDelay;

            bool m_isCh;
            bool m_isMal;
            bool m_dead;
            bool m_ack;
             
            Ipv4Address m_localAddress;
            Ipv4Address m_sinkAddress;
            Ipv4Address m_chAddress;
            Ipv4Address m_target;
            Ipv4Address m_chPrevAddress;

            std::vector<Ipv4Address> m_blacklist;
            std::vector<Ipv4Address> m_nodes;

            uint32_t m_sent;
            uint32_t m_received;

            uint32_t m_port;

            uint32_t m_packetSize;
            uint32_t m_maxPacketSize;

            uint32_t m_maxNodes;

            uint32_t m_roundEvents;
            uint32_t m_roundEventN;

            uint32_t m_rounds;
            uint32_t m_roundN;

            uint32_t m_routes;
            uint32_t m_routeN;

            uint32_t m_advertisments;

            uint32_t m_executedRounds;

            Ptr<Packet> m_agroPacket;

            uint8_t m_seqNum;

            Ptr<WifiRadioEnergyModel> m_energyModel;
    }; 
}

