#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"


namespace ns3{
    class LeachTag: public Tag{
        public:
            static const uint8_t NO = 0;
            static const uint8_t RE = 1;
            static const uint8_t ARE = 2;
            static const uint8_t AD = 3;
            static const uint8_t BK = 4;
            static const uint8_t ADN = 5;
            static const uint8_t AK = 6;

            LeachTag();

            void SetPackType(uint8_t type);
            void SetSeqNum(uint8_t seq);
            void SetTimestamp(Time timestamp);
            void SetNodeAddress(Ipv4Address address);
            void SetCHAddress(Ipv4Address address);
            void SetCHPrevAddress(Ipv4Address address);

            uint8_t GetPackType();
            uint8_t GetSeqNum();
            Time GetTimestamp();
            Ipv4Address GetNodeAddress();
            Ipv4Address GetCHAddress();
            Ipv4Address GetCHPrevAddress();

            TypeId GetTypeId() const;

            TypeId GetInstanceTypeId() const override;

            uint32_t GetSerializedSize() const override; 

            void Serialize(TagBuffer buffer) const override;

            void Deserialize(TagBuffer buffer) override;

            void Print(std::ostream &os) const override;

        private:
            uint8_t m_packType;
            uint8_t m_seqNum;
            Time m_timestamp;
            Ipv4Address m_nodeAddress;
            Ipv4Address m_chAddress;
            Ipv4Address m_chPrevAddress;
    };
}
