#include "ns3/arc-leach-sink-helper.h"




namespace ns3{
    NS_LOG_COMPONENT_DEFINE("ArcLeachSinkHelper");

    ArcLeachSinkHelper::ArcLeachSinkHelper(uint32_t nodeNum){
        NS_LOG_FUNCTION(this);

        NS_LOG_DEBUG("ArcLeachSinkHelper: Number of nodes is " << nodeNum);

        m_factory.SetTypeId(ArcLeachSinkApplication::GetTypeId());
    }

    ApplicationContainer ArcLeachSinkHelper::Install(Ptr<Node> node) const{
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer ArcLeachSinkHelper::Install(NodeContainer nodes) const{
        ApplicationContainer apps;

        NodeContainer::Iterator node = nodes.Begin();

        while(node != nodes.End()){
            apps.Add(InstallPriv(*node));

            node++;
        }

        return apps;
    }

    void ArcLeachSinkHelper::SetAttribute(std::string name, const AttributeValue &value){
        m_factory.Set(name, value);
    }

    Ptr<Application> ArcLeachSinkHelper::InstallPriv(Ptr<Node> node) const{
        Ptr<ArcLeachSinkApplication> app;  

        app = m_factory.Create<ArcLeachSinkApplication>();

        node->AddApplication(app);

        return app;
    } 
}
