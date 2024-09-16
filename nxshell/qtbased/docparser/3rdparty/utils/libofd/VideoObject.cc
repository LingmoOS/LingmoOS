#include "ofd/VideoObject.h"
#include "ofd/Page.h"
#include "ofd/Document.h"


#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class OFDVideoObject ****************

VideoObject::VideoObject(LayerPtr layer) :
    Object(layer, ObjectType::VIDEO, "VideoObject"){
}

VideoObject::~VideoObject(){
}

std::string VideoObject::to_string() const{
    return Object::to_string();
}

void VideoObject::GenerateAttributesXML(XMLWriter &writer) const{
    Object::GenerateAttributesXML(writer);
}

void VideoObject::GenerateElementsXML(XMLWriter &writer) const{
    Object::GenerateElementsXML(writer);
}

bool VideoObject::FromAttributesXML(XMLElementPtr objectElement){
    if ( Object::FromAttributesXML(objectElement) ){
        return true;
    }
    return false;
}

bool VideoObject::IterateElementsXML(XMLElementPtr childElement){
    if ( Object::IterateElementsXML(childElement) ){
        return true;
    }
    return false;
}

void VideoObject::RecalculateBoundary(){
}
