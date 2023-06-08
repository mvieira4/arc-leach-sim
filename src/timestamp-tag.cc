#include "ns3/timestamp-tag.h"

namespace ns3{
    TimestampTag::TimestampTag(): m_timestamp(0){}

    void TimestampTag::SetTimestamp(Time timestamp){
        m_timestamp = timestamp;
    }

    Time TimestampTag::GetTimestamp(){
        return m_timestamp;
    }

    ns3::TypeId TimestampTag::GetTypeId() const{
        static ns3::TypeId tid = ns3::TypeId("ns3::TimestampTag")
                                    .SetParent<Tag>()
                                    .SetGroupName("Leach")
                                    .AddConstructor<TimestampTag>();

        return tid;
    }

    ns3::TypeId TimestampTag::GetInstanceTypeId() const{
        return GetTypeId();
    }

    uint32_t TimestampTag::GetSerializedSize() const{
        return 8;
    }

    void TimestampTag::Serialize(TagBuffer buffer) const{
        buffer.WriteU64(m_timestamp.GetTimeStep());
    }

    void TimestampTag::Deserialize(TagBuffer buffer){
        m_timestamp = TimeStep(buffer.ReadU64());
    }

    void TimestampTag::Print(std::ostream &os) const{
        os << m_timestamp.As(Time::S) << "\n";
    }
}
