#include "utils/utils.h"
#include "utils/zip.h"
#include <cstring>

#define ZIP_DISABLE_DEPRECATED
#include <zip.h>

using namespace utils;

// **************** class Zip::ImplCls ****************
class Zip::ImplCls{
public:
    ImplCls(Zip *zip);
    ~ImplCls();

    bool Open(const std::string &filename, bool bWrite);
    void Close();

    std::tuple<std::string, bool> ReadFileString(const std::string &fileinzip) const;
    std::tuple<char*, size_t, bool> ReadFileRaw(const std::string &fileinzip) const;
    bool AddFileString(const std::string &filename, const std::string &text);
    bool AddFileRaw(const std::string &filename, const char *buf, size_t bufSize);
    bool AddDir(const std::string &dirName);

public:
    Zip *m_zip;
    zip *m_archive;
};


Zip::ImplCls::ImplCls(Zip *zip) : m_zip(zip), m_archive(nullptr) {
}

Zip::ImplCls::~ImplCls(){
    Close();
}

bool Zip::ImplCls::Open(const std::string &filename, bool bWrite){
    int error = 0;

    if ( bWrite ){
        //m_archive = zip_open(filename.c_str(), ZIP_CREATE | ZIP_EXCL, &error);
        m_archive = zip_open(filename.c_str(), ZIP_CREATE, &error);
    } else {
        m_archive = zip_open(filename.c_str(), 0, &error);
    }

    if ( m_archive == nullptr ){
        if ( error == ZIP_ER_EXISTS ){
//            LOG(ERROR) << "Error: Open " << filename << " failed. error=" << error << " The file exists and ZIP_EXCL is set.";
        } else {
//            LOG(ERROR) << "Error: Open " << filename << " failed. error=" << error;
        }
        return false;
    }

    return true;
}

void Zip::ImplCls::Close(){
    if ( m_archive != nullptr ){
        zip_close(m_archive);
        m_archive = nullptr;
    }
}

std::tuple<std::string, bool> Zip::ImplCls::ReadFileString(const std::string &fileinzip) const{
    bool ok = false;
    std::string fileContent;

    if ( m_archive != nullptr ){
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat(m_archive, fileinzip.c_str(), ZIP_FL_NOCASE, &st);
//        LOG(DEBUG) << "zip_stat:" << st.valid;

        size_t filesize = st.size;
        __attribute__((unused)) size_t compsize = st.comp_size;

        zip_file *file = zip_fopen(m_archive, fileinzip.c_str(), ZIP_FL_NOCASE);
        if (file) {
            char *content = new char[filesize + 1];
            size_t did_read = zip_fread(file, content, filesize);
//            LOG(DEBUG) << "did_read:" << did_read << " filesize:" << filesize;
            if (did_read != filesize ) {
//                LOG(WARNING) << "File " << fileinzip << " readed " << did_read << " bytes, but is not equal to excepted filesize " << filesize << " bytes.";
                delete[] content;
            } else {
                content[filesize] = '\0';
                fileContent = std::string(content);
                ok = true;
                delete[] content;
            }
            zip_fclose(file);
        }
    }

    return std::make_tuple(fileContent, ok);
}

std::tuple<char*, size_t, bool> Zip::ImplCls::ReadFileRaw(const std::string &fileinzip) const {
    bool ok = false;
    char *content = nullptr;
    size_t filesize = 0;

    if ( m_archive != nullptr ) {
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat(m_archive, fileinzip.c_str(), ZIP_FL_NOCASE, &st);
//        LOG(DEBUG) << "zip_stat:" << st.valid;

        filesize = st.size;
        __attribute__((unused)) size_t compsize = st.comp_size;

        zip_file *file = zip_fopen(m_archive, fileinzip.c_str(), ZIP_FL_NOCASE);
        if (file) {
            content = new char[filesize];
            size_t did_read = zip_fread(file, content, filesize);
//            LOG(DEBUG) << "did_read:" << did_read << " filesize:" << filesize;
            if (did_read != filesize ) {
//                LOG(WARNING) << "File " << fileinzip << " readed " << did_read << " bytes, but is not equal to excepted filesize " << filesize << " bytes.";
                delete[] content;
            } else {
                ok = true;
            }
            zip_fclose(file);
        }
    }

    return std::make_tuple(content, filesize, ok);
}

bool Zip::ImplCls::AddFileString(const std::string &filename, const std::string &text) {
    if ( m_archive ){
        return AddFileRaw(filename, text.c_str(), text.length());
    } else {
        return false;
    }
}

bool Zip::ImplCls::AddFileRaw(const std::string &filename, const char *text, size_t len) {
    if ( m_archive != nullptr ){

        // FIXME
        char *buf = new char[len + 1];
        memcpy(buf, text, len);
        buf[len] = '\0';
        size_t bufSize = len;

        /*zip_source *s = zip_source_buffer(archive, text.c_str(), text.length(), 0);*/
        zip_source *s = zip_source_buffer(m_archive, buf, bufSize, 1);
        if ( s == nullptr ) {
//            LOG(ERROR) << "zip_source_buffer_create() failed. filename:" << filename;
            return false;
        }
        int ret = zip_file_add(m_archive, filename.c_str(), s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
        if ( ret >= 0 ){
            return true;
        } else {
//            LOG(ERROR) << "zip_file_add() failed. filename:" << filename;
            zip_source_free(s);
            return false;
        }
    }
    return false;
}

bool Zip::ImplCls::AddDir(const std::string &dirName) {
    if ( m_archive != nullptr ){
        int ret = zip_dir_add(m_archive, dirName.c_str(), ZIP_FL_ENC_UTF_8);
        return ret != -1;
    } 
    return false;
}


// **************** class Zip ****************

Zip::Zip(){
    m_impl = make_unique<Zip::ImplCls>(this);
}

Zip::~Zip(){
}

bool Zip::Open(const std::string &filename, bool bWrite){
    return m_impl->Open(filename, bWrite);
}

void Zip::Close(){
    m_impl->Close();
}

ZipPtr Zip::GetSelf(){
    return shared_from_this();
}

std::tuple<std::string, bool> Zip::ReadFileString(const std::string &fileinzip) const{
    return m_impl->ReadFileString(fileinzip);
}

std::tuple<char*, size_t, bool> Zip::ReadFileRaw(const std::string &fileinzip) const {
    return m_impl->ReadFileRaw(fileinzip);
}

bool Zip::AddFile(const std::string &filename, const std::string &text) {
//    LOG(DEBUG) << "Zip::AddFile(). filename: " << filename;
    return m_impl->AddFileString(filename, text);
}

bool Zip::AddFile(const std::string &filename, const char *buf, size_t bufSize) {
//    LOG(DEBUG) << "Zip::AddFile(). filename: " << filename;
    return m_impl->AddFileRaw(filename, buf, bufSize);
}

bool Zip::AddDir(const std::string &dirName) {
//    LOG(DEBUG) << "Zip::AddDir(). dirame: " << dirName;
    return m_impl->AddDir(dirName);
}
