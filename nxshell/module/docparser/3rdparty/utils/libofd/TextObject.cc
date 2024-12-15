#include <assert.h>
#include "ofd/TextObject.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "ofd/Layer.h"
#include "ofd/Color.h"
#include "ofd/Resource.h"
#include "utils/xml.h"


using namespace ofd;

ColorPtr TextObject::DefaultStrokeColor = Color::Instance(0, 0, 0, 0);
ColorPtr TextObject::DefaultFillColor = Color::Instance(0, 0, 0, 255);

TextObject::TextObject(LayerPtr layer)
    : Object(layer, ObjectType::TEXT, "TextObject"),
      FontSize(12.0),
      Stroke(false),
      Fill(true),
      HScale(1.0),
      RD(Text::ReadDirection::ANGLE0),
      CD(Text::CharDirection::ANGLE0),
      Italic(false),
      FillColor(nullptr),
      StrokeColor(nullptr)
{
}

TextObject::~TextObject()
{
}

void TextObject::SetFillColor(ColorPtr fillColor)
{
    if (!fillColor->Equal(DefaultFillColor)) {
        FillColor = fillColor;
    }
}

void TextObject::SetStrokeColor(ColorPtr strokeColor)
{
    if (!strokeColor->Equal(DefaultStrokeColor)) {
        StrokeColor = strokeColor;
    }
}

std::string TextObject::to_string() const
{
    return Object::to_string();
}

// -------- <TextObject>
// OFD (section 11.2) P63. Page.xsd.
void TextObject::GenerateAttributesXML(utils::XMLWriter &writer) const
{
    Object::GenerateAttributesXML(writer);

    // FIXME
    // -------- <TextObject Font="">
    // Required.

//    LOG(WARNING) << "Attribute Font is required in TextObject XML.";

    // -------- <TextObject Size="">
    // Required.
    writer.WriteAttribute("Size", FontSize, 1);

    // -------- <TextObject Stroke="">
    // Optional, default value: false.
    if (Stroke) {
        writer.WriteAttribute("Stroke", true);
    }

    // -------- <TextObject Fill="">
    // Optional, default value: true.
    if (!Fill) {
        writer.WriteAttribute("Fill", false);
    }

    // -------- <Textobject HScale="">
    // Optional, default value: 1.0
    if (fabs(HScale - 1.0) > 0.0000001) {
        writer.WriteAttribute("HScale", HScale);
    }
}

// -------- <TextObject>
// OFD (section 11.2) P63. Page.xsd.
void TextObject::GenerateElementsXML(utils::XMLWriter &writer) const
{
    Object::GenerateElementsXML(writer);

    // -------- <FillColor>
    // OFD (section 11.3) P65. Page.xsd
    // Optional.
    if (FillColor != nullptr) {
        writer.StartElement("FillColor");
        {
            FillColor->WriteColorXML(writer);
        }
        writer.EndElement();
    }

    // -------- <StrokeColor>
    // OFD (section 11.3) P65. Page.xsd
    // Optional.
    if (StrokeColor != nullptr) {
        writer.StartElement("StrokeColor");
        {
            StrokeColor->WriteColorXML(writer);
        }
        writer.EndElement();
    }

    // -------- <TextCode>
    // OFD (section 11.3) P65. Page.xsd
    // Required.
    for (auto textCode : m_textCodes) {
        writer.StartElement("TextCode");
        {

            // -------- <TextCode X="'>
            // Optional.
            writer.WriteAttribute("X", textCode.X);

            // -------- <TextCode Y="'>
            // Optional.
            writer.WriteAttribute("Y", textCode.Y);

            // -------- <TextCode DeltaX="'>
            // Optional.
            std::string strDeltaX;
            for (auto d : textCode.DeltaX) {
                strDeltaX += std::to_string(d) + " ";
            }
            writer.WriteAttribute("DeltaX", strDeltaX);

            // -------- <TextCode DeltaY="'>
            // Optional.
            std::string strDeltaY;
            for (auto d : textCode.DeltaY) {
                strDeltaY += std::to_string(d) + " ";
            }
            writer.WriteAttribute("DeltaY", strDeltaY);

            writer.WriteString(textCode.Text);
        }
        writer.EndElement();
    }
}

// -------- TextObject --------
// OFD (section 11.2) P63
bool TextObject::FromAttributesXML(utils::XMLElementPtr objectElement)
{
    if (!Object::FromAttributesXML(objectElement)) {
        return false;
    }

    bool ok = true;

    // -------- <TextObject Font="">
    // Required.
    uint64_t fontID = 0;
    bool exist = false;
    std::tie(fontID, exist) = objectElement->GetIntAttribute("Font");
    if (!exist) {
//        LOG(ERROR) << "Attribute Font is required in TextObject XML.";
        return false;
    } else {
        const ResourcePtr documentRes = GetDocumentRes();
        assert(documentRes != nullptr);
    }

    // -------- <TextObject Size="">
    // Required.
    std::tie(FontSize, exist) = objectElement->GetIntAttribute("Size");
    if (!exist) {
//        LOG(ERROR) << "Attribute Size is required in TextObject XML.";
        return false;
    }

    return ok;
}

// -------- TextObject --------
// OFD (section 11.2) P63
bool TextObject::IterateElementsXML(utils::XMLElementPtr childElement)
{
    if (!Object::IterateElementsXML(childElement)) {
        return false;
    }

    bool ok = false;

    std::string childName = childElement->GetName();

    if (childName == "TextCode") {

        Text::TextCode textCode;

        bool exist = false;
        std::tie(textCode.X, exist) = childElement->GetFloatAttribute("X");
        if (!exist) {
//            LOG(ERROR) << "Attribute X is required in TextCode XML";
            return false;
        }

        std::tie(textCode.Y, exist) = childElement->GetFloatAttribute("Y");
        if (!exist) {
//            LOG(ERROR) << "Attribute Y is required in TextCode XML";
            return false;
        }

        // TODO
        //std::string strDeltaX;
        //std::tie(strDeltaX, std::ignore) = childElement->GetStringAttribute("DeltaX");
        //std::string strDeltaY;
        //std::tie(strDeltaY, std::ignore) = childElement->GetStringAttribute("DeltaY");

        std::tie(textCode.Text, std::ignore) = childElement->GetStringValue();

//        LOG(DEBUG) << "X: " << textCode.X << " Y: " << textCode.Y << " Text: " << textCode.Text;
        m_textCodes.push_back(textCode);

        ok = true;
    } else if (childName == "FillColor") {
        ColorPtr fillColor = nullptr;
        bool exist = false;
        std::tie(fillColor, exist) = Color::ReadColorXML(childElement);
        if (exist) {
            FillColor = fillColor;
        }
    } else if (childName == "StrokeColor") {
        ColorPtr strokeColor = nullptr;
        bool exist = false;
        std::tie(strokeColor, exist) = Color::ReadColorXML(childElement);
        if (exist) {
            StrokeColor = strokeColor;
//            LOG(DEBUG) << "Readed stroke color = (" << strokeColor->Value.RGB.Red << "," << strokeColor->Value.RGB.Green << "," << strokeColor->Value.RGB.Blue << ")";
        }
    }

    return ok;
}

void TextObject::RecalculateBoundary()
{
}
