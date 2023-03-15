#include <string>

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/device-energy-model-container.h"

#include "ns3/leach-app.h"





namespace ns3{
    class LeachNodeHelper{
        public:
            LeachNodeHelper(uint32_t nodeNum);

            LeachNodeHelper(uint32_t nodeNum, double malProb);

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
