#include "ofd/CompositeObject.h"
#include "ofd/Page.h"
#include "ofd/Document.h"

#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDCompositeObject ****************

CompositeObject::CompositeObject(LayerPtr layer) :
    Object(layer, ObjectType::COMPOSITE, "CompositeObject"){
}

CompositeObject::~CompositeObject(){
}

std::string CompositeObject::to_string() const{
    return Object::to_string();
}

void CompositeObject::GenerateAttributesXML(XMLWriter &writer) const{
    Object::GenerateAttributesXML(writer);
}

void CompositeObject::GenerateElementsXML(XMLWriter &writer) const{
    Object::GenerateElementsXML(writer);
}

bool CompositeObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( Object::FromAttributesXML(objectElement) ){
        return true;
    }
    return false;
}

bool CompositeObject::IterateElementsXML(XMLElementPtr childElement){
    if ( Object::IterateElementsXML(childElement) ){
        return true;
    }
    return false;
}

void CompositeObject::RecalculateBoundary(){
    Boundary.clear();

    //size_t numObjects = m_objects.size();
    //if ( numObjects == 0 ) return;

    //m_objects[0]->RecalculateBoundary();
    //Boundary = m_objects[0].Boundary;
    //for ( size_t i = 1 ; i < numObjects ; i++ ){
        //obj->RecalculateBoundary();
        //if ( obj->Boundary.XMin < Boundary.XMin ){
            //Boundary.XMin = obj->Boundary.XMin;
        //}
    //}
}
