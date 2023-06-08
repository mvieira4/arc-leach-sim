#include "ns3/arc-leach-helper.h"





namespace ns3{
    NS_LOG_COMPONENT_DEFINE("ArcLeachNodeHelper");

    ArcLeachNodeHelper::ArcLeachNodeHelper(uint32_t nodeNum){
        NS_LOG_FUNCTION(this);

        NS_LOG_DEBUG("ArcLeachNodeHelper: Number of nodes is " << nodeNum);

        m_factory.SetTypeId(ArcLeachNodeApplication::GetTypeId());
    }


    ApplicationContainer ArcLeachNodeHelper::Install(Ptr<Node> node, Ptr<DeviceEnergyModel> energyModel) const{
        return ApplicationContainer(InstallPriv(node, energyModel));
    }

    ApplicationContainer ArcLeachNodeHelper::Install(NodeContainer nodes, DeviceEnergyModelContainer energyModels) const{
        ApplicationContainer apps;

        NodeContainer::Iterator node = nodes.Begin();
        DeviceEnergyModelContainer::Iterator energyModel = energyModels.Begin();

        while(node != nodes.End()){
            apps.Add(InstallPriv(*node, *energyModel));

            node++;
            energyModel++;
        }


        return apps;
    }

    void ArcLeachNodeHelper::SetAttribute(std::string name, const AttributeValue &value){
        m_factory.Set(name, value);
    }

    Ptr<Application> ArcLeachNodeHelper::InstallPriv(Ptr<Node> node, Ptr<DeviceEnergyModel> energyModel) const{
        Ptr<ArcLeachNodeApplication> app;  

        // Create a random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
          
        // Define the range of the random number
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        double random_double = dis(gen);

        app = m_factory.Create<ArcLeachNodeApplication>();
        app->SetEnergyModel(energyModel);

        node->AddApplication(app);

        return app;
    } 
}
