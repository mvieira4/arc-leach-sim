#include "ns3/leach-sink-helper.h"




namespace ns3{
    NS_LOG_COMPONENT_DEFINE("LeachSinkHelper");

    LeachSinkHelper::LeachSinkHelper(uint32_t nodeNum){
        NS_LOG_FUNCTION(this);

        NS_LOG_DEBUG("LeachSinkHelper: Number of nodes is " << nodeNum);

        m_factory.SetTypeId(LeachSinkApplication::GetTypeId());
    }

    ApplicationContainer LeachSinkHelper::Install(Ptr<Node> node) const{
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer LeachSinkHelper::Install(NodeContainer nodes) const{
        ApplicationContainer apps;

        NodeContainer::Iterator node = nodes.Begin();

        while(node != nodes.End()){
            apps.Add(InstallPriv(*node));

            node++;
        }

        return apps;
    }

    void LeachSinkHelper::SetAttribute(std::string name, const AttributeValue &value){
        m_factory.Set(name, value);
    }

    Ptr<Application> LeachSinkHelper::InstallPriv(Ptr<Node> node) const{
        Ptr<LeachSinkApplication> app;  

        app = m_factory.Create<LeachSinkApplication>();

        node->AddApplication(app);

        return app;
    } 
}
