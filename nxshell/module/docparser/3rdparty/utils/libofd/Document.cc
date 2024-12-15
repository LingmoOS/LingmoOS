#include <sstream>
#include "ofd/Package.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "ofd/Resource.h"
#include "utils/xml.h"
#include "utils/uuid.h"

using namespace ofd;
using namespace utils;

namespace ofd{
    Document::CT_DocInfo::CT_DocInfo() : 
        DocUsage("Normal"){
            DocID = generate_uuid32();
        }
}

Document::Document(PackagePtr package, const std::string &docRoot){ 
    m_package = package;
    m_opened = false;
    m_docBody.DocRoot = docRoot;
}

Document::~Document(){
}

DocumentPtr Document::GetSelf(){
    return shared_from_this();
}

std::string Document::to_string() const{
    std::ostringstream ss;
    ss << "\n======== ofd::OFDDocument ========\n";
    ss << "Pages: " << m_pages.size() << "\n";
    ss << std::endl;
    return ss.str();
}

// ======== Document::CreateNewDocument() ========
DocumentPtr Document::CreateNewDocument(PackagePtr package, const std::string &docRoot){
    DocumentPtr document = std::shared_ptr<Document>(new Document(package, docRoot));
    document->m_commonData.PublicRes = Resource::CreateNewResource(package);
    document->m_commonData.DocumentRes = Resource::CreateNewResource(document->GetSelf()); 
    return document;
}

bool Document::Open(){
    if ( m_opened ) return true;

    if ( m_commonData.PublicRes != nullptr ){
        std::string strResXML;
        std::tie(strResXML, std::ignore) = m_package.lock()->ReadZipFileString(m_docBody.DocRoot + "/" + m_commonData.PublicRes->GetResDescFile());
        if ( !m_commonData.PublicRes->FromResXML(strResXML) ){
//            LOG(ERROR) << "m_commonData.PublicRes.FromResXML() failed.";
            return false;
        }
    }

    if ( m_commonData.DocumentRes != nullptr ){
        std::string strResXML;
        std::tie(strResXML, std::ignore) = m_package.lock()->ReadZipFileString(m_docBody.DocRoot + "/" + m_commonData.DocumentRes->GetResDescFile());
        if ( !m_commonData.DocumentRes->FromResXML(strResXML) ){
//            LOG(ERROR) << "m_commonData.DocumentRes.FromResXML() failed.";
            return false;
        } else {
            m_commonData.DocumentRes->LoadFonts();
            m_commonData.DocumentRes->LoadImages();
        }
    }
    m_opened = true;

    return m_opened;
}

void Document::Close(){
    if ( !m_opened ) return;
}

size_t Document::GetNumPages() const{
    return m_pages.size();
}

const PagePtr Document::GetPage(size_t idx) const{
    return m_pages[idx];
}

PagePtr Document::GetPage(size_t idx){
    return m_pages[idx];
}

PagePtr Document::AddNewPage(){
    PagePtr page = Page::CreateNewPage(GetSelf());
    page->ID = m_pages.size();
    m_pages.push_back(page);
    return page;
}

