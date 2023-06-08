#include "leach-tag-list.h"



namespace ns3{
    NS_LOG_COMPONENT_DEFINE("LeachTagList");

    LeachTagList::LeachTagList(){
        NS_LOG_FUNCTION(this);

        m_list = new std::vector<LeachTag>();
    }

    LeachTagList::~LeachTagList(){
        NS_LOG_FUNCTION(this);

        delete m_list;
        m_list = nullptr;    
    }


    void LeachTagList::PushTag(LeachTag tag){
        NS_LOG_FUNCTION(this);

        m_list->push_back(tag);
    }

    LeachTag LeachTagList::BackTag(){
        NS_LOG_FUNCTION(this);

        LeachTag tag = m_list->back();

        return tag;
    }

    LeachTag LeachTagList::PopTag(){
        NS_LOG_FUNCTION(this);

        LeachTag tag = m_list->back();
        m_list->pop_back();
        return tag;
    }

    uint32_t LeachTagList::GetSize(){
        return m_list->size();
    }


    ns3::TypeId LeachTagList::GetTypeId() const{
        NS_LOG_FUNCTION(this);

       static ns3::TypeId tid = ns3::TypeId("ns3::LeachTagList")
                                    .SetParent<Tag>()
                                    .SetGroupName("Leach")
                                    .AddConstructor<LeachTagList>();

       return tid;
    }

    ns3::TypeId LeachTagList::GetInstanceTypeId() const{
        NS_LOG_FUNCTION(this);

        return GetTypeId();
    }

    uint32_t LeachTagList::GetSerializedSize() const{
        NS_LOG_FUNCTION(this);

        uint8_t size = sizeof(m_list);
        return size;
    }

    void LeachTagList::Serialize(TagBuffer buffer) const{
        NS_LOG_FUNCTION(this);

        size_t ptrSize = sizeof(m_list); 
        uint8_t* ptrBytes = new uint8_t[ptrSize];
        std::memcpy(ptrBytes, &m_list, ptrSize);
        
        buffer.Write(ptrBytes, ptrSize);
        
    }

    void LeachTagList::Deserialize(TagBuffer buffer){
        NS_LOG_FUNCTION(this);

        size_t ptrSize = sizeof(m_list); 
        uint8_t* ptrBytes = new uint8_t[ptrSize];
        buffer.Read(ptrBytes, ptrSize);

        std::vector<LeachTag> *tmp_list;
        std::memcpy(&tmp_list, ptrBytes, ptrSize);
        m_list = tmp_list;
    }

    void LeachTagList::Print(std::ostream &os) const{
        NS_LOG_FUNCTION(this);

        os << "List Size: " << m_list->size()
            << "\n";
    }
}
