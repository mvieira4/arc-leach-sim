#include "info-id-tag.h"



InfoIdTag::InfoIdTag(uint32_t id): m_infoId(id){}

void InfoIdTag::Serialize(ns3::TagBuffer buffer) const{
    buffer.WriteU32(m_infoId);
}

void InfoIdTag::Deserialize(ns3::TagBuffer buffer){
    m_infoId = buffer.ReadU32();
}

void InfoIdTag::Print(std::ostream &os) const{
    os << "Info Id: " << m_infoId << "\n"; 
}

uint32_t InfoIdTag::GetSerializedSize() const{
    return sizeof(m_infoId);
}

void InfoIdTag::SetInfoId(std::string id){
    m_infoId = id;
}

std::string InfoIdTag::GetInfoId(){
    return m_infoId;
}

