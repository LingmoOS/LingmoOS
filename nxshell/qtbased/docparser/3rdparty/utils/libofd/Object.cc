#include "ofd/Object.h"
#include "ofd/Layer.h"
#include "ofd/Page.h"
#include "ofd/Document.h"
#include "utils/xml.h"
#include "utils/utils.h"

using namespace ofd;
using namespace utils;

namespace ofd{
void ConcatCTM(double *ctm, double a, double b, double c,
			 double d, double e, double f) {
  double a1 = ctm[0];
  double b1 = ctm[1];
  double c1 = ctm[2];
  double d1 = ctm[3];

  ctm[0] = a * a1 + b * c1;
  ctm[1] = a * b1 + b * d1;
  ctm[2] = c * a1 + d * c1;
  ctm[3] = c * b1 + d * d1;
  ctm[4] = e * a1 + f * c1 + ctm[4];
  ctm[5] = e * b1 + f * d1 + ctm[5];
}
}

Object::Object(LayerPtr layer, ObjectType objectType, const std::string& objectLabel) :
    ID(0), Type(objectType),
    ObjectLabel(objectLabel),
    Visible(true), LineWidth(0.353), Alpha(255),
    m_layer(layer){
        CTM[0] = 1.0;
        CTM[1] = 0.0;
        CTM[2] = 0.0;
        CTM[3] = 1.0;
        CTM[4] = 0.0;
        CTM[5] = 0.0;
}

Object::~Object(){
}

const LayerPtr Object::GetLayer() const {
    return m_layer.lock();
}

LayerPtr Object::GetLayer(){
    return m_layer.lock();
}

const PagePtr Object::GetPage() const{
    return GetLayer()->GetPage();
}

PagePtr Object::GetPage(){
    return GetLayer()->GetPage();
}

const DocumentPtr Object::GetDocument() const{
    const PagePtr page = GetPage();
    //assert(page != nullptr);
    if ( page != nullptr ){
        const DocumentPtr document = page->GetDocument();
        return document;
    }
    return nullptr;
}

DocumentPtr Object::GetDocument(){
    PagePtr page = GetPage();
    //assert(page != nullptr);
    if ( page != nullptr ){
        DocumentPtr document = page->GetDocument();
        return document;
    }
    return nullptr;
}

const ResourcePtr Object::GetDocumentRes() const{
    const DocumentPtr document = GetDocument();
    //assert(document != nullptr);
    if ( document != nullptr ){
        const Document::CommonData &commonData = document->GetCommonData();
        const ResourcePtr documentRes = commonData.DocumentRes;
        //assert(documentRes != nullptr);
        return documentRes;
    }

    return nullptr;
}

ResourcePtr Object::GetDocumentRes(){
    DocumentPtr document = GetDocument();
    //assert(document != nullptr);
    if ( document != nullptr ){
        Document::CommonData &commonData = document->GetCommonData();
        ResourcePtr documentRes = commonData.DocumentRes;
        //assert(documentRes != nullptr);
        return documentRes;
    }

    return nullptr;
}

const ResourcePtr Object::GetPublicRes() const{
    const DocumentPtr document = GetDocument();
    //assert(document != nullptr);
    if ( document != nullptr ){
        const Document::CommonData &commonData = document->GetCommonData();
        const ResourcePtr publicRes = commonData.PublicRes;
        //assert(publicRes != nullptr);
        return publicRes;
    }

    return nullptr;
}

ResourcePtr Object::GetPublicRes(){
    DocumentPtr document = GetDocument();
    //assert(document != nullptr);
    if ( document != nullptr ){
        Document::CommonData &commonData = document->GetCommonData();
        ResourcePtr publicRes = commonData.PublicRes;
        //assert(publicRes != nullptr);
        return publicRes;
    }

    return nullptr;
}

std::string Object::to_string() const {
    std::stringstream ss;
    ss << "[" << ObjectLabel << "] " 
        << "ID:" << ID << " | "
        << "Type: " << (int)Type << " | "
        << "CTM:(" << CTM[0] << "," << CTM[1] << "," << CTM[2] << "," << CTM[3] << "," << CTM[4] << "," << CTM[5] << ") | "
        << "Boundary:" << Boundary.to_string() << " | "
        << "Alpha:" << Alpha << " | "
        << "Visible:" << Visible << " | "
        << "LineWidth:" << LineWidth << " | "
        ;

    return ss.str();
}

// Called by OFDPage::ImplCls::generateContentXML()
void Object::GenerateXML(XMLWriter &writer) const{
    writer.StartElement(ObjectLabel);{
        GenerateAttributesXML(writer);
        GenerateElementsXML(writer);
    } writer.EndElement();
}

