#ifndef __OFD_RESOURCE_H__
#define __OFD_RESOURCE_H__

#include <memory>
#include <string>
#include "ofd/Common.h"

namespace ofd {

enum class ResourceLevel {
    PACKAGE = 0,
    DOCUMENT,
    PAGE,
};

// OFD (section 7.9) P23. Res.xsd
class Resource : public std::enable_shared_from_this<Resource>
{
private:
    Resource(PackagePtr package, const std::string &resDescFile);
    Resource(DocumentPtr document, const std::string &resDescFile);
    Resource(PagePtr page, const std::string &resDescFile);

public:
    ~Resource();

    ResourcePtr GetSelf();

    static ResourcePtr CreateNewResource(PackagePtr package, const std::string &resDescFile = "PublicRes.xml");
    static ResourcePtr CreateNewResource(DocumentPtr document, const std::string &resDescFile = "DocumentRes.xml");
    static ResourcePtr CreateNewResource(PagePtr page, const std::string &resDescFile = "PageRes.xml");

    ResourceLevel GetResourceLevel() const;

    const PackagePtr GetPackage() const;
    const DocumentPtr GetDocument() const;
    const PagePtr GetPage() const;

    std::string GetEntryRoot() const;
    std::string GetBaseLoc() const;
    void SetBaseLoc(const std::string &baseLoc);
    std::string GetResDescFile() const;

    const ColorSpaceArray &GetColorSpaces() const;
    void AddColorSpace(const ColorSpacePtr &colorSpace);

    std::string GenerateResXML() const;
    bool FromResXML(const std::string &strResXML);

    bool LoadFonts();

    bool LoadImages();

    std::string GenerateResourceFilePath(const std::string resourceFile);

private:
    class ImplCls;
    std::unique_ptr<ImplCls> m_impl;

};   // class Resource

};   // namespace ofd

#endif   // __OFD_RESOURCE_H__
