#include "seq-num-tag.h"



SeqNumTag::SeqNumTag(): m_seqNum(0){}

void SeqNumTag::Serialize(ns3::TagBuffer buffer) const{
    buffer.WriteU32(m_seqNum);
}

void SeqNumTag::Deserialize(ns3::TagBuffer buffer){
    m_seqNum = buffer.ReadU32();
}

void SeqNumTag::Print(std::ostream &os) const{
    os << "Sequence Number: " << m_seqNum << "\n";
}

uint32_t SeqNumTag::GetSerializedSize() const{
    return sizeof(m_seqNum);
}

void SeqNumTag::SetSeq(uint32_t seq){
    m_seqNum = seq;
}

uint32_t SeqNumTag::GetSeqNum(){
    return m_seqNum;
}
