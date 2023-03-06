#include "leach-helper.h"

LeachNodeHelper::LeachNodeHelper(){

}

ns3::ApplicationContainer LeachNodeHelper::Install(ns3::Ptr<ns3::Node> node) const{
    return ns3::ApplicationContainer(InstallPriv(node));
}

ns3::ApplicationContainer LeachNodeHelper::Install(ns3::NodeContainer nodes) const{
    ns3::ApplicationContainer apps;
    for(ns3::NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); i++){
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

ns3::ApplicationContainer LeachNodeHelper::Install(std::string nodeName) const{
    ns3::Ptr<ns3::Node> node = ns3::Names::Find<ns3::Node>(nodeName);
    return ns3::ApplicationContainer(InstallPriv(node));
}

ns3::Ptr<ns3::Application> LeachNodeHelper::InstallPriv(ns3::Ptr<ns3::Node> node) const{
    ns3::Ptr<ns3::Application> app = m_factory.Create<LeachNodeApplication>();
    node->AddApplication(app);

    return app;
}

void LeachNodeHelper::SetAttribute(std::string name, const ns3::AttributeValue &value){
    m_factory.Set(name, value);
}
