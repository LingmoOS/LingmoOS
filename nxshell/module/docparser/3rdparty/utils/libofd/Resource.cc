#include <vector>
#include <assert.h>

#include "ofd/Resource.h"
#include "ofd/Document.h"
#include "ofd/Package.h"
#include "ofd/Page.h"
#include "utils/xml.h"


using namespace ofd;

// **************** class Resource::ImplCls ****************

class Resource::ImplCls
{
public:
    ImplCls(Resource *resource, PackagePtr package, const std::string &resDescFile);
    ImplCls(Resource *resource, DocumentPtr document, const std::string &resDescFile);
    ImplCls(Resource *resource, PagePtr page, const std::string &resDescFile);
    ImplCls(Resource *resource, PackagePtr package, DocumentPtr document, PagePtr page, const std::string &resDescFile);

    ~ImplCls();

    void Init_After_Construct();

    const ColorSpaceArray &GetColorSpaces() const { return m_colorSpaces; };
    void AddColorSpace(const ColorSpacePtr &colorSpace) { m_colorSpaces.push_back(colorSpace); };

    std::string GenerateResXML() const;
    bool FromResXML(const std::string &strResXML);

    bool LoadFonts();
    bool LoadImages();

    ResourceLevel GetResourceLevel() const;
    std::string GetEntryRoot() const;

    std::string GenerateResourceFilePath(const std::string resourceFile);

private:
    bool FromColorSpacesXML(utils::XMLElementPtr colorSpacesElement);
    bool FromFontsXML(utils::XMLElementPtr fontsElement);
    bool FromImagesXML(utils::XMLElementPtr fontsElement);

    // -------- Private Attributes --------
public:
    Resource *m_resource;
    std::weak_ptr<Package> m_package;
    std::weak_ptr<Document> m_document;
    std::weak_ptr<Page> m_page;
    std::string m_baseLoc;
    std::string m_resDescFile;

    ColorSpaceArray m_colorSpaces;

};   // class Resource::ImplCls

Resource::ImplCls::ImplCls(Resource *resource, PackagePtr package, const std::string &resDescFile)
    : m_resource(resource), m_package(package), m_baseLoc("Res"), m_resDescFile(resDescFile)
{
}

Resource::ImplCls::ImplCls(Resource *resource, DocumentPtr document, const std::string &resDescFile)
    : m_resource(resource),
      m_document(document),
      m_baseLoc("Res"),
      m_resDescFile(resDescFile)
{
}

Resource::ImplCls::ImplCls(Resource *resource, PagePtr page, const std::string &resDescFile)
    : m_resource(resource),
      m_page(page),
      m_baseLoc("Res"),
      m_resDescFile(resDescFile)
{
}

void Resource::ImplCls::Init_After_Construct()
{
    if (!m_page.expired()) {
        DocumentPtr document = m_page.lock()->GetDocument();
        assert(document != nullptr);
        m_document = document;

        PackagePtr package = document->GetPackage();
        assert(package != nullptr);
        m_package = package;

        if (m_resDescFile.empty()) {
            m_resDescFile = "PageRes.xml";
        }
    } else {
        if (!m_document.expired()) {
            PackagePtr package = m_document.lock()->GetPackage();
            assert(package != nullptr);
            m_package = package;

            if (m_resDescFile.empty()) {
                m_resDescFile = "DocumentRes.xml";
            }
        } else {
            if (m_resDescFile.empty()) {
                m_resDescFile = "PublicRes.xml";
            }
        }
    }
}

Resource::ImplCls::ImplCls(Resource *resource, PackagePtr package, DocumentPtr document, PagePtr page, const std::string &resDescFile)
    : m_resource(resource),
      //m_package(package), m_document(document), m_page(page),
      m_baseLoc("Res"),
      m_resDescFile(resDescFile)
{

    m_package.reset();
    m_document.reset();
    m_page.reset();

    if (package != nullptr) {
        m_package = package;
    }

    if (document != nullptr) {
        m_document = document;
    }

    if (page != nullptr) {
        m_page = page;
    }

    if (m_resDescFile.empty()) {
        if (m_package.lock() != nullptr) {
            if (m_document.lock() != nullptr) {
                if (m_page.lock() != nullptr) {
                    m_resDescFile = "PageRes.xml";
                } else {
                    m_resDescFile = "DocumentRes.xml";
                }
            } else {
                m_resDescFile = "PublicRes.xml";
            }
        } else {
//            LOG(ERROR) << "m_package == nullptr in Resource.";
        }
    }
}