void Object::GenerateAttributesXML(XMLWriter &writer) const{

    // -------- GraphUnit attributes --------
    // OFD (section 8.5) P50. Page.xsd.

    // -------- <Object ID="">
    // Required
    writer.WriteAttribute("ID", ID);

    // -------- <Object Boundary="">
    // Required.
    writer.WriteAttribute("Boundary", Boundary.to_xmlstring());

    // -------- <Object Name="">
    // Optional
    if ( !Name.empty() ){
        writer.WriteAttribute("Name", Name);
    }

    // -------- <Object Visible="">
    // Optional, default value: true.
    if ( !Visible ){
        writer.WriteAttribute("Visible", "false");
    }

    // TODO
    // -------- <Object CTM="">
    if ( CTM[0] != 1.0 || CTM[1] != 0.0 ||
            CTM[2] != 0.0 || CTM[3] != 1.0 ||
            CTM[4] != 0.0 || CTM[5] != 0.0 ){
        std::stringstream ss;
        ss << CTM[0] << " " << CTM[1] << " "
            << CTM[2] << " " << CTM[3] << " "
            << CTM[4] << " " << CTM[5];
        writer.WriteAttribute("CTM", ss.str());
    }

    // TODO
    // -------- <Object LineWidth="">
    if ( LineWidth > 0 ){
        writer.WriteAttribute("LineWidth", LineWidth, 3);
    }

    // -------- <Object Alpha="">
    // Optional, default value: 0.
    if ( Alpha > 0 ){
        writer.WriteAttribute("Alpha", std::to_string(Alpha));
    }
}

void Object::GenerateElementsXML(XMLWriter &writer) const{

    // -------- GraphUnit elements --------
    // OFD P50. Page.xsd.

    // TODO
    // -------- <Actions>

    // TODO
    // -------- <Clips>

}

bool Object::FromXML(XMLElementPtr objectElement){
    bool ok = true;

    ok = FromAttributesXML(objectElement);
    if ( ok ){
        FromElementsXML(objectElement);
    } else {
//        LOG(WARNING) << "FromAttributesXML() return false;";
    }

    return ok;
}

// -------- CT_GraphicUnit --------
// OFD (section 8.5) P50.
bool Object::FromAttributesXML(utils::XMLElementPtr objectElement){
    bool ok = true;

    // -------- <TextObject ID="">
    // Required.
    bool exist = false;
    std::tie(ID, exist) = objectElement->GetIntAttribute("ID");
    if ( !exist ){
//        LOG(ERROR) << "Attribute ID is required in Object XML.";
        return false;
    }

    // -------- <Object Boundary="">
    // Required.
    std::string strBoundary;
    std::tie(strBoundary, exist) = objectElement->GetStringAttribute("Boundary");
    //LOG(INFO) << "Boundary: " << strBoundary;

    if ( !exist ){
//        LOG(ERROR) << "Attribute ID is required in Object XML.";
        return false;
    }
    std::vector<std::string> tokens = utils::SplitString(strBoundary);
    if ( tokens.size() >= 4 ){
        double left = atof(tokens[0].c_str());
        double top = atof(tokens[1].c_str());
        double width = atof(tokens[2].c_str());
        double height = atof(tokens[3].c_str());
        if ( width < 0 ){
            left -= width;
            width = fabs(width);
        }
        if ( height < 0 ){
            top -= height;
            height = fabs(height);
        }
        Boundary.XMin = left;
        Boundary.YMin = top;
        Boundary.XMax = left + width;
        Boundary.YMax = top + height;
        ok = true;
    } else {
//        LOG(ERROR) << "Box String tokens size >= 4 failed. boxString:" << strBoundary << " element name: " << objectElement->GetName();
        return false;
    }

    // -------- <Object LineWidth="">
    // Optional
    std::tie(LineWidth, std::ignore) = objectElement->GetFloatAttribute("LineWidth");

    // -------- <Object Alpha="">
    // Optional
    uint64_t alpha;
    std::tie(alpha, std::ignore) = objectElement->GetFloatAttribute("Alpha");
    Alpha = alpha;

    // -------- <Object Name="">
    // Optional

    // -------- <Visible Name="">
    // Optional

    // -------- <CTM Name="">
    // Optional
    std::string ctm;
    std::tie(ctm, std::ignore) = objectElement->GetStringAttribute("CTM");
    if ( !ctm.empty() ){
        std::vector<std::string> tokens = utils::SplitString(ctm);
        if ( tokens.size() == 6 ){
            CTM[0] = atof(tokens[0].c_str());
            CTM[1] = atof(tokens[1].c_str());
            CTM[2] = atof(tokens[2].c_str());
            CTM[3] = atof(tokens[3].c_str());
            CTM[4] = atof(tokens[4].c_str());
            CTM[5] = atof(tokens[5].c_str());
        }
    }

    // -------- <DrawParam Name="">
    // Optional

    // -------- <Cap Name="">
    // Optional

    // -------- <Join Name="">
    // Optional

    // -------- <MiterLimit Name="">
    // Optional

    // -------- <DashOffset Name="">
    // Optional

    // -------- <DashPattern Name="">
    // Optional

    return ok;
}

bool Object::FromElementsXML(utils::XMLElementPtr objectElement){
    bool ok = true;

    XMLElementPtr childElement = objectElement->GetFirstChildElement();
    while ( childElement != nullptr ){

        IterateElementsXML(childElement);

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// -------- CT_GraphicUnit --------
// OFD (section 8.5) P50.
bool Object::IterateElementsXML(utils::XMLElementPtr childElement){
    bool ok = true;


    // -------- <Actions Name="">
    // Optional

    // -------- <Clips Name="">
    // Optional


    return ok;
}
