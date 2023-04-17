#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"


namespace ns3{
    class SeqNumTag: protected Tag{
        public:
            SeqNumTag();

            void Serialize(TagBuffer buffer) const override;

            void Deserialize(TagBuffer buffer) override;

            void Print(std::ostream &os) const override;

            uint32_t GetSerializedSize() const override; 

            void SetSeq(uint32_t seq);

            uint32_t GetSeqNum();

        private:
            uint32_t m_seqNum;
    };
}