Resource::ImplCls::~ImplCls()
{
}

void generateColorSpacesXML(utils::XMLWriter &writer)
{
}

void generateDrawParamsXML(utils::XMLWriter &writer)
{
}

void generateMultiMediasXML(utils::XMLWriter &writer)
{
}

void generateCompositeGraphicUnitsXML(utils::XMLWriter &writer)
{
}

// ======== Resource::ImplCls::GenerateResXML() ========
// OFD (section 7.9) P23. Res.xml.
std::string Resource::ImplCls::GenerateResXML() const
{
    utils::XMLWriter writer(true);

    writer.StartDocument();

    // -------- <Page>
    writer.StartElement("Res");
    {
        OFDXML_HEAD_ATTRIBUTES;

        writer.WriteAttribute("BaseLoc", m_baseLoc);

        // TODO
        // -------- <ColorSpaces>
        // Optional.
        if (m_colorSpaces.size() > 0) {
            writer.StartElement("ColorSpaces");
            {
                generateColorSpacesXML(writer);
            }
            writer.EndElement();
        }

        // TODO
        // -------- <DrawParams>
        // Optional.
        //if ( m_drawParams.size() > 0 ){
        //writer.StartElement("DrawParams");{
        //generateDrawParamsXML(writer);
        //} writer.EndElement();
        //}

        // -------- <Fonts>
        // Optional.

        // TODO
        // -------- <MultiMedias>
        // Optional.
        //if ( m_multiMedias.size() > 0 ){
        //writer.StartElement("MultiMedias");{
        //generateMultiMediasXML(writer);
        //} writer.EndElement();
        //}

        // TODO
        // -------- <CompositeGraphicUnits>
        // Optional.
        //if ( m_compositeGraphicUnits.size() > 0 ){
        //writer.StartElement("CompositeGraphicUnits");{
        //generateCompositeGraphicUnitsXML(writer);
        //} writer.EndElement();
        //}
    }
    writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

bool Resource::ImplCls::FromColorSpacesXML(utils::XMLElementPtr colorSpacesElement)
{
    bool ok = true;

    return ok;
}

bool Resource::ImplCls::LoadFonts()
{
    bool ok = true;

    if (m_package.expired()) {
        return false;
    }

    return ok;
}

bool Resource::ImplCls::LoadImages()
{
    bool ok = true;

    if (m_package.expired()) {
        return false;
    }

    return ok;
}

std::string Resource::ImplCls::GenerateResourceFilePath(const std::string resourceFile)
{
    std::string resourceFilePath = GetEntryRoot() + "/" + m_baseLoc + "/" + resourceFile;
    return resourceFilePath;
}

bool Resource::ImplCls::FromFontsXML(utils::XMLElementPtr fontsElement)
{
    bool ok = false;

    utils::XMLElementPtr childElement = fontsElement->GetFirstChildElement();
    while (childElement != nullptr) {

        ok = true;

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

bool Resource::ImplCls::FromImagesXML(utils::XMLElementPtr imagesElement)
{
    bool ok = false;

    utils::XMLElementPtr childElement = imagesElement->GetFirstChildElement();
    while (childElement != nullptr) {

        ok = true;

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// ======== Resource::ImplCls::FromResXML() ========
// OFD (section 7.9) P23. Res.xml.
bool Resource::ImplCls::FromResXML(const std::string &strResXML)
{
    bool ok = true;

    utils::XMLElementPtr rootElement = utils::XMLElement::ParseRootElement(strResXML);
    if (rootElement != nullptr) {
        if (rootElement->GetName() == "Res") {

            // -------- <Res BaseLoc="">
            // Required.
            bool exist = false;
            std::tie(m_baseLoc, exist) = rootElement->GetStringAttribute("BaseLoc");
            if (!exist) {
//                LOG(ERROR) << "BaseLoc attribute is required in Res.xsd";
                return false;
            }

            utils::XMLElementPtr childElement = rootElement->GetFirstChildElement();
            while (childElement != nullptr) {
                std::string childName = childElement->GetName();

                if (childName == "ColorSpaces") {
                    // -------- <ColorSpaces>
                    // Optional
                    FromColorSpacesXML(childElement);

                    //} else if ( childName == "DrawParams" ){
                    // TODO
                    // -------- <DrawParams>
                    // Optional

                } else if (childName == "Fonts") {
                    // -------- <Fonts>
                    // Optional
                    FromFontsXML(childElement);

                } else if (childName == "Images") {
                    // -------- <Images>
                    // Optional
                    FromImagesXML(childElement);

                    //} else if ( childName == "MultiMedias" ){
                    // TODO
                    // -------- <MultiMedias>
                    // Optional

                    //} else if ( childName == "CompositeGraphicUnits" ){
                    // TODO
                    // -------- <CompositeGraphicUnits>
                    // Optional
                }

                childElement = childElement->GetNextSiblingElement();
            }
        }
    }

    return ok;
}

// **************** class Resource ****************

Resource::Resource(PackagePtr package, const std::string &resDescFile)
    : m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, package, resDescFile)))
{
}

Resource::Resource(DocumentPtr document, const std::string &resDescFile)
    : m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, document, resDescFile)))
{
}

