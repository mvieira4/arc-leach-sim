#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"



class InfoIdTag: protected ns3::Tag{
    public:
        InfoIdTag();

        void SetInfoId(std::string id);

        std::string GetInfoId();

        ns3::TypeId GetTypeId() const;

        ns3::TypeId GetInstanceTypeId() const override;

        uint32_t GetSerializedSize() const override;

        void Serialize(ns3::TagBuffer buffer) const override;

        void Deserialize(ns3::TagBuffer buffer) override;

        void Print(std::ostream &os) const override;

    private:
        std::string m_infoId;
};


