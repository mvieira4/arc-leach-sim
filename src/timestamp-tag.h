#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"


namespace ns3{
    class TimestampTag: protected ns3::Tag{
        public:
                TimestampTag();

                void SetTimestamp(Time timestamp);

                Time GetTimestamp();

                TypeId GetTypeId() const;

                TypeId GetInstanceTypeId() const override;

                uint32_t GetSerializedSize() const override;

                void Serialize(TagBuffer buffer) const override;

                void Deserialize(TagBuffer buffer) override;

                void Print(std::ostream &os) const override;

        private:
                Time m_timeStamp;
    };
}
