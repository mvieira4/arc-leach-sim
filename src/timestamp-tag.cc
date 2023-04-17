#include "timestamp-tag.h"

namespace ns3{
    TimestampTag::TimestampTag(): m_timeStamp(0){}

    void TimestampTag::Serialize(TagBuffer buffer) const{
        buffer.WriteDouble(m_timeStamp.GetDouble());
    }

    void TimestampTag::Deserialize(TagBuffer buffer){
        m_timeStamp = Seconds(buffer.ReadDouble());
    }

    void TimestampTag::Print(std::ostream &os) const{
        os << "Timestamp: " << m_timeStamp << "\n";
    }

    uint32_t TimestampTag::GetSerializedSize() const{
        return sizeof(m_timeStamp);
    }

    void TimestampTag::SetTimestamp(Time timestamp){
        m_timeStamp = timestamp;
    }

    Time TimestampTag::GetTimestamp(){
        return m_timeStamp;
    }
}
