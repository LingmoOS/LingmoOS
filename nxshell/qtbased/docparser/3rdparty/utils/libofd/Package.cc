#include <assert.h>
#include <fstream>
#include "ofd/Package.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "ofd/Resource.h"
#include "utils/xml.h"
#include "utils/zip.h"


using namespace ofd;
using namespace utils;

Package::Package()
    : Version("1.0"), DocType("OFD"), m_opened(false), m_zip(nullptr)
{
}

Package::Package(const std::string &filename)
    : Version("1.0"), DocType("OFD"), m_filename(filename), m_opened(false), m_zip(nullptr)
{
}

Package::~Package()
{
}

PackagePtr Package::GetSelf()
{
    return shared_from_this();
}

// ======== Package::Open() ========
bool Package::Open(const std::string &filename)
{
    if (m_opened) return true;

    if (!filename.empty()) m_filename = filename;
    if (m_filename.empty()) return false;

    m_zip = std::make_shared<utils::Zip>();
    if (!m_zip->Open(m_filename, false)) {
//        LOG(ERROR) << "Error: Open " << m_filename << " failed.";
        return false;
    }

    bool ok = false;
    std::string strOFDXML;
    std::tie(strOFDXML, ok) = ReadZipFileString("OFD.xml");

    if (ok) {
        m_opened = fromOFDXML(strOFDXML);
    }

    return m_opened;
}

// ======== Package::Close() ========
void Package::Close()
{
    if (m_opened) {
        if (m_zip != nullptr) {
            m_zip->Close();
            m_zip = nullptr;
        }
    }
}

const DocumentPtr Package::GetDefaultDocument() const
{
    DocumentPtr defaultDocument = nullptr;
    if (m_documents.size() > 0) {
        defaultDocument = m_documents[0];
    }
    return defaultDocument;
}

DocumentPtr Package::GetDefaultDocument()
{
    DocumentPtr defaultDocument = nullptr;
    if (m_documents.size() > 0) {
        defaultDocument = m_documents[0];
    }
    return defaultDocument;
}

void test_libsodium();

// ======== Package::Save() ========
bool Package::Save(const std::string &filename)
{
    return false;
    //if ( !m_opened ) return false;
}

// -------- Package::generateOFDXML() --------
std::string Package::generateOFDXML() const
{
    XMLWriter writer(true);

    writer.StartDocument();

    // -------- <OFD>
    writer.StartElement("OFD");
    {
        OFDXML_HEAD_ATTRIBUTES;

        // -------- <OFD Version="">
        writer.WriteAttribute("Version", Version);
        // -------- <OFD DocType="">
        writer.WriteAttribute("DocType", DocType);

        for (auto document : m_documents) {

            // -------- <DocBody>
            writer.StartElement("DocBody");
            {

                std::string strDocBody = document->GenerateDocBodyXML();
                writer.WriteRaw(strDocBody);
            }
            writer.EndElement();
        }
    }
    writer.EndElement();

    writer.EndDocument();

    return writer.GetString();
}

// ======== Package::AddNewDocument()() ========
DocumentPtr Package::AddNewDocument()
{
    size_t idx = m_documents.size();
    std::string docRoot = std::string("Doc_") + std::to_string(idx);
//    LOG(DEBUG) << "Calling OFDPackage::AddNewDocument(). docRoot: " << docRoot;

    DocumentPtr document = Document::CreateNewDocument(GetSelf(), docRoot);

//    LOG(DEBUG) << "After create document.";
    m_documents.push_back(document);

    return document;
}

// ======== Package::GetDocumentsCount() ========
size_t Package::GetDocumentsCount() const
{
    return m_documents.size();
}

// ======== Package::GetDocument() ========
const DocumentPtr Package::GetDocument(size_t idx) const
{
    return m_documents[idx];
}

// ======== Package::GetDocument() ========
DocumentPtr Package::GetDocument(size_t idx)
{
    return m_documents[idx];
}

// ======== Package::ReadZipFileString() ========
std::tuple<std::string, bool> Package::ReadZipFileString(const std::string &fileinzip) const
{
    std::string content;
    bool ok = false;

    if (m_zip != nullptr) {
        std::tie(content, ok) = m_zip->ReadFileString(fileinzip);
    }

    return std::make_tuple(content, ok);
}

// ======== Package::ReadZipFileRaw() ========
std::tuple<char *, size_t, bool> Package::ReadZipFileRaw(const std::string &fileinzip) const
{
    char *buf = nullptr;
    size_t buflen = 0;
    bool ok = false;

    if (m_zip != nullptr) {
        std::tie(buf, buflen, ok) = m_zip->ReadFileRaw(fileinzip);
    }

    return std::make_tuple(buf, buflen, ok);
}

// -------- Package::fromOFDXML() --------
// OFD (section 7.4) P6. OFD.xsd
bool Package::fromOFDXML(const std::string &strOFDXML)
{
    bool ok = true;

    XMLElementPtr rootElement = XMLElement::ParseRootElement(strOFDXML);
    if (rootElement != nullptr) {
        std::string rootName = rootElement->GetName();
        if (rootName == "OFD") {

            std::string Version, DocType;
            bool exist = false;

            // -------- <OFD Version="">
            // Required.
            std::tie(Version, exist) = rootElement->GetStringAttribute("Version");
            if (!exist) {
//                LOG(ERROR) << "Attribute Version is Required in OFD.xsd";
                return false;
            }

            // -------- <OFD DocType="">
            // Required.
            std::tie(DocType, exist) = rootElement->GetStringAttribute("DocType");
            if (!exist) {
//                LOG(ERROR) << "Attribute DocType is Required in OFD.xsd";
                return false;
            }

            bool hasDocBody = false;
            XMLElementPtr childElement = rootElement->GetFirstChildElement();
            while (childElement != nullptr) {
                std::string childName = childElement->GetName();

                // -------- <DocBody>
                // Required.
                if (childName == "DocBody") {
                    hasDocBody = true;

                    DocumentPtr document = AddNewDocument();
                    document->FromDocBodyXML(childElement);

                    std::string docRoot = document->GetDocRoot();
                    std::string docXMLFile = docRoot + "/Document.xml";
//                    LOG(INFO) << "Document xml:" << docXMLFile;

                    std::string strDocumentXML;
                    std::tie(strDocumentXML, ok) = ReadZipFileString(docXMLFile);
                    ok = document->FromDocumentXML(strDocumentXML);
                }

                childElement = childElement->GetNextSiblingElement();
            }
            if (!hasDocBody) {
            }
        } else {
//            LOG(ERROR) << "Root element in OFD.xml is not named 'OFD'";
        }
    } else {
//        LOG(ERROR) << "No root element in OFD.xml";
    }

    return ok;
}
