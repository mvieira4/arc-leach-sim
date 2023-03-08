#include "leach-helper.h"





namespace ns3{
    LeachNodeHelper::LeachNodeHelper(){
        m_factory.SetTypeId(LeachNodeApplication::GetTypeId());
    }

    ApplicationContainer LeachNodeHelper::Install(Ptr<Node> node) const{
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer LeachNodeHelper::Install(NodeContainer nodes) const{
        ApplicationContainer apps;
        for(NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); i++){
            apps.Add(InstallPriv(*i));
        }

        return apps;
    }

    ApplicationContainer LeachNodeHelper::Install(std::string nodeName) const{
        Ptr<Node> node = Names::Find<Node>(nodeName);
        return ApplicationContainer(InstallPriv(node));
    } 

    void LeachNodeHelper::SetAttribute(std::string name, const AttributeValue &value){
        m_factory.Set(name, value);
    }

    Ptr<Application> LeachNodeHelper::InstallPriv(Ptr<Node> node) const{
        Ptr<Application> app;  
        app = m_factory.Create<LeachNodeApplication>();
        node->AddApplication(app);

        return app;
    } 
}
