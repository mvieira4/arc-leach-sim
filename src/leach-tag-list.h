#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"

#include "ns3/leach-tag.h"


namespace ns3{
    class LeachTagList: public Tag{
        public:
            LeachTagList();

            ~LeachTagList();

            void PushTag(LeachTag tag);

            LeachTag PopTag();

            LeachTag BackTag();

            uint32_t GetSize();

            TypeId GetTypeId() const;

            TypeId GetInstanceTypeId() const override;

            uint32_t GetSerializedSize() const override; 

            void Serialize(TagBuffer buffer) const override;

            void Deserialize(TagBuffer buffer) override;

            void Print(std::ostream &os) const override;

        private:
            std::vector<LeachTag> *m_list;
            uint32_t m_listSize;
    };
}
