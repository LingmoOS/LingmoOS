#include <sstream>
#include <assert.h>
#include "ofd/Package.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "ofd/Layer.h"
#include "ofd/TextObject.h"
#include "ofd/PathObject.h"
#include "ofd/VideoObject.h"
#include "ofd/CompositeObject.h"
#include "utils/xml.h"


using namespace ofd;
using namespace utils;

Page::Page(DocumentPtr document)
    : ID(0),
      m_document(document),
      m_opened(false)
{
}

Page::~Page()
{
}

PagePtr Page::CreateNewPage(DocumentPtr document)
{
    PagePtr page = std::shared_ptr<Page>(new Page(document));
    return page;
}

std::string Page::to_string() const
{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDPage ========\n";
    ss << " ID: " << ID << "\n";
    ss << " BaseLoc: " << BaseLoc << "\n";
    ss << " Layers: " << m_layers.size() << "\n";
    if (m_layers.size() > 0) {
        // FIXME
        LayerPtr bodyLayer = m_layers[0];
        assert(bodyLayer != nullptr);
    }
    ss << " Area: " << Area.to_string() << "\n";

    ss << std::endl;
    return ss.str();
}

bool Page::Open()
{
    if (m_opened) return true;
    if (BaseLoc.empty()) return false;
    DocumentPtr document = m_document.lock();
    if (document == nullptr) return false;

    const PackagePtr package = document->GetPackage();
    if (package == nullptr) return false;

    std::string docRoot = document->GetDocRoot();
    std::string pageXMLFile = docRoot + "/" + BaseLoc;
    if (pageXMLFile.length() >= 4 && pageXMLFile.substr(pageXMLFile.length() - 4) != ".xml")
        pageXMLFile.append("/Content.xml");
//    LOG(INFO) << "Try to open zipfile " << pageXMLFile;

    bool ok = false;
    std::string strPageXML;
    std::tie(strPageXML, ok) = package->ReadZipFileString(pageXMLFile);
    if (ok) {
        m_opened = fromPageXML(strPageXML);

        if (m_opened) {
//            LOG(INFO) << "Open page success.";
//            LOG(INFO) << to_string();
        } else {
//            LOG(ERROR) << "Open page failed. ID: " << ID << " BaseLoc: " << BaseLoc;
        }
    } else {
//        LOG(ERROR) << "OFDPage::Open() ReadZipFileString() failed. " << pageXMLFile;
    }

    return m_opened;
}

void Page::Close()
{
    if (!m_opened) return;
}

LayerPtr Page::AddNewLayer(LayerType layerType)
{
    LayerPtr layer = std::make_shared<Layer>(GetSelf());
    layer->ID = m_layers.size();
    layer->Type = layerType;
    m_layers.push_back(layer);
    return layer;
}

const LayerPtr Page::GetBodyLayer() const
{
    if (m_layers.size() > 0) {
        const LayerPtr bodyLayer = m_layers[0];
        return bodyLayer;
    } else {
        return nullptr;
    }
}

LayerPtr Page::GetBodyLayer()
{
    if (m_layers.size() > 0) {
        const LayerPtr bodyLayer = m_layers[0];
        return bodyLayer;
    } else {
        return nullptr;
    }
}

std::tuple<ST_Box, bool> ReadBoxFromXML(XMLElementPtr boxElement)
{
    ST_Box box;
    bool exist = false;

    std::string boxString;
    std::tie(boxString, exist) = boxElement->GetStringValue();
//    LOG(INFO) << "Box String: " << boxString;
    if (exist) {
        std::vector<std::string> tokens = utils::SplitString(boxString);
        if (tokens.size() >= 4) {
            box.Left = atof(tokens[0].c_str());
            box.Top = atof(tokens[1].c_str());
            box.Width = atof(tokens[2].c_str());
            box.Height = atof(tokens[3].c_str());
        } else {
//            LOG(ERROR) << "Box String tokens size >= 4 failed. boxString:" << boxString;
            exist = false;
        }
    }

    return std::make_tuple(box, exist);
}

// Defined in Document.cc
void writePageAreaXML(XMLWriter &writer, const CT_PageArea &pageArea);

