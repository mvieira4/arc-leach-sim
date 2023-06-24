#include <string>
#include <random>

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/device-energy-model-container.h"

#include "ns3/alt-route-app.h"





namespace ns3{
    class AltRouteNodeHelper{
        public:
            AltRouteNodeHelper(uint32_t nodeNum);

            AltRouteNodeHelper(uint32_t nodeNum, double malProb);

            ApplicationContainer Install(Ptr<Node> node, Ptr<DeviceEnergyModel> energyModel) const;

            ApplicationContainer Install(NodeContainer nodes, DeviceEnergyModelContainer energyModels) const;

            void SetAttribute(std::string name, const AttributeValue &value);

        private:
            Ptr<Application> InstallPriv(Ptr<Node> node, Ptr<DeviceEnergyModel> energyModel) const;

            ObjectFactory m_factory;

            uint32_t m_nodeNum;
            double m_malProb;
    };
}
