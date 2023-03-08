#include <string>

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include "ns3/leach-app.h"





namespace ns3{
    class LeachNodeHelper{
        public:
            LeachNodeHelper();

            ApplicationContainer Install(Ptr<Node> node) const;

            ApplicationContainer Install(std::string nodeName) const;

            ApplicationContainer Install(NodeContainer nodes) const;

            void SetAttribute(std::string name, const AttributeValue &value);

        private:
            Ptr<Application> InstallPriv(Ptr<Node> node) const;

            ObjectFactory m_factory;
    };
}