Resource::Resource(PagePtr page, const std::string &resDescFile)
    : m_impl(std::unique_ptr<ImplCls>(new ImplCls(this, page, resDescFile)))
{
}

Resource::~Resource()
{
}

ResourcePtr Resource::GetSelf()
{
    return shared_from_this();
}

std::string Resource::GetBaseLoc() const
{
    return m_impl->m_baseLoc;
}

ResourceLevel Resource::ImplCls::GetResourceLevel() const
{
    assert(m_package.lock() != nullptr);

    if (m_page.lock() != nullptr) {
        assert(m_document.lock() != nullptr);
        return ResourceLevel::PAGE;
    } else if (m_document.lock() != nullptr) {
        return ResourceLevel::DOCUMENT;
    } else {
        return ResourceLevel::PACKAGE;
    }
}

ResourceLevel Resource::GetResourceLevel() const
{
    return m_impl->GetResourceLevel();
}

// FIXME
std::string Resource::ImplCls::GetEntryRoot() const
{
    ResourceLevel level = GetResourceLevel();
    if (level == ResourceLevel::PAGE) {
        // FIXME
        return "";
    } else if (level == ResourceLevel::DOCUMENT) {
        return "Doc_0";
    } else {
        return "";
    }
}

std::string Resource::GetEntryRoot() const
{
    return m_impl->GetEntryRoot();
}

const PackagePtr Resource::GetPackage() const
{
    return m_impl->m_package.lock();
}

const DocumentPtr Resource::GetDocument() const
{
    return m_impl->m_document.lock();
}

const PagePtr Resource::GetPage() const
{
    return m_impl->m_page.lock();
}

void Resource::SetBaseLoc(const std::string &baseLoc)
{
    m_impl->m_baseLoc = baseLoc;
}

const ColorSpaceArray &Resource::GetColorSpaces() const
{
    return m_impl->GetColorSpaces();
}

void Resource::AddColorSpace(const ColorSpacePtr &colorSpace)
{
    m_impl->AddColorSpace(colorSpace);
}

std::string Resource::GenerateResXML() const
{
    return m_impl->GenerateResXML();
}

bool Resource::FromResXML(const std::string &strResXML)
{
    return m_impl->FromResXML(strResXML);
}

bool Resource::LoadFonts()
{
    return m_impl->LoadFonts();
}

bool Resource::LoadImages()
{
    return m_impl->LoadImages();
}

std::string Resource::GetResDescFile() const
{
    return m_impl->m_resDescFile;
}

ResourcePtr Resource::CreateNewResource(PackagePtr package, const std::string &resDescFile)
{

    ResourcePtr resource = std::shared_ptr<Resource>(new Resource(package, resDescFile));
    resource->m_impl->Init_After_Construct();
    return resource;
}

ResourcePtr Resource::CreateNewResource(DocumentPtr document, const std::string &resDescFile)
{
    ResourcePtr resource = std::shared_ptr<Resource>(new Resource(document, resDescFile));
    resource->m_impl->Init_After_Construct();
    return resource;
}

ResourcePtr Resource::CreateNewResource(PagePtr page, const std::string &resDescFile)
{
    ResourcePtr resource = std::shared_ptr<Resource>(new Resource(page, resDescFile));
    resource->m_impl->Init_After_Construct();
    return resource;
}

std::string Resource::GenerateResourceFilePath(const std::string resourceFile)
{
    return m_impl->GenerateResourceFilePath(resourceFile);
}
