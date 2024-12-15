#include "ofd/Object.h"
#include "ofd/Layer.h"
#include "ofd/Page.h"

using namespace ofd;
using namespace utils;

uint64_t numObjects = 0;

Layer::Layer(PagePtr page) :
    ID(0), Type(LayerType::BODY),
    m_page(page){
}

Layer::~Layer(){
}

void Layer::AddObject(ObjectPtr object) {
    if ( object != nullptr ){
        object->ID = numObjects++;
        object->RecalculateBoundary();
        m_objects.push_back(object);
    }
}
