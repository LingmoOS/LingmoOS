// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dpdfdoc.h"
#include "dpdfpage.h"

#include "public/fpdfview.h"
#include "public/fpdf_doc.h"
#include "public/fpdf_save.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdf_formfill.h"

#include "core/fpdfdoc/cpdf_bookmark.h"
#include "core/fpdfdoc/cpdf_bookmarktree.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfdoc/cpdf_pagelabel.h"

#include <QFile>
#include <QTemporaryDir>
#include <QUuid>
#include <unistd.h>

/**
 * @brief The PDFIumLoader class for FPDF_FILEACCESS
 */
class PDFIumLoader
{
public:
    PDFIumLoader() {}
    PDFIumLoader(const char *pBuf, size_t len);
    const char *m_pBuf;
    size_t m_Len;
};

/**
 * @brief GetBlock for FPDF_FILEACCESS
 * @param param 为PDFIumLoader类型
 */
static int GetBlock(void *param, unsigned long pos, unsigned char *pBuf, unsigned long size)
{
    PDFIumLoader *pLoader = static_cast<PDFIumLoader *>(param);
    if (pos + size < pos || pos + size > pLoader->m_Len) return 0;
    memcpy(pBuf, pLoader->m_pBuf + pos, size);
    return 1;
}

/**
 * @brief IsDataAvail for FX_FILEAVAIL
 */
static FPDF_BOOL IsDataAvail(FX_FILEAVAIL * /*pThis*/, size_t /*offset*/, size_t /*size*/)
{
    return true;
}

DPdfDoc::Status parseError(int error)
{
    DPdfDoc::Status err_code = DPdfDoc::SUCCESS;
    // Translate FPDFAPI error code to FPDFVIEW error code
    switch (error) {
    case FPDF_ERR_SUCCESS:
        err_code = DPdfDoc::SUCCESS;
        break;
    case FPDF_ERR_FILE:
        err_code = DPdfDoc::FILE_ERROR;
        break;
    case FPDF_ERR_FORMAT:
        err_code = DPdfDoc::FORMAT_ERROR;
        break;
    case FPDF_ERR_PASSWORD:
        err_code = DPdfDoc::PASSWORD_ERROR;
        break;
    case FPDF_ERR_SECURITY:
        err_code = DPdfDoc::HANDLER_ERROR;
        break;
    }
    return err_code;
}

class DPdfDocPrivate
{
    friend class DPdfDoc;
public:
    DPdfDocPrivate();

    ~DPdfDocPrivate();

public:
    DPdfDoc::Status loadFile(const QString &filePath, const QString &password);

private:
    DPdfDocHandler *m_docHandler;

    QVector<DPdfPage *> m_pages;

    QString m_filePath;

    int m_pageCount = 0;

    DPdfDoc::Status m_status;
};

DPdfDocPrivate::DPdfDocPrivate()
{
    m_docHandler = nullptr;
    m_pageCount = 0;
    m_status = DPdfDoc::NOT_LOADED;
}

DPdfDocPrivate::~DPdfDocPrivate()
{
    DPdfMutexLocker locker("DPdfDocPrivate::~DPdfDocPrivate()");

    qDeleteAll(m_pages);

    if (nullptr != m_docHandler)
        FPDF_CloseDocument(reinterpret_cast<FPDF_DOCUMENT>(m_docHandler));
}

DPdfDoc::Status DPdfDocPrivate::loadFile(const QString &filePath, const QString &password)
{
    m_filePath = filePath;

    m_pages.clear();

    if (!QFile::exists(m_filePath)) {
        m_status = DPdfDoc::FILE_NOT_FOUND_ERROR;
        return m_status;
    }

    DPdfMutexLocker locker("DPdfDocPrivate::loadFile");

    qDebug() << "deepin-pdfium正在加载PDF文档... 文档名称: " << filePath;
    void *ptr = FPDF_LoadDocument(m_filePath.toUtf8().constData(),
                                  password.toUtf8().constData());

    m_docHandler = static_cast<DPdfDocHandler *>(ptr);

    m_status = m_docHandler ? DPdfDoc::SUCCESS : parseError(static_cast<int>(FPDF_GetLastError()));
    qDebug() << "文档（" << filePath << "）文档加载状态: " << m_status;

    if (m_docHandler) {
        m_pageCount = FPDF_GetPageCount(reinterpret_cast<FPDF_DOCUMENT>(m_docHandler));
        m_pages.fill(nullptr, m_pageCount);
    }

    return m_status;
}


DPdfDoc::DPdfDoc(QString filename, QString password)
    : d_ptr(new DPdfDocPrivate())
{
    d_func()->loadFile(filename, password);
}

DPdfDoc::~DPdfDoc()
{

}

bool DPdfDoc::isValid() const
{
    return d_func()->m_docHandler != nullptr;
}

