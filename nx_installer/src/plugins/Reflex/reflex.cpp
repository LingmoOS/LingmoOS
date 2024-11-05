#include "reflex.h"
class ClassInfo;

std::string Factory::justAddedClass("");//default value is empty when no class is added to the register map

Factory::~Factory()
{
    for (; m_classInfoMap->size() > 0;) {
        delete (m_classInfoMap);
        m_classInfoMap->erase(0);
    }
}

bool Factory::Register(ClassInfo * pCInfo)
{
    if (!m_classInfoMap) {
        m_classInfoMap = new QList<QPair<std::string, ClassInfo*>>();
    }
    if (!pCInfo) {
        return false;
    }
    for (int i = 0; i < m_classInfoMap->size(); i++) {
        if (m_classInfoMap->at(i).first == pCInfo->GetClassName()) {
            return false;
        }
    }
    m_classInfoMap->append(qMakePair((std::string)pCInfo->GetClassName(), (ClassInfo*)pCInfo));
    justAddedClass = pCInfo->GetClassName();
    return true;
}

bool Factory::Unregister(std::string className)
{
//    if(m_classInfoMap) {
//        auto iter = m_classInfoMap->find(className);
//        if(m_classInfoMap->end() != iter) {
//            m_classInfoMap->erase(iter);
//            return true;
//        }
//    }

    if (m_classInfoMap) {
        for (int i = 0; i < m_classInfoMap->size(); i++) {
            if (m_classInfoMap->at(i).first == className) {
                if (m_classInfoMap->size() != i) {
                    m_classInfoMap->removeAt(i);
                    return true;
                }
            }
        }
    }
    //unregister failed because the class to be unregistered is not registered at first place
    return false;
}

std::string Factory::getNewAddedClassName()
{
    return justAddedClass;
}

std::vector<std::string> *Factory::GetClassNames()
{
    std::vector<std::string>* keys = new std::vector<std::string>();
    if (m_classInfoMap){
        for (auto c_iter = m_classInfoMap->begin(); c_iter != m_classInfoMap->end(); ++c_iter){
            keys->push_back(c_iter->first);
        }
    }
    return keys;
}

ReflexBase * Factory::CreateObject(std::string className)
{
    if(!m_classInfoMap)
        return NULL;
//    std::map<std::string, ClassInfo*>::const_iterator c_iter = m_classInfoMap->find(className);
    for (int i = 0; i < m_classInfoMap->size(); i++) {
        if (m_classInfoMap->at(i).first == className) {
            if (m_classInfoMap->size() != i) {
                return m_classInfoMap->at(i).second->CreateObject();
            }
        }
    }
//    if (m_classInfoMap->end() != c_iter) {
//        return c_iter->second->CreateObject();
//    }
    return NULL;
}

ClassInfo * Factory::GetClassInfo(std::string className)
{
    if(!m_classInfoMap)
        return NULL;

    for (int i = 0; i < m_classInfoMap->size(); i++) {
        if (m_classInfoMap->at(i).first == className) {
            if (m_classInfoMap->size() != i) {
                return m_classInfoMap->at(i).second;
            }
        }
    }
//    std::map<std::string, ClassInfo*>::const_iterator c_iter = m_classInfoMap->find(className);
//    if (m_classInfoMap->end() != c_iter) {
//        return c_iter->second;
//    }
    return NULL;
}
