#include "info-id-tag.h"



InfoIdTag::InfoIdTag(){}

void InfoIdTag::SetInfoId(std::string id){
    m_infoId = id;
}

std::string InfoIdTag::GetInfoId(){
    return m_infoId;
}

ns3::TypeId InfoIdTag::GetTypeId() const{
    static ns3::TypeId tid = ns3::TypeId("ns3::InfoIdTag")
                                .SetParent<Tag>()
                                .SetGroupName("Leach")
                                .AddConstructor<InfoIdTag>();

    return tid;
}

ns3::TypeId InfoIdTag::GetInstanceTypeId() const{
    return GetTypeId();
}

uint32_t InfoIdTag::GetSerializedSize() const{
    return sizeof(m_infoId);
}

void InfoIdTag::Serialize(ns3::TagBuffer buffer) const{
    const char* tmp = m_infoId.c_str();
    uint8_t len = (uint8_t)m_infoId.size();

    buffer.WriteU8(len);
    buffer.Write((uint8_t*)tmp, (uint32_t)len);
}

void InfoIdTag::Deserialize(ns3::TagBuffer buffer){
    uint8_t len = buffer.ReadU8();
    char buf[256];
 
    buffer.Read((uint8_t*)buf, (uint32_t)len);
    m_infoId = std::string(buf, len);
}

void InfoIdTag::Print(std::ostream &os) const{
    os << "Info Id: " << m_infoId << "\n"; 
}

