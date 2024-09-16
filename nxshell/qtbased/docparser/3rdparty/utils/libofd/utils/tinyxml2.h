#ifndef __UTILS_TINYXML2_H__
#define __UTILS_TINYXML2_H__

#include <tuple>
#include <string>

namespace tinyxml2{
    class XMLElement;
}

extern std::string GetChildElements(const tinyxml2::XMLElement *element);
extern std::tuple<double, double, double, double, bool> parsePhysicalBoxElement(const tinyxml2::XMLElement *physicalBoxElement);

#define GetChildElementText(element, childname, variable) \
    { \
        const XMLElement *childElement = element->FirstChildElement(#childname); \
        if (childElement != NULL ){ \
            variable = childElement->GetText(); \
        } \
    } \
    
#endif // __UTILS_TINYXML2_H__ 
