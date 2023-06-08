#include <string>

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/device-energy-model-container.h"

#include "ns3/alt-route-sink-app.h"





namespace ns3{
    class AltRouteSinkHelper{
        public:
            AltRouteSinkHelper(uint32_t nodeNum);

            ApplicationContainer Install(Ptr<Node> node) const;

            ApplicationContainer Install(NodeContainer nodes) const;

            void SetAttribute(std::string name, const AttributeValue &value);

        private:
            Ptr<Application> InstallPriv(Ptr<Node> node) const;

            ObjectFactory m_factory;

            uint32_t m_nodeNum;
    };
}