bool DPdfDoc::isEncrypted() const
{
    if (!isValid())
        return false;

    DPdfMutexLocker locker("DPdfDoc::isEncrypted()");

    return FPDF_GetDocPermissions(reinterpret_cast<FPDF_DOCUMENT>(d_func()->m_docHandler)) != 0xFFFFFFFF;
}

DPdfDoc::Status DPdfDoc::tryLoadFile(const QString &filename, const QString &password)
{
    Status status = NOT_LOADED;
    if (!QFile::exists(filename)) {
        status = FILE_NOT_FOUND_ERROR;
        return status;
    }

    DPdfMutexLocker locker("DPdfDoc::tryLoadFile");

    void *ptr = FPDF_LoadDocument(filename.toUtf8().constData(),
                                  password.toUtf8().constData());

    DPdfDocHandler *docHandler = static_cast<DPdfDocHandler *>(ptr);
    status = docHandler ? SUCCESS : parseError(static_cast<int>(FPDF_GetLastError()));

    if (docHandler) {
        FPDF_CloseDocument(reinterpret_cast<FPDF_DOCUMENT>(docHandler));
    }

    return status;
}

bool DPdfDoc::isLinearized(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        qInfo() << "file open failed when isLinearized" << fileName;
    }
    QByteArray content = file.readAll();
    int len = content.length();
    const char *pBuf = content.data();

    PDFIumLoader m_PDFIumLoader;
    m_PDFIumLoader.m_pBuf = pBuf;//pdf content
    m_PDFIumLoader.m_Len = size_t(len);//content len

    FPDF_FILEACCESS m_fileAccess;
    memset(&m_fileAccess, '\0', sizeof(m_fileAccess));
    m_fileAccess.m_FileLen = static_cast<unsigned long>(len);
    m_fileAccess.m_GetBlock = GetBlock;
    m_fileAccess.m_Param = &m_PDFIumLoader;

    FX_FILEAVAIL m_fileAvail;
    memset(&m_fileAvail, '\0', sizeof(m_fileAvail));
    m_fileAvail.version = 1;
    m_fileAvail.IsDataAvail = IsDataAvail;

    FPDF_AVAIL m_PdfAvail;
    m_PdfAvail = FPDFAvail_Create(&m_fileAvail, &m_fileAccess);

    return FPDFAvail_IsLinearized(m_PdfAvail) > 0;
}

static QFile saveWriter;

int writeFile(struct FPDF_FILEWRITE_* pThis, const void *pData, unsigned long size)
{
    Q_UNUSED(pThis)
    return 0 != saveWriter.write(static_cast<char *>(const_cast<void *>(pData)), static_cast<qint64>(size));
}

bool DPdfDoc::save()
{
    FPDF_FILEWRITE write;

    write.WriteBlock = writeFile;

    QTemporaryDir tempDir;

    QString tempFilePath = tempDir.path() + "/" + QUuid::createUuid().toString();

    saveWriter.setFileName(tempFilePath);

    if (!saveWriter.open(QIODevice::WriteOnly))
        return false;

    DPdfMutexLocker locker("DPdfDoc::save");
    bool result = FPDF_SaveAsCopy(reinterpret_cast<FPDF_DOCUMENT>(d_func()->m_docHandler), &write, FPDF_NO_INCREMENTAL);
    locker.unlock();

    saveWriter.close();

    QFile tempFile(tempFilePath);

    if (!tempFile.open(QIODevice::ReadOnly))
        return false;

    QByteArray array = tempFile.readAll();

    tempFile.close();

    QFile file(d_func()->m_filePath);

    file.remove();          //不remove会出现第二次导出丢失数据问题 (保存动作完成之后，如果当前文档是当初打开那个，下一次导出会出错)

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    if (array.size() != file.write(array))
        result = false;

    file.flush();//函数将用户缓存中的内容写入内核缓冲区
    fsync(file.handle());//将内核缓冲写入文件(磁盘)
    file.close();
    return result;
}

bool DPdfDoc::saveAs(const QString &filePath)
{
    FPDF_FILEWRITE write;

    write.WriteBlock = writeFile;

    saveWriter.setFileName(filePath);

    if (!saveWriter.open(QIODevice::ReadWrite))
        return false;

    DPdfMutexLocker locker("DPdfDoc::saveAs");
    bool result = FPDF_SaveAsCopy(reinterpret_cast<FPDF_DOCUMENT>(d_func()->m_docHandler), &write, FPDF_NO_INCREMENTAL);
    locker.unlock();

    saveWriter.close();

    return result;
}

QString DPdfDoc::filePath() const
{
    return d_func()->m_filePath;
}

int DPdfDoc::pageCount() const
{
    return d_func()->m_pageCount;
}

DPdfDoc::Status DPdfDoc::status() const
{
    return d_func()->m_status;
}

