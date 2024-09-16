#include <assert.h>
#include "ofd/Color.h"
#include "utils/xml.h"
#include "utils/utils.h"

using namespace ofd;
using namespace utils;

// **************** class ColorSpace ****************

ColorSpacePtr ColorSpace::DefaultInstance = std::make_shared<ColorSpace>();
ColorSpaceMap ColorSpace::GlobalColorSpaces;

void ColorSpace::GlobalClearColorSpaces(){
    GlobalColorSpaces.clear();
}

uint64_t ColorSpace::GlobalAddColorSpace(ColorSpacePtr colorSpace){
    uint64_t refID = GlobalColorSpaces.size() + 1;
    colorSpace->SetRefID(refID);
    GlobalColorSpaces.insert(ColorSpaceMap::value_type(refID, colorSpace));
    return refID;
}

ColorSpacePtr ColorSpace::GlobalGetColorSpace(uint64_t refID){
    if ( refID == 0 ){
        return ColorSpace::DefaultInstance;
    }
    auto it = GlobalColorSpaces.find(refID);
    if ( it != GlobalColorSpaces.end() ){
        return it->second;
    } else {
        return nullptr;
    }
}

ColorSpace::ColorSpace() : 
    Type(ColorSpaceType::RGB), BPC(8),
    m_refID(0){
}

// **************** class Color ****************

Color::Color(uint32_t gray, ColorSpacePtr colorSpace, uint32_t alpha) :
    weakColorSpace(colorSpace),
    Value(ColorValue(gray)),
    Index(0), Alpha(alpha),
    m_bUsePalette(false){
}

Color::Color(uint32_t r, uint32_t g, uint32_t b, ColorSpacePtr colorSpace, uint32_t alpha) :
    weakColorSpace(colorSpace),
    Value(ColorValue(r, g, b)),
    Index(0), Alpha(alpha),
    m_bUsePalette(false){
}

Color::Color(uint32_t c, uint32_t m, uint32_t y, uint32_t k, ColorSpacePtr colorSpace, uint32_t alpha) :
    weakColorSpace(colorSpace),
    Value(ColorValue(c, m, y, k)),
    Index(0), Alpha(alpha),
    m_bUsePalette(false){
}

Color::Color(const ColorValue &colorValue, ColorSpacePtr colorSpace, uint32_t alpha) :
    weakColorSpace(colorSpace),
    Value(colorValue),
    Index(0), Alpha(alpha),
    m_bUsePalette(false){
}

Color::Color(ColorSpacePtr colorSpace, uint32_t index, uint32_t alpha) :
    weakColorSpace(colorSpace), 
    Index(index), Alpha(alpha),
    m_bUsePalette(true){
}

// ================ Color::Equal() ================
bool Color::Equal(ColorPtr color) const {
    if ( Alpha != color->Alpha ) return false;
    ColorSpacePtr cs1 = GetColorSpace();
    ColorSpacePtr cs2 = color->GetColorSpace();
    if ( cs1->GetRefID() != cs2->GetRefID() ) return false;
    if ( m_bUsePalette == color->m_bUsePalette ){
        if ( m_bUsePalette ){
            return Index == color->Index;
        } else {
            return Value == color->Value;
        }
    } else {
        return false;
    }
};

// ================ Color::GetRGBA() ================
std::tuple<double, double, double, double> Color::GetRGBA()const{
    double r, g, b, a;
    std::tie(r, g, b) = Value.RGB.GetRGB();
    a = (double)Alpha / 255.0;
    return std::make_tuple(r, g, b, a);
}

// ================ Color::GetCMYK() ================
std::tuple<double, double, double, double> Color::GetCMYK()const{
    return Value.CMYK.GetCMYK();
}

// ================ Color::GetColorSpace() ================
ColorSpacePtr Color::GetColorSpace() const {
    if ( !weakColorSpace.expired() ){
        return weakColorSpace.lock();
    } else {
        return ColorSpace::DefaultInstance;
    }
}

// ================ Color::WriteColorXML() ================
void Color::WriteColorXML(XMLWriter &writer) const{

    if ( !m_bUsePalette ){
        // -------- <Color Value="">
        // Optional

        ColorSpacePtr colorSpace = GetColorSpace();
        assert(colorSpace != nullptr);

        size_t numChannels = 0;
        if ( colorSpace->Type == ColorSpaceType::RGB ){
            numChannels = 3;
        } else if ( colorSpace->Type == ColorSpaceType::GRAY ){
            numChannels = 1;
        } else if ( colorSpace->Type == ColorSpaceType::CMYK ){
            numChannels = 4;
        } else {
            throw "Unknown ColorSpace Type : " + std::to_string((int)colorSpace->Type);
        }

        std::stringstream ss;
        for ( size_t k = 0 ; k < numChannels - 1 ; k++ ){
            ss << Value.Values[k] << " ";
        }
        if ( numChannels > 0 ){
            ss << Value.Values[numChannels - 1];
        }


        writer.WriteAttribute("Value", ss.str());

    } else {
        // -------- <Color Index="">
        // Optional
        writer.WriteAttribute("Index", (uint64_t)Index);
    }
    // -------- <Color ColorSpace="">
    // Optional
    if ( !weakColorSpace.expired() ){
        ColorSpacePtr colorSpace = weakColorSpace.lock();
        uint64_t refID = colorSpace->GetRefID();
        if ( refID > 0 ){
            writer.WriteAttribute("ColorSpace", refID);
        }
    }

    // -------- <Color Alpha="">
    // Optional
    if ( Alpha != 255 ){
        writer.WriteAttribute("Alpha", (uint64_t)Alpha);
    }
}

// ================ static Color::ReadColorXML() ================
std::tuple<ColorPtr, bool> Color::ReadColorXML(XMLElementPtr colorElement){
    bool ok = false;

    ColorPtr color = nullptr;

    ColorValue colorValue;
    uint32_t index = 0;

    bool exist = false;

    ColorSpacePtr colorSpace = nullptr;
    uint64_t refID = 0;
    std::tie(refID, exist) = colorElement->GetIntAttribute("ColorSpace");
    if ( exist && refID > 0 ){
        colorSpace = ColorSpace::GlobalGetColorSpace(refID);
    }
    if ( colorSpace == nullptr ){
        colorSpace = ColorSpace::DefaultInstance;
    }

    uint32_t alpha = 255;
    std::tie(alpha, exist) = colorElement->GetIntAttribute("Alpha");
    if ( !exist ) alpha = 255;

    std::string valueData;
    std::tie(valueData, exist) = colorElement->GetStringAttribute("Value");
    if ( exist ){
        std::vector<std::string> tokens = utils::SplitString(valueData);
        if ( tokens.size() == 3 ){
            uint32_t r = atoi(tokens[0].c_str());
            uint32_t g = atoi(tokens[1].c_str());
            uint32_t b = atoi(tokens[2].c_str());
            color = Color::Instance(r, g, b, colorSpace, alpha);
        }
    } else {
        std::tie(index, exist) = colorElement->GetIntAttribute("Index");
        if ( !exist ){
            return std::make_tuple(nullptr, false);
        }
        color = Color::Instance(colorSpace, index, alpha);
    }

    ok = true;


    return std::make_tuple(color, ok);
}
