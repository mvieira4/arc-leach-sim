#include "ns3/leach-helper.h"





namespace ns3{
    NS_LOG_COMPONENT_DEFINE("LeachNodeHelper");

    LeachNodeHelper::LeachNodeHelper(uint32_t nodeNum){
        NS_LOG_FUNCTION(this);

        NS_LOG_DEBUG("LeachNodeHelper: Number of nodes is " << nodeNum);

        m_factory.SetTypeId(LeachNodeApplication::GetTypeId());
    }


    ApplicationContainer LeachNodeHelper::Install(Ptr<Node> node, Ptr<DeviceEnergyModel> energyModel) const{
        return ApplicationContainer(InstallPriv(node, energyModel));
    }

    ApplicationContainer LeachNodeHelper::Install(NodeContainer nodes, DeviceEnergyModelContainer energyModels) const{
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

    void LeachNodeHelper::SetAttribute(std::string name, const AttributeValue &value){
        m_factory.Set(name, value);
    }

    Ptr<Application> LeachNodeHelper::InstallPriv(Ptr<Node> node, Ptr<DeviceEnergyModel> energyModel) const{
        Ptr<LeachNodeApplication> app;  

        // Create a random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
          
        // Define the range of the random number
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        double random_double = dis(gen);

        app = m_factory.Create<LeachNodeApplication>();
        app->SetEnergyModel(energyModel);

        node->AddApplication(app);

        return app;
    } 
}
