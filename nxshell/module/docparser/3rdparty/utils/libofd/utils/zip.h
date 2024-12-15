#ifndef __UTILS_ZIP_H__
#define __UTILS_ZIP_H__

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include "utils/utils.h"

namespace utils {

    class Zip : public std::enable_shared_from_this<Zip> {
    public:
        Zip();
        ~Zip();

        ZipPtr GetSelf();

        bool Open(const std::string &filename, bool bWrite);
        void Close();

        std::tuple<std::string, bool> ReadFileString(const std::string &fileinzip) const;
        std::tuple<char*, size_t, bool> ReadFileRaw(const std::string &fileinzip) const;
        bool AddFile(const std::string &filename, const std::string &text);
        bool AddFile(const std::string &filename, const char *buf, size_t bufSize);
        bool AddDir(const std::string &dirName);

    private:
        class ImplCls;
        std::unique_ptr<ImplCls> m_impl;
    }; // class Zip

}; // namespace utils

#endif // __UTILS_ZIP_H__
