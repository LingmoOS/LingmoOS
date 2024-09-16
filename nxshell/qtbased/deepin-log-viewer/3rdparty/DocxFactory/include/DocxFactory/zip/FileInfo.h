
#ifndef __DOCXFACTORY_FILE_INFO_H__
#define __DOCXFACTORY_FILE_INFO_H__

#include "minizip/zip.h"
#include "minizip/unzip.h"



namespace DocxFactory {
using namespace std;

class FileInfo
{
public:
    FileInfo(unz_file_info p_unzFileInfo);
    virtual ~FileInfo();

    const unz_file_info *getUnzFileInfo()   const;
    const zip_fileinfo *getZipFileInfo()    const;

protected:

private:
    FileInfo(const FileInfo &p_other);
    FileInfo operator = (const FileInfo &p_other);

    unz_file_info   m_unzFileInfo;
    zip_fileinfo    m_zipFileInfo;
};
};

#endif