// ======== Document::GenerateDocumentXML() ========
// Called by ofd::Package::Save().
std::string Document::GenerateDocumentXML() const{
    XMLWriter writer(true);

    writer.StartDocument();

    writer.StartElement("Document");{
        OFDXML_HEAD_ATTRIBUTES;

        // -------- <CommonData> 
        // Required.
        generateCommonDataXML(writer);    

        // -------- <Pages> 
        // Required.
        generatePagesXML(writer);

    } writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

// OFD (section 7.5) P11. Definitions.xsd
void writePageAreaXML(XMLWriter &writer, const CT_PageArea &pageArea){
    // -------- <PhysicalBox> 
    // Required.
    writer.WriteElement("PhysicalBox", pageArea.PhysicalBox.to_xmlstring());

   
    // -------- <ApplicationBox>
    // Optional.
    if ( pageArea.HasApplicationBox() ){
        writer.WriteElement("ApplicationBox", pageArea.ApplicationBox.to_xmlstring());
    }

    // -------- <ContentBox>
    // Optional.
    if ( pageArea.HasContentBox() ){
        writer.WriteElement("ContentBox", pageArea.ContentBox.to_xmlstring());
    }
    
    // -------- <BleedBox>
    // Optional.
    if ( pageArea.HasBleedBox() ){
        writer.WriteElement("BleedBox", pageArea.BleedBox.to_xmlstring());
    }
}

// -------- Document::generateCommonDataXML() --------
// Called by Document::GenerateDocumentXML().
void Document::generateCommonDataXML(XMLWriter &writer) const{

    // -------- <CommonData> 
    // OFD (section 7.5) P10. OFD.xsd
    // Required.
    writer.StartElement("CommonData");{

        // -------- <MaxUnitID> 
        // Required.
        writer.WriteElement("MaxUnitID", m_commonData.MaxUnitID);

        // -------- <PageArea> 
        // Required.
        const CT_PageArea &pageArea = m_commonData.PageArea;
        writer.StartElement("PageArea");{

            writePageAreaXML(writer, pageArea);

        } writer.EndElement();

        // -------- <PublicRes>
        // Optional.
        if ( m_commonData.PublicRes != nullptr ){
            writer.WriteElement("PublicRes", m_commonData.PublicRes->GetResDescFile());
        }

        // -------- <DocumentRes>
        // Optional.
        if ( m_commonData.DocumentRes != nullptr ){
            writer.WriteElement("DocumentRes", m_commonData.DocumentRes->GetResDescFile());
        }

        // TODO
        // -------- <TemplatePage>
        // Optional

        // TODO
        // -------- <DefaultCS>
        // Optional


    } writer.EndElement();
}

// Called by OFDDocument::GenerateDocumentXML().
void Document::generatePagesXML(XMLWriter &writer) const{

    // -------- <Pages> 
    // OFD (section 7.6) P17. Page.xsd.
    // Required.
    writer.StartElement("Pages");{
        size_t idx = 0;
        for ( auto page : m_pages ){
            // FIXME
            //uint64_t pageID = page->GetID();
            uint64_t pageID = 0;

            // -------- <Page>
            // Required.
            writer.StartElement("Page");{

                // -------- <Page ID="">
                // Required
                writer.WriteAttribute("ID", pageID);

                // -------- <Page BaseLoc="">
                // Required.
                writer.WriteAttribute("BaseLoc", std::string("Pages/Page_") + std::to_string(idx));

                idx++;
            } writer.EndElement();
        }
    } writer.EndElement();
}

// ======== Document::GenerateDocBodyXML() ========
// Called by Package::generateOFDXML()
std::string Document::GenerateDocBodyXML() const{

    XMLWriter writer;

    writer.StartDocument();

    // -------- <DocInfo> 
    // OFD (section 7.4) P7. OFD.xsd.
    // Required.
    writer.StartElement("DocInfo");{

        const CT_DocInfo &docInfo = m_docBody.DocInfo;

        // -------- <DocID>
        // Required.
        writer.WriteElement("DocID", docInfo.DocID);

        // -------- <Title>
        if ( !docInfo.Title.empty() ){
            writer.WriteElement("Title", docInfo.Title);
        }
        // -------- <Author>
        if ( !docInfo.Author.empty() ){
            writer.WriteElement("Author", docInfo.Author);
        }
        // -------- <Subject>
        if ( !docInfo.Subject.empty() ){
            writer.WriteElement("Subject", docInfo.Subject);
        }
        // -------- <Abstract>
        if ( !docInfo.Abstract.empty() ){
            writer.WriteElement("Abstract", docInfo.Abstract);
        }
        // -------- <CreationDate>
        if ( !docInfo.CreationDate.empty() ){
            writer.WriteElement("CreationDate", docInfo.CreationDate);
        }
        // -------- <ModDate>
        if ( !docInfo.ModDate.empty() ){
            writer.WriteElement("ModDate", docInfo.ModDate);
        }
        // FIXME
        // -------- <DocUsage>
        //writer.WriteElement("DocUsage", docInfo.DocUsage);
        // -------- <Cover>
        if ( !docInfo.Cover.empty() ){
            writer.WriteElement("Cover", docInfo.Cover);
        }
        // -------- <Keywords>
        if ( docInfo.Keywords.size() > 0 ){
            writer.StartElement("Keywords");{
                for ( auto keyword : docInfo.Keywords ){
                    writer.WriteElement("Keyword", keyword);
                }
            } writer.EndElement();
        }
        // -------- <Creator>
        if ( !docInfo.Creator.empty() ){
            writer.WriteElement("Creator", docInfo.Creator);
        }
        // -------- <CreatorVersion>
        if ( !docInfo.CreatorVersion.empty() ){
            writer.WriteElement("CreatorVersion", docInfo.CreatorVersion);
        }
        // -------- <CustomDatas>
        if ( docInfo.CustomDatas.size() > 0 ){
            writer.StartElement("CustomDatas");{

                for ( auto customData : docInfo.CustomDatas ){
                    const std::string &name = customData.first;
                    const std::string &value = customData.second;
                    writer.StartElement("CustomData");{

                        writer.WriteAttribute("Name", name);
                        writer.WriteString(value);

                    } writer.EndElement();
                }

            } writer.EndElement();
        }

    } writer.EndElement();

    // -------- <DocRoot> 
    const ST_Loc &docRoot = m_docBody.DocRoot;
    if ( !docRoot.empty() ){
        writer.WriteElement("DocRoot", docRoot);
    }

    // TODO
    // -------- <Versions> 
    const DocBody::VersionArray &versions = m_docBody.Versions;
    if ( versions.size() > 0 ){
        writer.StartElement("Versions");

        writer.EndElement();
    }

    // TODO
    // -------- <Signatures> 
    if ( !m_docBody.Signatures.empty() ){
        writer.WriteElement("Signatures", m_docBody.Signatures);
    }

    writer.EndDocument();

    return writer.GetString();
}

// ======== Document::FromDocBodyXML() ========
// Called by Package::fromOFDXML()
// OFD (section 7.4) P7. OFD.xsd.
bool Document::FromDocBodyXML(XMLElementPtr docBodyElement){
    bool ok = true;

    XMLElementPtr childElement = docBodyElement->GetFirstChildElement();
    bool hasDocInfo = false;
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        // -------- <DocInfo>
        // Required.
        if ( childName == "DocInfo"){
            hasDocInfo = true;
            fromDocInfoXML(childElement);

        // -------- <DocRoot>
        // Optional.
        } else if ( childName == "DocRoot" ){
            std::string docRoot;
            std::tie(docRoot, std::ignore) = childElement->GetStringValue();
//            LOG(DEBUG) << "DocRoot: " << docRoot;

        // TODO
        // -------- <Versions>
        // Optional.
        //} else if ( childName == "Versions" ){

        // TODO
        // -------- <Signatures>
        // Optional.
        //} else if ( childName == "Signatures" ){

        }

        childElement = childElement->GetNextSiblingElement();
    }

    if ( !hasDocInfo ){
//        LOG(ERROR) << "No DocInfo element in DocBody.";
    }

    //if ( reader.EnterChildElement("DocBody") ){
        //while ( reader.HasElement() ){
            //// -------- <DocInfo>
            //if ( reader.CheckElement("DocInfo") ){
                //FromDocInfoXML(reader);

            //// -------- <DocRoot>
            //} else if ( reader.CheckElement("DocRoot") ){
                //std::string content;
                //reader.ReadElement(content);
//                LOG(DEBUG) << "DocRoot: " << content;

            //// -------- <Versions>
            //} else if ( reader.CheckElement("Versions") ){

            //// -------- <Signatures>
            //} else if ( reader.CheckElement("Signatures") ){
            //}

            //reader.NextElement();
        //};
        //reader.BackParentElement();
    //} 

    return ok;
}
// -------- Document::fromDocInfoXML() --------
// Called by Document::FromDocBodyXML()
// OFD (section 7.4) P7. OFD.xsd
bool Document::fromDocInfoXML(XMLElementPtr docInfoElement){
    bool ok = true;
    
    CT_DocInfo &docInfo = m_docBody.DocInfo;

    XMLElementPtr childElement = docInfoElement->GetFirstChildElement();
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        // -------- <DocID>
        // Optional.
        if ( childName == "DocID" ){
            std::tie(docInfo.DocID, std::ignore) = childElement->GetStringValue();
//            LOG(DEBUG) << "DocID: " << docInfo.DocID;

        // -------- <Title>
        // Optional.
        } else if ( childName == "Title" ){
            std::tie(docInfo.Title, std::ignore) = childElement->GetStringValue();
//            LOG(DEBUG) << "Title: " << docInfo.Title;

        // -------- <Author>
        // Optional.
        } else if ( childName == "Author" ){
            std::tie(docInfo.Author, std::ignore) = childElement->GetStringValue();

        // -------- <Subject>
        // Optional.
        } else if ( childName == "Subject" ){
            std::tie(docInfo.Subject, std::ignore) = childElement->GetStringValue();

        // -------- <Abstract>
        // Optional.
        } else if ( childName == "Abstract" ){
            std::tie(docInfo.Abstract, std::ignore) = childElement->GetStringValue();

        // -------- <CreationDate>
        // Optional.
        } else if ( childName == "CreationDate" ){
            std::tie(docInfo.CreationDate, std::ignore) = childElement->GetStringValue();

        // -------- <ModDate>
        // Optional.
        } else if ( childName == "ModDate" ){
            std::tie(docInfo.ModDate, std::ignore) = childElement->GetStringValue();

        // FIXME
        // -------- <DocUsage>
        // Optional
        //} else if ( childName == "DocUsage" ){
        //

        // -------- <Cover>
        // Optional
        } else if ( childName == "Cover" ){
            std::tie(docInfo.Cover, std::ignore) = childElement->GetStringValue();

        // -------- <Keywords>
        // Optional.
        } else if ( childName == "Keywords" ){ 
            docInfo.Keywords.clear();
            XMLElementPtr keyWordElement = childElement->GetFirstChildElement();
            while ( keyWordElement != nullptr ){
                std::string keyword;
                std::tie(keyword, std::ignore) = keyWordElement->GetStringValue();
                keyWordElement = keyWordElement->GetNextSiblingElement();
                docInfo.Keywords.push_back(keyword);
            }

        // -------- <Creator>
        // Optional.
        } else if ( childName == "Creator" ){ 
            std::tie(docInfo.Creator, std::ignore) = childElement->GetStringValue();

        // -------- <CreatorVersion>
        // Optional
        } else if ( childName == "CreatorVersion" ){ 
            std::tie(docInfo.CreatorVersion, std::ignore) = childElement->GetStringValue();

        // -------- <CustomDatas>
        // Optional
        } else if ( childName == "CustomDatas" ){ 
            docInfo.CustomDatas.clear();
            XMLElementPtr customDataElement = childElement->GetFirstChildElement();
            while ( customDataElement != nullptr ){
                bool exist = false;
                std::string name;
                std::tie(name, exist) = customDataElement->GetStringAttribute("Name");
                if ( exist ){
                    std::string value;
                    std::tie(value, std::ignore) = customDataElement->GetStringValue();
                    docInfo.CustomDatas[name] = value;
                }

                customDataElement = customDataElement->GetNextSiblingElement();
            }

        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;


    //if ( reader.EnterChildElement("DocInfo") ){
        //while ( reader.HasElement() ){

            //// -------- <DocID>
            //if ( reader.CheckElement("DocID") ){
                //std::string docID;
                //reader.ReadElement(docID);
                //LOG(DEBUG) << "DocID: " << docID;

            //// -------- <DocRoot>
            //} else if ( reader.CheckElement("Title") ){
                //std::string title;
                //reader.ReadElement(title);
                //LOG(DEBUG) << "Title: " << title;
            //}

            //reader.NextElement();
        //};
        //reader.BackParentElement();
    //} 

    return ok;
}
// ======== Document::FromDocumentXML() ========
// Called by Package::fromOFDXML()
// OFD (section 7.5) P9. Document.xsd
bool Document::FromDocumentXML(const std::string &strDocumentXML){
    bool ok = true;

    XMLElementPtr rootElement = XMLElement::ParseRootElement(strDocumentXML);
    if ( rootElement != nullptr ){
        std::string rootName = rootElement->GetName();
        if ( rootName == "Document" ){
            XMLElementPtr childElement = rootElement->GetFirstChildElement();
            while ( childElement != nullptr ){
                std::string childName = childElement->GetName();

                // -------- <CommonData>
                // OFD (section 7.5) P10. Document.xsd
                // Required.
                if ( childName == "CommonData" ) {
                    fromCommonDataXML(childElement);

                // -------- <Pages>
                // OFD (section 7.6) P17. Document.xsd
                // Required.
                } else if ( childName == "Pages" ) {
                    fromPagesXML(childElement);

                //// TODO
                //// -------- <Outlines>
                //// OFD (section 7.8) P22. Document.xsd
                //// Optional.
                //} else if ( childName == "Outlines" ) {
                    //FromOutlinesXML(childElement);

                // TODO
                // -------- <Permissions>
                // OFD (section 7.5) P13. Document.xsd
                // Optional.
                /*} else if ( childName == "Permissions" ) {*/
                /*FromPermissionsXML(childElement);*/

                // TODO
                // -------- <Actions>
                // OFD (section 14.1) P73. Document.xsd
                // Optional.
                /*} else if ( childName == "Actions" ) {*/
                /*FromActionsXML(childElement);*/

                // TODO
                // -------- <VPreferences>
                // OFD (section 7.5) P15. Document.xsd
                // Optional.
                /*} else if ( childName == "VPreferences" ) {*/
                /*FromVPreferencesXML(childElement);*/

                // TODO
                // -------- <Bookmarks>
                // OFD (section 7.5) P17. Document.xsd
                // Optional.
                /*} else if ( childName == "Bookmarks" ) {*/
                /*FromBookmarksXML(childElement);*/

                // TODO
                // -------- <Attachments>
                // OFD (section 20) P88. Document.xsd
                // Optional.
                /*} else if ( childName == "Attachments" ) {*/
                /*FromAttachmentsXML(childElement);*/

                // TODO
                // -------- <CustomTags>
                // OFD (section 16) P80. Document.xsd
                // Optional.
                /*} else if ( childName == "CustomTags" ) {*/
                /*FromCustomTagsXML(childElement);*/

                // TODO
                // -------- <Extensions>
                // OFD (section 17) P81. Document.xsd
                // Optional.
                /*} else if ( childName == "Extensions" ) {*/
                /*FromExtensionsXML(childElement);*/

                }

                childElement = childElement->GetNextSiblingElement();
            }
        } else {
//            LOG(ERROR) << "Root element in Document Content.xml is not named 'Document'";
        }
    } else {
//        LOG(ERROR) << "No root element in Document Content.xml";
    }

    return ok;
}

// Defined in Page.cc
std::tuple<CT_PageArea,bool> fromPageAreaXML(XMLElementPtr pageAreaElement);

// -------- Document::fromCommonDataXML() --------
// -------- <CommonData>
// Called by Document::FromDocumentXML()
// OFD (section 7.5) P10. Document.xsd
// Required.
bool Document::fromCommonDataXML(XMLElementPtr commonDataElement){
    bool ok = true;

    XMLElementPtr childElement = commonDataElement->GetFirstChildElement();
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        if ( childName == "MaxUnitID" ){
            // -------- <MaxUnitID>
            // Required.

        } else if ( childName == "PageArea" ){
            // -------- <PageArea>
            // OFD (section 7.5) P11. Definitions.xsd
            // Required.
            std::tie(m_commonData.PageArea, ok) = fromPageAreaXML(childElement);
//            LOG(DEBUG) << "CommonData.PageArea = " << m_commonData.PageArea.to_string();

        } else if ( childName == "PublicRes" ){
            // -------- <PublicRes>
            // Optional.
            std::string resDescFile;
            std::tie(resDescFile, std::ignore) = childElement->GetStringValue();
            if ( !resDescFile.empty() ){
                //m_commonData.PublicRes = std::make_shared<OFDRes>(m_package.lock(), nullptr, nullptr, resDescFile);
                m_commonData.PublicRes = Resource::CreateNewResource(m_package.lock());
            }

        } else if ( childName == "DocumentRes" ){
            // -------- <DocumentRes>
            // Optional.
            std::string resDescFile;
            std::tie(resDescFile, std::ignore) = childElement->GetStringValue();
            if ( !resDescFile.empty() ){
                //m_commonData.DocumentRes = std::make_shared<OFDRes>(m_ofdDocument->GetPackage(), m_ofdDocument->GetSelf(), nullptr, resDescFile);

                // FIXME
                //m_commonData.DocumentRes = Resource::CreateNewResource(m_ofdDocument->GetSelf());
            }

        //} else if ( childName == "TemplatePage" ){
            //// TODO
            //// -------- <TemplatePage>
            //// Optional.

        //} else if ( childName == "DefaultCS" ){
            //// TODO
            //// -------- <DefaultCS>
            //// Optional.

        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}

// -------- Document::fromPagesXML() --------
// -------- <Pages>
// Called by Document::FromDocumentXML()
// OFD (section 7.6) P17. Document.xsd
// Required.
bool Document::fromPagesXML(XMLElementPtr pagesElement){
    bool ok = true;

    XMLElementPtr childElement = pagesElement->GetFirstChildElement();
    while ( childElement != nullptr ){
        std::string childName = childElement->GetName();

        if ( childName == "Page" ){
            // -------- <Page>
            // OFD (section 7.7) P18. Page.xsd
            // Required.
            uint64_t pageID = 0;
            bool exist = false;
            std::tie(pageID, exist) = childElement->GetIntAttribute("ID");
            if ( !exist ){
//                LOG(ERROR) << "Attribute ID is required in Document.xsd";
                return false;
            }

            std::string baseLoc;
            std::tie(baseLoc, exist) = childElement->GetStringAttribute("BaseLoc");
            if ( !exist ){
//                LOG(ERROR) << "Attribute BaseLoc is required in Document.xsd";
                return false;
            }

//            LOG(DEBUG) << "PageID: " << pageID << " BaseLoc: " << baseLoc;

            PagePtr page = AddNewPage();
            page->ID = pageID;
            page->BaseLoc = baseLoc;
        }

        childElement = childElement->GetNextSiblingElement();
    }

    return ok;
}
