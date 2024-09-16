#include "ofd/PathObject.h"
#include "ofd/Page.h"
#include "ofd/Document.h"
#include "ofd/Path.h"
#include "ofd/Color.h"
#include "ofd/Pattern.h"

#include "utils/xml.h"

using namespace utils;
using namespace ofd;

// **************** class PathObject ****************

ColorPtr PathObject::DefaultStrokeColor = Color::Instance(0, 0, 0, 255);
ColorPtr PathObject::DefaultFillColor = Color::Instance(0, 0, 0, 0);

PathObject::PathObject(LayerPtr layer)
    : Object(layer, ObjectType::PATH, "PathObject"),
      Stroke(true),
      Fill(false),
      Rule(PathRule::NonZero),
      FillPattern(nullptr),
      FillShading(nullptr),
      FillColor(nullptr),
      StrokeColor(nullptr),
      m_path(nullptr)
{
}

PathObject::~PathObject()
{
}

std::string PathObject::to_string() const
{
    std::string baseInfo = Object::to_string();
    std::string pathInfo = m_path->to_string();
    return baseInfo + "\n" + pathInfo;
}

void PathObject::SetFillColor(ColorPtr fillColor)
{
    if (!fillColor->Equal(DefaultFillColor)) {
        FillColor = fillColor;
    }
}

void PathObject::SetStrokeColor(ColorPtr strokeColor)
{
    if (!strokeColor->Equal(DefaultStrokeColor)) {
        StrokeColor = strokeColor;
    }
}

void PathObject::GenerateAttributesXML(XMLWriter &writer) const
{
    Object::GenerateAttributesXML(writer);

    // -------- <PathObject Stroke="">
    // Optional, default value: true.
    if (!Stroke) {
        writer.WriteAttribute("Stroke", false);
    }

    // -------- <PathObject Fill="">
    // Optional, default value: false.
    if (Fill) {
        writer.WriteAttribute("Fill", true);
    }

    // -------- <PathObject Rule="Even-Odd">
    // Optional, default value: "NonZero".
    if (Rule != PathRule::NonZero) {
        writer.WriteAttribute("Rule", "Even-Odd");
    }
}

void PathObject::GenerateElementsXML(XMLWriter &writer) const
{
    Object::GenerateElementsXML(writer);

    // -------- <FillColor>
    // OFD (section 9.1) P52. Page.xsd
    // Optional.
    if (FillShading != nullptr || FillPattern != nullptr || FillColor != nullptr) {

        writer.StartElement("FillColor");
        {

            if (FillShading != nullptr) {
                ;
            } else if (FillPattern != nullptr) {
                FillPattern->WritePatternXML(writer);
            } else {
                FillColor->WriteColorXML(writer);
            }
        }
        writer.EndElement();
    }

    // -------- <StrokeColor>
    // OFD (section 9.1) P52. Page.xsd
    // Optional.
    if (StrokeColor != nullptr) {
        writer.StartElement("StrokeColor");
        {
            StrokeColor->WriteColorXML(writer);
        }
        writer.EndElement();
    }

    // -------- <AbbreviatedData>
    // OFD (section 9.1) P53. Page.xsd
    // Required.
    writer.StartElement("AbbreviatedData");
    {
        std::string pathData = m_path->ToPathData();
        writer.WriteString(pathData);
        //std::stringstream ss;
        //size_t numSubpaths = m_path->GetNumSubpaths();
        ////LOG(DEBUG) << "AbbreviateData: numSubpaths:" << numSubpaths;
        //if ( numSubpaths > 0 ){
        //for ( size_t idx = 0 ; idx < numSubpaths ; idx++){
        //SubpathPtr subpath = m_path->GetSubpath(idx);
        //if ( subpath == nullptr ) continue;
        //size_t numPoints = subpath->GetNumPoints();
        ////LOG(DEBUG) << "AbbreviateData: numPoints:" << numPoints;
        //if ( numPoints < 2 ) continue;

        //const Point &startPoint = subpath->GetFirstPoint();
        //if ( idx == 0 ){
        //ss << "S " << startPoint.x << " " << startPoint.y << " ";
        //} else {
        //ss << "M " << startPoint.x << " " << startPoint.y << " ";
        //}
        //for ( size_t n = 1 ; n < numPoints ; n++ ){
        //const Point &p = subpath->GetPoint(n);
        //ss << "L " << p.x << " " << p.y << " ";
        //}
        //if ( subpath->IsClosed() ){
        //ss << "C ";
        //}
        //}
        //writer.WriteString(ss.str());
        //}
    }
    writer.EndElement();
}

bool PathObject::FromAttributesXML(XMLElementPtr objectElement)
{
    bool ok = false;
    if (Object::FromAttributesXML(objectElement)) {

        bool exist = false;

        // -------- <PathObject Stroke="">
        // Optional, default value: true.
        Stroke = true;
        std::tie(Stroke, std::ignore) = objectElement->GetBooleanAttribute("Stroke");

        // -------- <PathObject Fill="">
        // Optional, default value: false.
        Fill = false;
        std::tie(Fill, std::ignore) = objectElement->GetBooleanAttribute("Fill");

        // -------- <PathObject Rule="Even-Odd">
        // Optional, default value: "NonZero".
        std::string strRule;
        std::tie(strRule, exist) = objectElement->GetStringAttribute("Rule");
        Rule = PathRule::NonZero;
        if (exist) {
            if (strRule == std::string("Even-Odd")) {
                Rule = PathRule::EvenOdd;
            }
        }

        ok = true;
    }

    return ok;
}

bool PathObject::IterateElementsXML(XMLElementPtr childElement)
{
    if (Object::IterateElementsXML(childElement)) {

        std::string childName = childElement->GetName();

        if (childName == "FillColor") {

            utils::XMLElementPtr shadingElement = childElement->GetFirstChildElement();
            if (shadingElement != nullptr) {
                std::string name = shadingElement->GetName();

            } else {
                ColorPtr fillColor = nullptr;
                bool exist = false;
                std::tie(fillColor, exist) = Color::ReadColorXML(childElement);
                if (exist) {
                    FillColor = fillColor;
                }
            }
        } else if (childName == "StrokeColor") {
            ColorPtr strokeColor = nullptr;
            bool exist = false;
            std::tie(strokeColor, exist) = Color::ReadColorXML(childElement);
            if (exist) {
                StrokeColor = strokeColor;
//                LOG(DEBUG) << "Readed stroke color = (" << strokeColor->Value.RGB.Red << "," << strokeColor->Value.RGB.Green << "," << strokeColor->Value.RGB.Blue << ")";
            }
        } else if (childName == "AbbreviatedData") {
            std::string pathData;
            std::tie(pathData, std::ignore) = childElement->GetStringValue();
            m_path = Path::FromPathData(pathData);
        }

        return true;
    }
    return false;
}

void PathObject::RecalculateBoundary()
{
    Boundary = m_path->CalculateBoundary();
}
