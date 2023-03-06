#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"



class SeqNumTag: protected ns3::Tag{
    public:
        SeqNumTag();

        void Serialize(ns3::TagBuffer buffer) const override;

        void Deserialize(ns3::TagBuffer buffer) override;

        void Print(std::ostream &os) const override;

        uint32_t GetSerializedSize() const override; 

        void SetSeq(uint32_t seq);

        uint32_t GetSeqNum();

    private:
        uint32_t m_seqNum;
};
