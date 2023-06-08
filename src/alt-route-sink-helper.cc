#include "ns3/alt-route-sink-helper.h"




namespace ns3{
    NS_LOG_COMPONENT_DEFINE("AltRouteSinkHelper");

    AltRouteSinkHelper::AltRouteSinkHelper(uint32_t nodeNum){
        NS_LOG_FUNCTION(this);

        NS_LOG_DEBUG("AltRouteSinkHelper: Number of nodes is " << nodeNum);

        m_factory.SetTypeId(AltRouteSinkApplication::GetTypeId());
    }

    ApplicationContainer AltRouteSinkHelper::Install(Ptr<Node> node) const{
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer AltRouteSinkHelper::Install(NodeContainer nodes) const{
        ApplicationContainer apps;

        NodeContainer::Iterator node = nodes.Begin();

        while(node != nodes.End()){
            apps.Add(InstallPriv(*node));

            node++;
        }

        return apps;
    }

    void AltRouteSinkHelper::SetAttribute(std::string name, const AttributeValue &value){
        m_factory.Set(name, value);
    }

    Ptr<Application> AltRouteSinkHelper::InstallPriv(Ptr<Node> node) const{
        Ptr<AltRouteSinkApplication> app;  

        app = m_factory.Create<AltRouteSinkApplication>();

        node->AddApplication(app);

        return app;
    } 
}
