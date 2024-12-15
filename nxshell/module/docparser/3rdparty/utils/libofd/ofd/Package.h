#ifndef __OFD_PACKAGE_H__
#define __OFD_PACKAGE_H__

#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include "ofd/Common.h"

namespace ofd{

    class Package : public std::enable_shared_from_this<Package> {
        public:
            Package();
            Package(const std::string &filename);
            virtual ~Package();

            PackagePtr GetSelf();

            // =============== Public Attributes ================
        public:
            std::string   Version;   // 文件格式的版本号，取值为”1.0“
            std::string   DocType;   // 文件格式子集类型，取值为”OFD“，表明此文件符合本标准
                                // 取值为”OFD-A“，表明此文件符合OFD存档规范

            // =============== Public Methods ================
        public:
            bool Open(const std::string &filename);
            void Close();
            bool Save(const std::string &filename);
            DocumentPtr AddNewDocument();

            std::tuple<std::string, bool> ReadZipFileString(const std::string &fileinzip) const;
            std::tuple<char*, size_t, bool> ReadZipFileRaw(const std::string &fileinzip) const;

            // ---------------- Private Attributes ----------------
        public:
            // 获取包文件中文件对象。
            size_t GetDocumentsCount() const;
            const DocumentPtr GetDocument(size_t idx) const;
            DocumentPtr GetDocument(size_t idx);
            const DocumentPtr GetDefaultDocument() const;
            DocumentPtr GetDefaultDocument();

        protected:
            std::string m_filename;  // 包文件绝对路径
            bool m_opened;           // 包文件是否成功打开标志

        private:
            DocumentArray m_documents; // 文件对象入口集合
            utils::ZipPtr m_zip;

            std::string generateOFDXML() const;
            bool fromOFDXML(const std::string &strOFDXML);

    }; // class Package

}; // namespace ofd

#endif // __OFD_PACKAGE_H__
