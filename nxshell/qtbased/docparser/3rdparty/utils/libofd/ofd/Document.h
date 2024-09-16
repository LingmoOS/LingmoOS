#ifndef __OFD_DOCUMENT_H__
#define __OFD_DOCUMENT_H__

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include "ofd/Common.h"

namespace ofd{

    class Document : public std::enable_shared_from_this<Document> {
        private:
            Document(PackagePtr package, const std::string &docRoot);
        public:
            static DocumentPtr CreateNewDocument(PackagePtr package, const std::string &docRoot);

        public:
            virtual ~Document();
            DocumentPtr GetSelf();
            std::string to_string() const;

            // =============== Public Attributes ================
        public:

            // ======== struct CommonData ========
            // 文档公共数据，定义了页面区域、公共资源等数据。
            typedef struct CommonData{
                public:
                    // 当前文档中所有对象使用标识的最大值，初始值为0。
                    // MaxUnitID主要用于文档编辑，在向文档中新增一个对象时，
                    // 需要分配一个新的标识，新标识取值宜为MaxUnitID+1，
                    // 同时需要修改此MaxUnitID值。必选。
                    uint64_t MaxUnitID;

                    // 指定该文档区域的默认大小和位置。必选。
                    CT_PageArea PageArea;

                    // 公共资源序列，每一个节点指向OFD包内的一个资源描述文档。
                    //std::vector<ST_Loc> PublicRes;
                    ResourcePtr PublicRes;

                    // 文档资源序列，每一个节点指向OFD包内的一个资源描述文档。
                    //std::vector<ST_Loc> DocumentRes;
                    ResourcePtr DocumentRes;

                    // 模板页序列，为一系列模板页的集合，模板页内容结构和普通页相同。
                    //CT_TemplatePage TemplatePage;

                    // 引用在资源文件中定义的颜色空间标识。如果此项不存在，采用RGB作为默认颜色空间。
                    //uint64_t DefaultCS;


                    CommonData() : MaxUnitID(0) {};

            } CommonData_t; // struct CommonData

            // ======== struct CT_DocInfo ========
            // 文档元数据定义。
            // 标准 P7 页，OFD.xsd。
            typedef struct CT_DocInfo{

                typedef std::vector<std::string> KeywordArray;
                typedef std::unordered_map<std::string, std::string> CustomDataMap;

                std::string    DocID;          // 采用UUID算法生成的由32个字节组成的文件标识。
                // 每个DocID在文档创建或生成的时候进行分配。
                std::string    Title;          // 文档标题，标题可以与文件名不同 
                std::string    Author;         // 文档作者
                std::string    Subject;        // 文档主题
                std::string    Abstract;       // 文档摘要与注释
                ST_TIME        CreationDate;   // 文档的创建日期
                ST_TIME        ModDate;        // 文档的最近修改日期
                std::string    DocUsage;       // 文档分类 ("Normal", "EBook", "ENewsPaper", "EMagzine")
                ST_Loc         Cover;          // 文档封面，此路径指向一个图片文件。
                KeywordArray   Keywords;       // 关键词集合
                std::string    Creator;        // 创建文档的应用程序。
                std::string    CreatorVersion; // 创建文档的应用程序的版本信息。
                CustomDataMap  CustomDatas;     // 用户自定义数据

                CT_DocInfo();

            } CT_DocInfo_t;

            // ******** struct DocBody ********
            // 文件对象入口，可以存在多个，以便在一个文档中包含多个版式文件。
            typedef struct DocBody{
                public:

                    class Version{
                        public:
                            std::string ID;      // 版本标识
                            int         Index;   // 版本号
                            bool        Current; // 是否默认版本，默认值为false。
                            ST_Loc      BaseLoc; // 指向包内的版本描述文件。

                            Version() : Current(false){};
                    };
                    typedef std::vector<Version> VersionArray;

                    CT_DocInfo   DocInfo;    // 文档元数据信息描述。
                    std::string  DocRoot;    // 指向文档根节点文档。
                    VersionArray Versions;   // 包含多个版本描述节点，
                    // 用于定义文件因注释和其它改动产生的版本信息。
                    std::string  Signatures; // 指向文档中签名和签章结构。

            } DocBody_t;

            // =============== Public Methods ================

            bool Open();
            void Close();
            bool IsOpened() const{return m_opened;};

            size_t GetNumPages() const;
            const PagePtr GetPage(size_t idx) const;
            PagePtr GetPage(size_t idx);
            PagePtr AddNewPage();

            // Called by ofd::Package::Save().
            std::string GenerateDocumentXML() const;
            // Called by Package::generateOFDXML()
            std::string GenerateDocBodyXML() const;


            // Called by Package::fromOFDXML()
            bool FromDocBodyXML(utils::XMLElementPtr docBodyElement);

            // Called by Package::fromOFDXML()
            bool FromDocumentXML(const std::string &strDocumentXML);

            // ---------------- Private Attributes ----------------
        public:
            const PackagePtr GetPackage() const {return m_package.lock();};
            PackagePtr GetPackage(){return m_package.lock();};

            std::string GetDocRoot() const {return m_docBody.DocRoot;};
            const CommonData& GetCommonData() const {return m_commonData;};
            CommonData& GetCommonData() {return m_commonData;};
            const ResourcePtr GetDocumentRes() const{return GetCommonData().DocumentRes;};
            ResourcePtr GetDocumentRes() {return GetCommonData().DocumentRes;};
            const DocBody& GetDocBody() const {return m_docBody;};
            DocBody& GetDocBody() {return m_docBody;};

        private:
            std::weak_ptr<Package> m_package;
            bool              m_opened;
            PageArray         m_pages;
            CommonData        m_commonData;
            DocBody           m_docBody;

            void generateCommonDataXML(utils::XMLWriter &writer) const;
            void generatePagesXML(utils::XMLWriter &writer) const;
            bool fromDocInfoXML(utils::XMLElementPtr docInfoElement);
            bool fromCommonDataXML(utils::XMLElementPtr commonDataElement);
            bool fromPagesXML(utils::XMLElementPtr pagesElement);

    }; // class Document


}; // namespce ofd

#endif // __OFD_DOCUMENT_H__