DPdfPage *DPdfDoc::page(int i, qreal xRes, qreal yRes)
{
    if (i < 0 || i >= d_func()->m_pageCount)
        return nullptr;

    if (!d_func()->m_pages[i]) {
        d_func()->m_pages[i] = new DPdfPage(d_func()->m_docHandler, i, xRes, yRes);
    }

    return d_func()->m_pages[i];
}

void collectBookmarks(DPdfDoc::Outline &outline, const CPDF_BookmarkTree &tree, CPDF_Bookmark This, qreal xRes, qreal yRes)
{
    DPdfDoc::Section section;

    const WideString &title = This.GetTitle();

    section.title = QString::fromWCharArray(title.c_str(), static_cast<int>(title.GetLength()));

    bool hasx = false, hasy = false, haszoom = false;
    float x = 0.0, y = 0.0, z = 0.0;

    const CPDF_Dest &dest = This.GetDest(tree.GetDocument()).GetArray() ? This.GetDest(tree.GetDocument()) : This.GetAction().GetDest(tree.GetDocument());
    section.nIndex = dest.GetDestPageIndex(tree.GetDocument());
    dest.GetXYZ(&hasx, &hasy, &haszoom, &x, &y, &z);
    section.offsetPointF = QPointF(static_cast<qreal>(x) * xRes / 72, static_cast<qreal>(y) * yRes / 72);

    const CPDF_Bookmark &Child = tree.GetFirstChild(&This);
    if (Child.GetDict() != nullptr) {
        collectBookmarks(section.children, tree, Child, xRes, yRes);
    }
    outline << section;

    const CPDF_Bookmark &SibChild = tree.GetNextSibling(&This);
    if (SibChild.GetDict() != nullptr) {
        collectBookmarks(outline, tree, SibChild, xRes, yRes);
    }
}

DPdfDoc::Outline DPdfDoc::outline(qreal xRes, qreal yRes)
{
    DPdfMutexLocker locker("DPdfDoc::outline");

    Outline outline;
    CPDF_BookmarkTree tree(reinterpret_cast<CPDF_Document *>(d_func()->m_docHandler));
    CPDF_Bookmark cBookmark;
    const CPDF_Bookmark &firstRootChild = tree.GetFirstChild(&cBookmark);
    if (firstRootChild.GetDict() != nullptr)
        collectBookmarks(outline, tree, firstRootChild, xRes, yRes);

    return outline;
}

DPdfDoc::Properies DPdfDoc::proeries()
{
    DPdfMutexLocker locker("DPdfDoc::proeries");

    Properies properies;
    int fileversion = 1;
    properies.insert("Version", "1");
    if (FPDF_GetFileVersion(reinterpret_cast<FPDF_DOCUMENT>(d_func()->m_docHandler), &fileversion)) {
        properies.insert("Version", QString("%1.%2").arg(fileversion / 10).arg(fileversion % 10));
    }
    properies.insert("Encrypted", isEncrypted());
    properies.insert("Linearized", FPDF_GetFileLinearized(reinterpret_cast<FPDF_DOCUMENT>(d_func()->m_docHandler)));
    properies.insert("KeyWords", QString());
    properies.insert("Title", QString());
    properies.insert("Creator", QString());
    properies.insert("Producer", QString());
    CPDF_Document *pDoc = reinterpret_cast<CPDF_Document *>(d_func()->m_docHandler);

    const CPDF_Dictionary *pInfo = pDoc->GetInfo();
    if (pInfo) {
        const WideString &KeyWords = pInfo->GetUnicodeTextFor("KeyWords");
        properies.insert("KeyWords", QString::fromWCharArray(KeyWords.c_str()));

        //windows和mac上生成的pdf此处编码格式不同,需要嗅探查找
        const ByteString &Title = pInfo->GetStringFor("Title");
        if ("utf-8" == DPdfGlobal::textCodeType(Title.c_str())) {
            properies.insert("Title", QString::fromUtf8(Title.c_str()));
        } else {
            const WideString &WTitle = pInfo->GetUnicodeTextFor("Title");
            properies.insert("Title", QString::fromWCharArray(WTitle.c_str()));
        }

        const WideString &Creator = pInfo->GetUnicodeTextFor("Creator");
        properies.insert("Creator", QString::fromWCharArray(Creator.c_str()));

        const WideString &Producer = pInfo->GetUnicodeTextFor("Producer");
        properies.insert("Producer", QString::fromWCharArray(Producer.c_str()));
    }

    return properies;
}

QString DPdfDoc::label(int index) const
{
    DPdfMutexLocker locker("DPdfDoc::label index = " + QString::number(index));

    CPDF_PageLabel label(reinterpret_cast<CPDF_Document *>(d_func()->m_docHandler));
    const Optional<WideString> &str = label.GetLabel(index);
    if (str.has_value())
        return QString::fromWCharArray(str.value().c_str(), static_cast<int>(str.value().GetLength()));
    return QString();
}
