#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"


namespace ns3{
    class TimestampTag: protected ns3::Tag{
        public:
                TimestampTag();

                void Serialize(TagBuffer buffer) const override;

                void Deserialize(TagBuffer buffer) override;

                void Print(std::ostream &os) const override;

                uint32_t GetSerializedSize() const override;

                void SetTimestamp(Time timestamp);

                Time GetTimestamp();

        private:
                Time m_timeStamp;
    };
}
