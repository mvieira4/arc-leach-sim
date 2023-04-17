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

        app = m_factory.Create<LeachNodeApplication>();
        app->SetEnergyModel(energyModel);
        //app->SetIsCh((((double)rand() / RAND_MAX) < 0.9));

        node->AddApplication(app);

        return app;
    } 
}
