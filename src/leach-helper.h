#include <string>

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include "leach-app.h"

class LeachNodeHelper{
    public:
        LeachNodeHelper();

        ns3::ApplicationContainer Install(ns3::Ptr<ns3::Node> node) const;

        ns3::ApplicationContainer Install(std::string nodeName) const;

        ns3::ApplicationContainer Install(ns3::NodeContainer nodes) const;

        void SetAttribute(std::string name, const ns3::AttributeValue &value);

    private:
        ns3::Ptr<ns3::Application> InstallPriv(ns3::Ptr<ns3::Node> node) const;

        ns3::ObjectFactory m_factory;
};
