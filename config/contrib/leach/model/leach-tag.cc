#include "leach-tag.h"



namespace ns3{
    NS_LOG_COMPONENT_DEFINE("LeachTag");

    LeachTag::LeachTag(){
        NS_LOG_FUNCTION(this);

        m_packType = 0;
        m_seqNum = 0;
        m_timestamp = Seconds(0);
    }

    void LeachTag::SetPackType(uint8_t type){
        m_packType = type;
    }

    void LeachTag::SetSeqNum(uint8_t seq){
        m_seqNum = seq;
    }

    void LeachTag::SetTimestamp(Time timestamp){
        m_timestamp = timestamp; 
    }

    void LeachTag::SetNodeAddress(Ipv4Address address){
        m_nodeAddress = address;
    }

    void LeachTag::SetCHAddress(Ipv4Address address){
        m_chAddress = address;
    }

    void LeachTag::SetCHPrevAddress(Ipv4Address address){
        m_chPrevAddress = address;
    }

    uint8_t LeachTag::GetPackType(){
        return m_packType;
    }

    uint8_t LeachTag::GetSeqNum(){
        return m_seqNum;
    }

    Time LeachTag::GetTimestamp(){
        return m_timestamp;
    }

    Ipv4Address LeachTag::GetNodeAddress(){
        return m_nodeAddress;
    }

    Ipv4Address LeachTag::GetCHAddress(){
        return m_chAddress;
    }

    Ipv4Address LeachTag::GetCHPrevAddress(){
        return m_chPrevAddress;
    }

    ns3::TypeId LeachTag::GetTypeId() const{
        NS_LOG_FUNCTION(this);

       static ns3::TypeId tid = ns3::TypeId("ns3::LeachTag")
                                    .SetParent<Tag>()
                                    .SetGroupName("Leach")
                                    .AddConstructor<LeachTag>()
                                    .AddAttribute("PackType", 
                                        "Value representing packet type", 
                                        UintegerValue(RE),
                                        MakeUintegerAccessor(&LeachTag::m_packType), 
                                        MakeIntegerChecker<uint8_t>())
                                    .AddAttribute("SeqNum", 
                                        "Number assosiated to packet", 
                                        UintegerValue(0),
                                        MakeUintegerAccessor(&LeachTag::m_seqNum), 
                                        MakeIntegerChecker<uint8_t>())
                                    .AddAttribute("Timestamp", 
                                        "Timestamp value", 
                                        TimeValue(Seconds(0)),
                                        MakeTimeAccessor(&LeachTag::m_timestamp), 
                                        MakeTimeChecker());

       return tid;
    }

    ns3::TypeId LeachTag::GetInstanceTypeId() const{
        NS_LOG_FUNCTION(this);

        return GetTypeId();
    }

    uint32_t LeachTag::GetSerializedSize() const{
        NS_LOG_FUNCTION(this);

        uint8_t size = sizeof(m_packType) + sizeof(m_seqNum) + sizeof(m_timestamp) + sizeof(m_nodeAddress) 
            + sizeof(m_chAddress) + sizeof(m_chPrevAddress);

        return size;
    }

    void LeachTag::Serialize(TagBuffer buffer) const{
        NS_LOG_FUNCTION(this);

        buffer.WriteU8(m_packType);
        buffer.WriteU8(m_seqNum);
        buffer.WriteU64(m_timestamp.GetTimeStep());

        NS_LOG_DEBUG("Node Addr: " << m_nodeAddress);
        buffer.WriteU32(m_nodeAddress.Get());

        NS_LOG_DEBUG("CH Addr: " << m_chAddress);
        buffer.WriteU32(m_chAddress.Get());

        NS_LOG_DEBUG("CH Prev Addr: " << m_chPrevAddress);
        buffer.WriteU32(m_chPrevAddress.Get());
    }

    void LeachTag::Deserialize(TagBuffer buffer){
        NS_LOG_FUNCTION(this);

        m_packType = buffer.ReadU8();
        m_seqNum = buffer.ReadU8();
        m_timestamp = TimeStep(buffer.ReadU64());

        m_nodeAddress = Ipv4Address(buffer.ReadU32());
        NS_LOG_DEBUG("Node Addr: " << m_nodeAddress);

        m_chAddress = Ipv4Address(buffer.ReadU32());
        NS_LOG_DEBUG("CH Addr: " << m_chAddress);

        m_chPrevAddress = Ipv4Address(buffer.ReadU32());
        NS_LOG_DEBUG("CH Prev Addr: " << m_chPrevAddress);
    }

    void LeachTag::Print(std::ostream &os) const{
        NS_LOG_FUNCTION(this);

        os << "Packet Type: " << m_packType
            << "Sequence Number" << m_seqNum
            << "Timestamp" << m_timestamp
            << "\n";
    }
}