// Generate content in Doc_N/Pages/Page_N/Content.xml
// Called by ofd::Package::Save()
// OFD (section 7.7) P18, Page.xsd。
std::string Page::GeneratePageXML() const
{

    XMLWriter writer(true);

    writer.StartDocument();

    // -------- <Page>
    writer.StartElement("Page");
    {
        OFDXML_HEAD_ATTRIBUTES;

        // TODO
        // -------- <Area>
        // Optional.
        writer.StartElement("Area");
        {
            writePageAreaXML(writer, Area);
        }
        writer.EndElement();

        // TODO
        // -------- <Template>
        // OFD (section 7.7) P19.
        // Optional.

        // TODO
        // -------- <PageRes>
        // Optional.

        // -------- <Content>
        // Optional. Blank page if not exist.
        generateContentXML(writer);

        // TODO
        // -------- <Actions>
        // Optional.
    }
    writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

// -------- Page::generateContentXML() --------
// Called by Page::GeneratePageXML()
void Page::generateContentXML(XMLWriter &writer) const
{
    if (m_layers.size() == 0) return;

    // -------- <Content>
    writer.StartElement("Content");
    {

        for (auto layer : m_layers) {

            // -------- <Layer>
            // OFD (section 7.7) P20.
            // Required.
            writer.StartElement("Layer");
            {

                // -------- <Layer ID="">
                writer.WriteAttribute("ID", layer->ID);

                // -------- CT_Layer --------

                // TODO
                // -------- <Layer Type="">

                // TODO
                // -------- <Layer DrawParam="">

                // -------- CT_PageBlock --------
                //LOG(DEBUG) << "############## There are " << layer->Objects.size() << " objects in layer";

                const ObjectArray &objects = layer->GetObjects();
                for (auto object : objects) {
                    object->GenerateXML(writer);
                }
            }
            writer.EndElement();
        }
    }
    writer.EndElement();
}

// OFD (section 7.5) P11. Definitions.xsd
std::tuple<CT_PageArea, bool> fromPageAreaXML(XMLElementPtr pageAreaElement)
{
    bool ok = true;
    CT_PageArea pageArea;

//    LOG(INFO) << "******** FromPagAreaXML()";

    XMLElementPtr childElement = pageAreaElement->GetFirstChildElement();
    while (childElement != nullptr) {

        std::string childName = childElement->GetName();
//        LOG(INFO) << "PageArea child name: " << childName;
        bool exist = false;

        // -------- <PhysicalBox>
        // Required
        if (childName == "PhysicalBox") {
            std::tie(pageArea.PhysicalBox, exist) = ReadBoxFromXML(childElement);
            if (!exist) {
//                LOG(ERROR) << "Attribute PhysicalBox is requred in PageArea XML";
                break;
            } else {
                ok = true;
            }

            // -------- <ApplicationBox>
        } else if (childName == "ApplicationBox") {
            std::tie(pageArea.ApplicationBox, exist) = ReadBoxFromXML(childElement);
            if (exist) {
                pageArea.EnableApplicationBox(true);
            }

            // -------- <ContentBox>
        } else if (childName == "ContentBox") {
            std::tie(pageArea.ContentBox, exist) = ReadBoxFromXML(childElement);
            if (exist) {
                pageArea.EnableContentBox(true);
            }

            // -------- <BleedBox>
        } else if (childName == "BleedBox") {
            std::tie(pageArea.BleedBox, exist) = ReadBoxFromXML(childElement);
            if (exist) {
                pageArea.EnableBleedBox(true);
            }
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return std::make_tuple(pageArea, ok);
}

// ======== OFDPage::fromPageXML() ========
// OFD (section 7.7) P18. Page.xsd
bool Page::fromPageXML(const std::string &strPageXML)
{
    bool ok = false;

    XMLElementPtr pageElement = XMLElement::ParseRootElement(strPageXML);
    if (pageElement != nullptr) {
        std::string elementName = pageElement->GetName();
        if (elementName == "Page") {

            XMLElementPtr childElement = pageElement->GetFirstChildElement();
            while (childElement != nullptr) {
                std::string childName = childElement->GetName();

                if (childName == "Area") {
                    // -------- <Area>
                    // Optional.
                    std::tie(Area, ok) = fromPageAreaXML(childElement);
                } else if (childName == "Content") {
                    // -------- <Content>
                    // Optional.
                    fromContentXML(childElement);

                    //if ( childName == "Template" ) {
                    //// TODO
                    //// -------- <Template>
                    //// Optional.

                    //} else if ( childName == "PageRes" ) {
                    //// TODO
                    //// -------- <PageRes>
                    //// Optional.

                    //} else if ( childName == "Actions" ) {
                    // TODO
                    // -------- <Actions>
                    // OFD (section 14.1) P73. Document.xsd
                    // Optional.
                    //FromActionsXML(childElement);

                    //}
                }

                childElement = childElement->GetNextSiblingElement();
            }
        }
    } else {
//        LOG(ERROR) << "No root element in Content.xml";
    }

    return ok;
}

// -------- fromLayerXML() --------
// Called by Page::fromContentXML()
LayerPtr Page::fromLayerXML(XMLElementPtr layerElement)
{
    LayerPtr layer = nullptr;

    assert(layerElement != nullptr);

    layer = std::make_shared<Layer>(GetSelf());

    bool exist = false;

    uint64_t layerID = 0;
    uint64_t layerType = 0;
    uint64_t drawParamID = 0;

    std::tie(layerID, exist) = layerElement->GetIntAttribute("ID");
    if (!exist) return nullptr;
    std::tie(layerType, std::ignore) = layerElement->GetIntAttribute("Type");
    std::tie(drawParamID, std::ignore) = layerElement->GetIntAttribute("DrawParam");

    layer->ID = layerID;
    layer->Type = (LayerType)layerType;

    XMLElementPtr childElement = layerElement->GetFirstChildElement();
    while (childElement != nullptr) {

        ObjectPtr object = nullptr;

        std::string childName = childElement->GetName();
        if (childName == "TextObject") {
            TextObject *textObject = new TextObject(layer);
            textObject->FromXML(childElement);
            object = std::shared_ptr<Object>(textObject);
            //LOG(DEBUG) << "Load text object. total: " << layer->Objects.size() << " GetObjectsCount() = " << layer->GetObjectsCount();

        } else if (childName == "PathObject") {
            PathObject *pathObject = new PathObject(layer);
            pathObject->FromXML(childElement);
            object = std::shared_ptr<Object>(pathObject);
        } else if (childName == "ImageObject") {
            ;
        } else if (childName == "VideoObject") {
            VideoObject *videoObject = new VideoObject(layer);
            videoObject->FromXML(childElement);
            object = std::shared_ptr<Object>(videoObject);
        } else if (childName == "CompositeObject") {
            CompositeObject *compositeObject = new CompositeObject(layer);
            compositeObject->FromXML(childElement);
            object = std::shared_ptr<Object>(compositeObject);
        }

        if (object != nullptr) {
            layer->AddObject(object);
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return layer;
}

// ======== Page::fromContentXML() ========
// OFD (section 7.7) P20. Page.xsd
bool Page::fromContentXML(XMLElementPtr contentElement)
{
    bool ok = false;

    assert(contentElement != nullptr);

    XMLElementPtr childElement = contentElement->GetFirstChildElement();
    while (childElement != nullptr) {
        std::string childName = childElement->GetName();

        if (childName == "Layer") {
            LayerPtr layer = fromLayerXML(childElement);
            if (layer != nullptr) {
                m_layers.push_back(layer);
//                LOG(INFO) << "layer added. GetObjectsCount() = " << layer->GetNumObjects();
                ok = true;
            }
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

double Page::GetFitScaling(double screenWidth, double screenHeight, double resolutionX, double resolutionY)
{
    double pageWidth = Area.ApplicationBox.Width * resolutionX / 72.0;
    double pageHeight = Area.ApplicationBox.Height * resolutionY / 72.0;
    double scalingX = screenWidth / pageWidth;
    double scalingY = screenHeight / pageHeight;
    double scaling = scalingX <= scalingY ? scalingX : scalingY;
    //LOG(DEBUG) << "Page::GetFitScaling() page size (" << pageWidth << "," << pageHeight << ") screen size (" << screenWidth << "," << screenHeight << ") scalingX=" << scalingX << " scalingY=" << scalingY << " scaling=" << scaling;
    return scaling;
}
